#include "linear_order.h"
#include "bb.h"
#include "graph.h"

bool LinearOrder::RunPass()
{
    ResetState();
    RunPass_(graph_->GetStartBasicBlock());
    ClearMarks();
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

void LinearOrder::ClearMarks()
{
    for (auto& bb : linear_bb_) {
        marking::Marker::ClearMark<LinearOrder, Marks::VISITED>(bb);
    }
}
