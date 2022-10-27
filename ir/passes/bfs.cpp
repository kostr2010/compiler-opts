#include "bfs.h"
#include "bb.h"
#include "graph.h"

#include <list>

bool BFS::RunPass()
{
    bfs_bb_.clear();

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

    for (auto& b : bfs_bb_) {
        analyser->ClearMark<BFS, MarkType::VISITED>(b->GetBits());
    }

    return true;
}

std::vector<BasicBlock*> BFS::GetBlocks()
{
    return bfs_bb_;
}