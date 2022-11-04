#include "bfs.h"
#include "bb.h"
#include "graph.h"

#include <list>

bool BFS::RunPass()
{
    ResetStructs();

    std::list<BasicBlock*> queue{};

    auto bb = graph_->GetStartBasicBlock();
    auto analyser = graph_->GetAnalyser();

    analyser->SetMark<BFS, MarkType::VISITED>(bb->GetBits());
    queue.push_back(bb);

    while (!queue.empty()) {
        bb = queue.front();
        bfs_bb_.push_back(bb);
        queue.pop_front();

        for (auto b : bb->GetSuccessors()) {
            auto bits = b->GetBits();
            if (analyser->GetMark<BFS, MarkType::VISITED>(*bits)) {
                continue;
            }

            analyser->SetMark<BFS, MarkType::VISITED>(bits);
            queue.push_back(b);
        }
    }

    ClearMarks();
    SetValid(true);

    return true;
}

std::vector<BasicBlock*> BFS::GetBlocks()
{
    return bfs_bb_;
}

void BFS::ResetStructs()
{
    bfs_bb_.clear();
}

void BFS::ClearMarks()
{
    auto analyser = graph_->GetAnalyser();
    for (auto& bb : bfs_bb_) {
        analyser->ClearMark<DFS, MarkType::VISITED>(bb->GetBits());
    }
}
