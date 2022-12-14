#include "graph_builder.h"

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

IdType GraphBuilder::NewParameter(ArgNumType arg_num)
{
    assert(graph_ != nullptr);
    assert(graph_->GetStartBasicBlock() != nullptr);

    auto inst = Inst::NewInst<ParamOp>(Opcode::PARAM, arg_num);

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

void GraphBuilder::SetType(IdType id, DataType t)
{
    auto inst = inst_map_[id];
    assert(inst != nullptr);
    inst->SetDataType(t);
}

void GraphBuilder::SetImm(IdType id, ImmType imm)
{
    auto inst = inst_map_[id];
    assert(inst != nullptr);

    switch (inst->GetOpcode()) {
    case Opcode::ADDI:
    case Opcode::SUBI:
    case Opcode::MULI:
    case Opcode::DIVI:
    case Opcode::MODI:
    case Opcode::MINI:
    case Opcode::MAXI:
    case Opcode::SHLI:
    case Opcode::SHRI:
    case Opcode::ASHRI:
    case Opcode::ANDI:
    case Opcode::ORI:
    case Opcode::XORI:
        static_cast<BinaryImmOp*>(inst)->SetImm(imm);
        break;
    case Opcode::IF_IMM:
        static_cast<IfImmOp*>(inst)->SetImm(imm);
        break;
    default:
        assert(false);
    }
}

void GraphBuilder::SetCond(IdType id, CondType c)
{
    auto inst = inst_map_[id];
    assert(inst != nullptr);
    assert(inst->IsCond());

    switch (inst->GetOpcode()) {
    case Opcode::IF:
        static_cast<IfOp*>(inst)->SetCond(c);
        break;
    case Opcode::IF_IMM:
        static_cast<IfImmOp*>(inst)->SetCond(c);
        break;
    case Opcode::CMP:
        static_cast<CompareOp*>(inst)->SetCond(c);
        break;
    default:
        assert(false);
    }
}

void GraphBuilder::ConstructCFG()
{
    for (auto& [bb_id, succs] : bb_succ_map_) {
        // assert(succs.size() <= 2);
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
            assert(inst_map_.find(input_id) != inst_map_.end());

            inst->SetInput(input_idx, inst_map_.at(input_id));
            ++input_idx;
        }

        if (inst->IsTypeSensitive()) {
            inst->CheckInputType();
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

            static_cast<PhiOp*>(inst)->AddInput(input_inst, input_bb);
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
            if (phi->GetInputs().size() > bb->GetPredecesors().size()) {
                LOG_ERROR("BB: " << bb->GetId() << ", INST_ID: " << phi->GetId()
                                 << ", number of phi inputs > number of BB predecessors");
                LOG(phi->GetInputs().size() << " " << bb->GetPredecesors().size() << "\n");
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