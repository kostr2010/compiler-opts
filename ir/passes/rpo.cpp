#include "rpo.h"
#include "bb.h"
#include "graph.h"
#include "marker.h"

bool RPO::RunPass()
{
    ResetState();
    RunPass_(graph_->GetStartBasicBlock());
    ClearMarks();
    SetValid(true);

    return true;
}

std::vector<BasicBlock*> RPO::GetBlocks()
{
    return rpo_bb_;
}

void RPO::RunPass_(BasicBlock* cur_bb)
{
    if (marking::Marker::GetMark<RPO, Marks::VISITED>(cur_bb)) {
        return;
    }

    marking::Marker::SetMark<RPO, Marks::VISITED>(cur_bb);
    rpo_bb_.push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        RunPass_(succ);
    }
}

void RPO::ResetState()
{
    rpo_bb_.clear();
}

void RPO::ClearMarks()
{
    for (auto& bb : rpo_bb_) {
        marking::Marker::ClearMark<RPO, Marks::VISITED>(bb);
    }
}
