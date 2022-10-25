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

void Graph::ClearDominators()
{
    for (auto bb : bb_vector_) {
        bb->ClearImmDominator();
        bb->ClearDominators();
        bb->ClearDominated();
    }
}

void Graph::UnbindBasicBlock(BasicBlock* bb)
{
    assert(bb != nullptr);

    for (auto pred : bb->GetPredecesors()) {
        pred->RemoveSucc(bb);
    }

    for (auto succ : bb->GetSuccessors()) {
        succ->RemovePred(bb);
    }
}

void Graph::BindBasicBlock(BasicBlock* bb)
{
    assert(bb != nullptr);

    for (auto pred : bb->GetPredecesors()) {
        pred->AddSucc(bb);
    }

    for (auto succ : bb->GetSuccessors()) {
        succ->AddPred(bb);
    }
}

void Graph::AddEdge(BasicBlock* from, BasicBlock* to)
{
    assert(to != nullptr);
    assert(from != nullptr);

    from->AddSucc(to);
    to->AddPred(from);
}
