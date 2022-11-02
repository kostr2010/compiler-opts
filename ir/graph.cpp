#include "graph.h"
#include "bb.h"

void Graph::InitStartBlock()
{
    assert(bb_vector_.empty());
    assert(BB_START_ID == bb_id_counter_);

    bb_vector_.emplace_back(new BasicBlock(bb_id_counter_++));
    bb_start_ = bb_vector_.back().get();
}

BasicBlock* Graph::NewBasicBlock()
{
    bb_vector_.emplace_back(new BasicBlock(bb_id_counter_++));
    return bb_vector_.back().get();
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

    analyser_.InvalidateCfgDependentPasses();
}

void Graph::RemoveEdge(BasicBlock* from, BasicBlock* to)
{
    assert(to != nullptr);
    assert(from != nullptr);

    from->RemoveSucc(to);
    to->RemovePred(from);

    analyser_.InvalidateCfgDependentPasses();
}
