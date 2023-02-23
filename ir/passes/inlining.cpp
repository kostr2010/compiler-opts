#include "inlining.h"
#include "bb.h"
#include "graph.h"

Inlining::Inlining(Graph* graph) : Pass(graph)
{
}

bool Inlining::RunPass()
{
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
        for (auto inst = bb->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
            if (!inst->HasFlag(InstFlags::IS_CALL)) {
                continue;
            }

            switch (inst->GetOpcode()) {
            case Opcode::CALL_STATIC:
                TryInlineStatic(inst);
                break;
            // case Opcode::CALL_DYNAMIC:
            //     TryInlineDynamic();
            //     break;
            default:
                assert(false);
            }

            ret_phi_.reset();
            ret_bbs_.clear();
        }
    }

    return true;
}

void Inlining::TryInlineStatic(Inst* inst)
{
    assert(inst->GetOpcode() == Opcode::CALL_STATIC);
    auto call = static_cast<typename Inst::to_inst_type<Opcode::CALL_STATIC>*>(inst);

    UpdateDFGParameters(inst);
    UpdateDFGReturns(inst);
    MoveConstants(call->GetCallee());
    MoveCalleeBlocks(call->GetCallee());
    InsertInlinedGraph(inst);
}

void Inlining::UpdateDFGParameters(Inst* inst)
{
    assert(inst->GetOpcode() == Opcode::CALL_STATIC);

    auto call_inst = static_cast<typename Inst::to_inst_type<Opcode::CALL_STATIC>*>(inst);
    auto callee_start_block = call_inst->GetCallee()->GetStartBasicBlock();

    assert(callee_start_block);

    auto param = callee_start_block->GetFirstInst();
    for (const auto& arg : call_inst->GetInputs()) {
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
void Inlining::UpdateDFGReturns(Inst* inst)
{
    assert(inst->GetOpcode() == Opcode::CALL_STATIC);

    auto call_inst = static_cast<typename Inst::to_inst_type<Opcode::CALL_STATIC>*>(inst);
    auto callee_start_block = call_inst->GetCallee()->GetStartBasicBlock();

    assert(callee_start_block);

    std::vector<Inst*> rets{};
    auto callee_blocks = call_inst->GetCallee()->GetAnalyser()->GetValidPass<RPO>()->GetBlocks();
    for (const auto& bb : callee_blocks) {
        auto last_inst = bb->GetLastInst();
        if (last_inst->IsReturn()) {
            rets.push_back(last_inst);
            ret_bbs_.push_back(bb);
        }
    }

    assert(!rets.empty());

    // only one return type per function
    switch (rets.front()->GetOpcode()) {
    case Opcode::RETURN: {
        Inst* call_ret_res{ nullptr };
        if (rets.size() == 1) {
            assert(Inst::get_num_inputs<Inst::to_inst_type<Opcode::RETURN> >() ==
                   rets.front()->GetInputs().size());
            call_ret_res = rets.front()->GetInputs().front().GetInst();
            call_ret_res->RemoveUser(rets.front());
            rets.front()->GetBasicBlock()->UnlinkInst(rets.front());
        } else {
            ret_phi_ = std::move(Inst::NewInst<Opcode::PHI>());
            call_ret_res = ret_phi_.get();

            assert(call_ret_res != nullptr);

            for (const auto& ret : rets) {
                assert(Inst::get_num_inputs<Inst::to_inst_type<Opcode::RETURN> >() ==
                       ret->GetInputs().size());
                // input is ret's input, but phi's bb is bb, where ret was
                auto ret_input = ret->GetInputs().front().GetInst();
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
    case Opcode::RETURN_VOID: {
        for (const auto& ret : rets) {
            ret->GetBasicBlock()->UnlinkInst(ret);
        }
    } break;
    default:
        assert(false);
    }
}

void Inlining::MoveConstants(Graph* callee)
{
    auto caller_start_bb = graph_->GetStartBasicBlock();
    auto callee_start_bb = callee->GetStartBasicBlock();

    graph_->AppendBasicBlock(caller_start_bb, callee_start_bb);
}

void Inlining::MoveCalleeBlocks(Graph* callee)
{
    callee_start_bb_ = callee->GetStartBasicBlock();
    for (const auto& bb : callee->GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
        graph_->NewBasicBlock(callee->ReleaseBasicBlock(bb->GetId()));
    }
}

void Inlining::InsertInlinedGraph(Inst* call)
{
    auto call_cont_block = graph_->SplitBasicBlock(call);
    auto call_block = call->GetBasicBlock();

    assert(call_block->GetSuccessors() == std::vector<BasicBlock*>{ call_cont_block });
    assert(call_cont_block->GetPredecesors() == std::vector<BasicBlock*>{ call_block });

    if (ret_phi_.get() != nullptr) {
        call_cont_block->PushBackPhi(std::move(ret_phi_));
    }

    assert(call_block->GetSuccessors().size() == 1);

    graph_->RemoveEdge(call_block, call_cont_block);

    assert(call_block->GetSuccessors().empty());
    assert(call_cont_block->GetPredecesors().empty());

    assert(callee_start_bb_->GetPredecesors().empty());
    graph_->AddEdge(call_block, callee_start_bb_);

    for (const auto& ret_bb : ret_bbs_) {
        graph_->AddEdge(ret_bb, call_cont_block);
    }
}
