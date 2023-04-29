#include "rpo.h"
#include "ir/bb.h"
#include "ir/graph.h"
#include "utils/marker/marker_factory.h"

bool RPO::RunPass()
{
    Markers markers = { marker::MarkerFactory::AcquireMarker() };

    ResetState();
    RunPass_(graph_->GetStartBasicBlock(), markers);
    SetValid(true);

    return true;
}

std::vector<BasicBlock*> RPO::GetBlocks()
{
    return rpo_bb_;
}

void RPO::RunPass_(BasicBlock* cur_bb, const Markers markers)
{
    if (cur_bb->SetMark(&markers[Marks::VISITED])) {
        return;
    }

    rpo_bb_.push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        RunPass_(succ, markers);
    }
}

void RPO::ResetState()
{
    rpo_bb_.clear();
}
