#include "po.h"
#include "bb.h"
#include "graph.h"

bool PO::RunPass()
{
    ResetState();
    RunPass_(graph_->GetStartBasicBlock());
    ClearMarks();
    SetValid(true);

    return true;
}

std::vector<BasicBlock*> PO::GetBlocks()
{
    return po_bb_;
}

void PO::RunPass_(BasicBlock* cur_bb)
{
    if (marking::Marker::GetMark<PO, Marks::VISITED>(cur_bb)) {
        return;
    }

    marking::Marker::SetMark<PO, Marks::VISITED>(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        RunPass_(succ);
    }

    po_bb_.push_back(cur_bb);
}

void PO::ResetState()
{
    po_bb_.clear();
}

void PO::ClearMarks()
{
    for (auto& bb : po_bb_) {
        marking::Marker::ClearMark<PO, Marks::VISITED>(bb);
    }
}
