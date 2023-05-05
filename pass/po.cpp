#include "po.h"
#include "ir/bb.h"
#include "ir/graph.h"
#include "utils/marker/marker_factory.h"

bool PO::Run()
{
    Markers markers = { marker::MarkerFactory::AcquireMarker() };

    ResetState();
    Run_(graph_->GetStartBasicBlock(), markers);
    SetValid(true);

    return true;
}

std::vector<BasicBlock*> PO::GetBlocks()
{
    return po_bb_;
}

void PO::Run_(BasicBlock* cur_bb, const Markers markers)
{
    if (cur_bb->SetMark(&markers[Marks::VISITED])) {
        return;
    }

    for (const auto succ : cur_bb->GetSuccessors()) {
        Run_(succ, markers);
    }

    po_bb_.push_back(cur_bb);
}

void PO::ResetState()
{
    po_bb_.clear();
}
