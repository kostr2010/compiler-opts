#include "po.h"
#include "bb.h"
#include "graph.h"
#include "marker/marker_factory.h"

bool PO::RunPass()
{
    Markers markers = { marker::MarkerFactory::AcquireMarker() };

    ResetState();
    RunPass_(graph_->GetStartBasicBlock(), markers);
    SetValid(true);

    return true;
}

std::vector<BasicBlock*> PO::GetBlocks()
{
    return po_bb_;
}

void PO::RunPass_(BasicBlock* cur_bb, const Markers markers)
{
    if (cur_bb->SetMark(&markers[Marks::VISITED])) {
        return;
    }

    for (const auto succ : cur_bb->GetSuccessors()) {
        RunPass_(succ, markers);
    }

    po_bb_.push_back(cur_bb);
}

void PO::ResetState()
{
    po_bb_.clear();
}
