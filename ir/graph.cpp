#include "graph.h"
#include "bb.h"

void Graph::InitStartBlock()
{
    BasicBlock* bb = new BasicBlock(this);
    bb->SetId(BB_START_ID);

    bb_vector_.at(BB_START_ID) = bb;
    bb_start_ = bb;
    ++bb_id_counter_;
}

void Graph::InitEndBlock()
{
    BasicBlock* bb = new BasicBlock(this);
    bb->SetId(BB_END_ID);

    bb_vector_.at(BB_END_ID) = bb;
    bb_end_ = bb;
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