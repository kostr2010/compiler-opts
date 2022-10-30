#include "dfs.h"
#include "bb.h"
#include "graph.h"

bool DFS::RunPass()
{
    dfs_bb_.clear();

    RunPass_(graph_->GetStartBasicBlock());

    auto analyser = graph_->GetAnalyser();
    for (auto& bb : dfs_bb_) {
        analyser->ClearMark<DFS, MarkType::VISITED>(bb->GetBits());
    }

    SetValid(true);

    return true;
}

std::vector<BasicBlock*> DFS::GetBlocks()
{
    return dfs_bb_;
}

void DFS::RunPass_(BasicBlock* cur_bb)
{
    auto analyser = graph_->GetAnalyser();
    auto bits = cur_bb->GetBits();

    if (analyser->GetMark<DFS, MarkType::VISITED>(*bits)) {
        return;
    }

    analyser->SetMark<DFS, MarkType::VISITED>(bits);
    dfs_bb_.push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        RunPass_(succ);
    }
}