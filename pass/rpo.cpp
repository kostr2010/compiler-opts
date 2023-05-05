#include "rpo.h"
#include "ir/bb.h"
#include "ir/graph.h"
#include "utils/marker/marker_factory.h"

bool RPO::Run()
{
    Markers markers = { marker::MarkerFactory::AcquireMarker() };

    ResetState();
    Run_(graph_->GetStartBasicBlock(), markers);
    SetValid(true);

    return true;
}

std::vector<BasicBlock*> RPO::GetBlocks()
{
    return rpo_bb_;
}

void RPO::Run_(BasicBlock* cur_bb, const Markers markers)
{
    if (cur_bb->SetMark(&markers[Marks::VISITED])) {
        return;
    }

    rpo_bb_.push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        Run_(succ, markers);
    }
}

void RPO::ResetState()
{
    rpo_bb_.clear();
}
