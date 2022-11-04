#include "dfs.h"
#include "bb.h"
#include "graph.h"

bool DFS::RunPass()
{
    ResetStructs();
    RunPass_(graph_->GetStartBasicBlock());
    ClearMarks();
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

    analyser->SetMark<DFS, MarkType::VISITED>(cur_bb->GetBits());
    dfs_bb_.push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        if (analyser->GetMark<DFS, MarkType::VISITED>(*(succ->GetBits()))) {
            continue;
        }
        RunPass_(succ);
    }
}

void DFS::ResetStructs()
{
    dfs_bb_.clear();
}

void DFS::ClearMarks()
{
    auto analyser = graph_->GetAnalyser();
    for (auto& bb : dfs_bb_) {
        analyser->ClearMark<DFS, MarkType::VISITED>(bb->GetBits());
    }
}
