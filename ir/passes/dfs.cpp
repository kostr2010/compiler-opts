#include "dfs.h"
#include "bb.h"
#include "graph.h"

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
    BbVisited::Set(cur_bb->GetBits());
    dfs_bb_.push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        if (BbVisited::Get(*(succ->GetBits()))) {
            continue;
        }
        RunPass_(succ);
    }
}