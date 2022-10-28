#include "graph.h"
#include "bb.h"

void Graph::InitStartBlock()
{
    assert(bb_vector_.empty());
    bb_vector_.emplace_back(new BasicBlock(this));

    auto bb = bb_vector_.back().get();
    bb->SetId(BB_START_ID);
    bb_start_ = bb;

    ++bb_id_counter_;
}

BasicBlock* Graph::NewBasicBlock()
{
    bb_vector_.emplace_back(new BasicBlock(this));

    auto bb = bb_vector_.back().get();
    bb->SetId(bb_id_counter_++);

    return bb;
}

void Graph::Dump()
{
    for (const auto& bb : bb_vector_) {
        bb->Dump();
    }
}

void Graph::ClearDominators()
{
    for (const auto& bb : bb_vector_) {
        bb->ClearImmDominator();
        bb->ClearDominators();
        bb->ClearDominated();
    }
}

void Graph::AddEdge(BasicBlock* from, BasicBlock* to)
{
    assert(to != nullptr);
    assert(from != nullptr);

    from->AddSucc(to);
    to->AddPred(from);
}
