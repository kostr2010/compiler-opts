#include "dfs.h"
#include "bb.h"
#include "graph.h"

#include <iostream>

bool DFS::RunPass()
{
    dfs_bb_.clear();

    RunPass_(graph_->GetStartBasicBlock());

    for (auto& bb : dfs_bb_) {
        bb->ResetBits();
    }

    return true;
}

std::vector<BasicBlock*> DFS::GetBlocks()
{
    return dfs_bb_;
}

void DFS::RunPass_(BasicBlock* cur_bb)
{
    auto bits = cur_bb->GetBits();

    if (BbVisited::Get(*bits)) {
        return;
    }

    BbVisited::Set(bits);
    dfs_bb_.push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        RunPass_(succ);
    }
}