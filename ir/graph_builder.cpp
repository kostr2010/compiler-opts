#include "graph_builder.h"
#include "isa/isa.h"

GraphBuilder::GraphBuilder(Graph* g)
{
    SetGraph(g);
}

void GraphBuilder::SetGraph(Graph* g)
{
    assert(g != nullptr);
    graph_ = g;

    bb_map_[g->BB_START_ID] = g->GetStartBasicBlock();

    cur_bb_ = g->GetStartBasicBlock();
    cur_bb_id_ = g->BB_START_ID;

    cfg_constructed = false;
    dfg_constructed = false;
}

IdType GraphBuilder::NewParameter()
{
    assert(graph_ != nullptr);
    assert(graph_->GetStartBasicBlock() != nullptr);
    auto last_inst = graph_->GetStartBasicBlock()->GetLastInst();
    if (last_inst != nullptr) {
        // assert that parameters are passed one after another
        assert(last_inst->IsParam());
    }

    auto inst = InstBase::NewInst<isa::inst::Opcode::PARAM>();

    assert(inst != nullptr);
    auto id = inst->GetId();

    cur_inst_ = inst.get();
    inst_map_[id] = inst.get();

    graph_->GetStartBasicBlock()->PushBackInst(std::move(inst));

    return id;
}

IdType GraphBuilder::NewBlock()
{
    assert(graph_ != nullptr);

    auto bb = graph_->NewBasicBlock();
    auto id = bb->GetId();

    cur_bb_ = bb;
    cur_bb_id_ = id;

    bb_map_[cur_bb_id_] = bb;

    return id;
}

void GraphBuilder::SetSuccessors(IdType bb_id, std::vector<IdType>&& succs)
{
    bb_succ_map_[bb_id] = succs;
}

void GraphBuilder::SetInputs(IdType id, std::vector<std::pair<IdType, IdType> >&& inputs)
{
    assert(inst_map_.find(id) != inst_map_.end());
    assert(inst_map_.at(id)->IsPhi());

    phi_inputs_map_[id].reserve(inputs.size());
    phi_inputs_map_.at(id) = std::move(inputs);
}

void GraphBuilder::SetType(IdType id, InstBase::DataType t)
{
    auto inst = inst_map_[id];
    assert(inst != nullptr);
    inst->SetDataType(t);
}

void GraphBuilder::SetImm(IdType id, size_t pos, ImmType imm)
{
    auto inst = inst_map_[id];
    assert(inst != nullptr);

    auto num_imms = inst->GetNumImms();
    assert(pos < num_imms);

    if (num_imms > 0) {
        static_cast<WithImm*>(inst)->SetImm(pos, imm);
        return;
    }

    UNREACHABLE("trying to set immediate in an instruction with no immediates");
}

void GraphBuilder::SetCond(IdType id, Conditional::Type c)
{
    auto inst = inst_map_[id];
    assert(inst != nullptr);
    assert(inst->IsConditional());

    switch (inst->GetOpcode()) {
    case isa::inst::Opcode::IF:
        static_cast<isa::inst_type::IF*>(inst)->SetCond(c);
        break;
    case isa::inst::Opcode::IF_IMM:
        static_cast<isa::inst_type::IF_IMM*>(inst)->SetCond(c);
        break;
    case isa::inst::Opcode ::CMP:
        static_cast<isa::inst_type::COMPARE*>(inst)->SetCond(c);
        break;
    default:
        UNREACHABLE("trying to set condition in an instruction with no condition");
    }
}

void GraphBuilder::ConstructCFG()
{
    for (auto& [bb_id, succs] : bb_succ_map_) {
        assert(succs.size() <= 2);
        auto bb = bb_map_.at(bb_id);
        for (auto succ : succs) {
            graph_->AddEdge(bb, bb_map_.at(succ));
        }
    }

    cfg_constructed = true;
}

void GraphBuilder::ConstructDFG()
{
    for (auto& [inst_id, inputs] : inst_inputs_map_) {
        assert(inst_map_.find(inst_id) != inst_map_.end());
        auto inst = inst_map_.at(inst_id);
        assert(!inst->IsPhi());

        size_t input_idx = 0;
        for (auto input_id : inputs) {
            if (inst->IsTypeSensitive()) {
                inst->CheckInputType();
            }

            assert(inst_map_.find(input_id) != inst_map_.end());
            auto input_inst = inst_map_.at(input_id);
            if (inst->IsDynamic()) {
                inst->AddInput(input_inst, input_inst->GetBasicBlock());
            } else {
                inst->SetInput(input_idx, input_inst);
            }
            ++input_idx;
        }
    }

    for (auto& [inst_id, inputs] : phi_inputs_map_) {
        assert(inst_map_.find(inst_id) != inst_map_.end());
        auto inst = inst_map_.at(inst_id);
        assert(inst->IsPhi());

        for (auto& input : inputs) {
            auto input_inst_id = input.first;
            auto input_inst_bb = input.second;

            assert(inst_map_.find(input_inst_id) != inst_map_.end());
            auto input_inst = inst_map_.at(input_inst_id);
            auto input_bb = bb_map_.at(input_inst_bb);

            static_cast<isa::inst_type::PHI*>(inst)->AddInput(input_inst, input_bb);
        }
    }

    dfg_constructed = true;
}

bool GraphBuilder::RunChecks()
{
    // FIXME:
    return true;

    if (!(cfg_constructed || dfg_constructed)) {
        LOG_ERROR("can't check graph without CFG or DFG!");
    }

    // check inputs of variable length
    auto analyser = graph_->GetAnalyser();
    auto rpo = analyser->GetValidPass<RPO>()->GetBlocks();
    for (auto bb : rpo) {
        for (auto phi = bb->GetFirstPhi(); phi != nullptr; phi = phi->GetPrev()) {
            if (phi->GetNumInputs() > bb->GetNumPredecessors()) {
                LOG_ERROR("BB: " << bb->GetId() << ", INST_ID: " << phi->GetId()
                                 << ", number of phi inputs > number of BB predecessors");
                LOG(phi->GetNumInputs() << " " << bb->GetNumPredecessors() << "\n");
                return false;
            }

            for (auto input : phi->GetInputs()) {
                if (!input.GetInst()->GetBasicBlock()->Dominates(input.GetSourceBB())) {
                    LOG_ERROR(
                        "BB: "
                        << bb->GetId() << ", INST_ID: " << phi->GetId()
                        << ", phi input's source BB must be dominated by input's original BB");
                    return false;
                }
            }
        }
    }

    // check types

    return true;
}

void GraphBuilder::AddInput(IdType i_id, IdType id)
{
    inst_inputs_map_[i_id].push_back(id);
}