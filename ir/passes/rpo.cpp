#include "rpo.h"
#include "bb.h"
#include "graph.h"

bool RPO::RunPass()
{
    ResetStructs();
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
    auto bits = cur_bb->GetMarkHolder();

    if (marking::Marker::GetMark<RPO, Marks::VISITED>(*bits)) {
        return;
    }

    marking::Marker::SetMark<RPO, Marks::VISITED>(bits);
    rpo_bb_.push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        RunPass_(succ);
    }
}

void RPO::ResetStructs()
{
    rpo_bb_.clear();
}

void RPO::ClearMarks()
{
    for (auto& bb : rpo_bb_) {
        marking::Marker::ClearMark<RPO, Marks::VISITED>(bb->GetMarkHolder());
    }
}
