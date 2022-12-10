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
    marking::Marker::SetMark<DFS, Marks::VISITED>(cur_bb->GetMarkHolder());
    dfs_bb_.push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        if (marking::Marker::GetMark<DFS, Marks::VISITED>(*(succ->GetMarkHolder()))) {
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
    for (auto& bb : dfs_bb_) {
        marking::Marker::ClearMark<DFS, Marks::VISITED>(bb->GetMarkHolder());
    }
}
