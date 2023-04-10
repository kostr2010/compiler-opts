#include "dfs.h"
#include "bb.h"
#include "graph.h"
#include "marker_factory.h"

bool DFS::RunPass()
{
    Markers markers = { marking::MarkerFactory::AcquireMarker() };

    ResetState();
    RunPass_(graph_->GetStartBasicBlock(), markers);

    SetValid(true);

    return true;
}

std::vector<BasicBlock*> DFS::GetBlocks()
{
    return dfs_bb_;
}

void DFS::RunPass_(BasicBlock* cur_bb, const Markers markers)
{
    cur_bb->SetMark(&markers[Marks::VISITED]);
    dfs_bb_.push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        if (succ->ProbeMark(&markers[Marks::VISITED])) {
            continue;
        }
        RunPass_(succ, markers);
    }
}

void DFS::ResetState()
{
    dfs_bb_.clear();
}
