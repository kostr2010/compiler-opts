#include "graph.h"
#include "bb.h"

void Graph::InitStartBlock()
{
    BasicBlock* bb = new BasicBlock(this);
    assert(bb_vector_.empty());
    bb->SetId(BB_START_ID);
    bb_vector_.push_back(bb);
    bb_start_ = bb;
    ++bb_id_counter_;
}

BasicBlock* Graph::NewBasicBlock()
{
    BasicBlock* bb = new BasicBlock(this);
    bb_vector_.push_back(bb);
    bb->SetId(bb_id_counter_++);

    return bb;
}

Inst* Graph::NewParam(ArgNumType arg_num)
{
    ParamOp* inst = Inst::NewInst<ParamOp>(Opcode::PARAM, arg_num);
    inst->SetId(inst_id_counter_++);
    GetStartBasicBlock()->PushBackInst(inst);
    return inst;
}

void Graph::Dump()
{
    for (auto bb : bb_vector_) {
        bb->Dump();
    }
}

void Graph::RPOPass_(std::vector<BasicBlock*>* rpo_bb, std::unordered_set<IdType>* rpo_visited,
                     BasicBlock* cur_bb) const
{
    auto bb_id = cur_bb->GetId();
    if (rpo_visited->find(bb_id) != rpo_visited->end()) {
        return;
    }

    rpo_visited->emplace(bb_id);
    rpo_bb->push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        RPOPass_(rpo_bb, rpo_visited, succ);
    }
}