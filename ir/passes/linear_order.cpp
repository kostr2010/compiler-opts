#include "linear_order.h"
#include "bb.h"
#include "graph.h"
#include "marker/marker_factory.h"

bool LinearOrder::RunPass()
{
    Markers markers = { marker::MarkerFactory::AcquireMarker() };

    ResetState();
    RunPass_(graph_->GetStartBasicBlock());
    SetValid(true);

    return true;
}

std::vector<BasicBlock*> LinearOrder::GetBlocks()
{
    return linear_bb_;
}

void LinearOrder::RunPass_(BasicBlock* cur_bb)
{
    cur_bb->Dump();
}

void LinearOrder::ResetState()
{
    linear_bb_.clear();
}
