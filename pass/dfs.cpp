#include "dfs.h"
#include "ir/bb.h"
#include "ir/graph.h"

bool DFS::Run()
{
    Markers markers{};

    ResetState();
    Run_(graph_->GetStartBasicBlock(), markers);

    SetValid(true);

    return true;
}

std::vector<BasicBlock*> DFS::GetBlocks()
{
    return dfs_bb_;
}

void DFS::Run_(BasicBlock* cur_bb, const Markers markers)
{
    cur_bb->SetMark(&markers[Marks::VISITED]);
    dfs_bb_.push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        if (succ->ProbeMark(&markers[Marks::VISITED])) {
            continue;
        }
        Run_(succ, markers);
    }
}

void DFS::ResetState()
{
    dfs_bb_.clear();
}
