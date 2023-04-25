#include "inlining.h"
#include "bb.h"
#include "graph.h"

GEN_VISIT_FUNCTIONS_WITH_BLOCK_ORDER(Inlining, RPO);

Inlining::Inlining(Graph* graph) : Pass(graph)
{
}

bool Inlining::RunPass()
{
    VisitGraph();
    return true;
}

void Inlining::VisitCALL_STATIC(GraphVisitor* v, InstBase* inst)
{
    using T = typename isa::inst::Inst<isa::inst::Opcode::CALL_STATIC>::Type;

    assert(v != nullptr);
    assert(inst != nullptr);
    assert(inst->GetOpcode() == isa::inst::Opcode::CALL_STATIC);

    auto _this = static_cast<Inlining*>(v);

    _this->cur_call_ = inst;
    _this->callee_start_bb_ = static_cast<T*>(inst)->GetCallee()->GetStartBasicBlock();

    _this->UpdateDFGParameters();
    _this->UpdateDFGReturns();
    _this->MoveConstants();
    _this->MoveCalleeBlocks();
    _this->InsertInlinedGraph();
    _this->ResetState();
}

void Inlining::ResetState()
{
    ret_bbs_.clear();
    cur_call_ = nullptr;
    callee_start_bb_ = nullptr;
}

void Inlining::UpdateDFGParameters()
{
    auto param = callee_start_bb_->GetFirstInst();
    for (const auto& arg : cur_call_->GetInputs()) {
        // argument number mismatch
        assert(param->IsParam());

        for (const auto& user : param->GetUsers()) {
            user.GetInst()->ReplaceInput(param, arg.GetInst());
            arg.GetInst()->AddUser(user);
        }
        param = param->GetNext();
    }

    // check that number of params matches number of arguments
    assert(param == nullptr || !param->IsParam());
}

// move caller users to return input instruction(or PHI instructions for several return
// instructions)
void Inlining::UpdateDFGReturns()
{
    auto call_inst =
        static_cast<typename isa::inst::Inst<isa::inst::Opcode::CALL_STATIC>::Type*>(cur_call_);
    auto callee_blocks = call_inst->GetCallee()->GetAnalyser()->GetValidPass<RPO>()->GetBlocks();

    std::vector<InstBase*> rets{};
    for (const auto& bb : callee_blocks) {
        auto last_inst = bb->GetLastInst();
        if (last_inst->IsReturn()) {
            rets.push_back(last_inst);
            ret_bbs_.push_back(bb);
        }
    }

    assert(!rets.empty());

    using ReturnT = typename isa::inst::Inst<isa::inst::Opcode::RETURN>::Type;
    using RetNumArgs = isa::InputValue<ReturnT, isa::input::Type::VREG>;

    // only one return type per function
    switch (rets.front()->GetOpcode()) {
    case isa::inst::Opcode::RETURN: {
        InstBase* call_ret_res{ nullptr };
        if (rets.size() == 1) {
            assert(RetNumArgs::value == rets.front()->GetNumInputs());
            call_ret_res = rets.front()->GetInput(0).GetInst();
            call_ret_res->RemoveUser(rets.front());
            rets.front()->GetBasicBlock()->UnlinkInst(rets.front());
        } else {
            ret_phi_ = InstBase::NewInst<isa::inst::Opcode::PHI>();
            call_ret_res = ret_phi_.get();

            assert(call_ret_res != nullptr);

            for (const auto& ret : rets) {
                assert(RetNumArgs::value == ret->GetNumInputs());
                // input is ret's input, but phi's bb is bb, where ret was
                auto ret_input = ret->GetInput(0).GetInst();
                ret_phi_->AddInput(ret_input, ret->GetBasicBlock());
                ret_input->RemoveUser(ret);
                ret->GetBasicBlock()->UnlinkInst(ret);
            }
        }

        for (const auto& user : call_inst->GetUsers()) {
            user.GetInst()->ReplaceInput(call_inst, call_ret_res);
            call_ret_res->AddUser(user);
        }
    } break;
    case isa::inst::Opcode::RETURN_VOID: {
        for (const auto& ret : rets) {
            ret->GetBasicBlock()->UnlinkInst(ret);
        }
    } break;
    default:
        assert(false);
    }
}

void Inlining::MoveConstants()
{
    graph_->AppendBasicBlock(graph_->GetStartBasicBlock(), callee_start_bb_);
}

void Inlining::MoveCalleeBlocks()
{
    auto call_inst =
        static_cast<typename isa::inst::Inst<isa::inst::Opcode::CALL_STATIC>::Type*>(cur_call_);
    auto callee = call_inst->GetCallee();
    for (const auto& bb : callee->GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
        graph_->NewBasicBlock(callee->ReleaseBasicBlock(bb->GetId()));
    }
}

void Inlining::InsertInlinedGraph()
{
    auto call_cont_block = graph_->SplitBasicBlock(cur_call_);
    auto call_block = cur_call_->GetBasicBlock();

    // remove call instruction by hand
    for (const auto& input : cur_call_->GetInputs()) {
        input.GetInst()->RemoveUser(cur_call_);
    }
    call_block->UnlinkInst(cur_call_);

    assert(call_block->GetNumSuccessors() == 1);
    assert(call_block->GetSuccessor(0) == call_cont_block);
    assert(call_cont_block->GetNumPredecessors() == 1);
    assert(call_cont_block->GetPredecessor(0) == call_block);

    if (ret_phi_.get() != nullptr) {
        call_cont_block->PushBackPhi(std::move(ret_phi_));
    }

    graph_->RemoveEdge(call_block, call_cont_block);

    assert(call_block->HasNoSuccessors());
    assert(call_cont_block->HasNoPredecessors());
    assert(callee_start_bb_->HasNoPredecessors());

    graph_->AddEdge(call_block, callee_start_bb_);

    for (const auto& ret_bb : ret_bbs_) {
        graph_->AddEdge(ret_bb, call_cont_block);
    }
}
