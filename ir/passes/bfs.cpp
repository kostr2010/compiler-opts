#include "bfs.h"
#include "bb.h"
#include "graph.h"

#include <list>

bool BFS::RunPass()
{
    bfs_bb_.clear();

    std::list<BasicBlock*> queue{};

    auto bb = graph_->GetStartBasicBlock();

    BbVisited::Set(bb->GetBits());
    queue.push_back(bb);

    while (!queue.empty()) {
        bb = queue.front();
        bfs_bb_.push_back(bb);
        queue.pop_front();

        for (auto b : bb->GetSuccessors()) {
            if (BbVisited::Get(*(bb->GetBits()))) {
                continue;
            }

            BbVisited::Set(b->GetBits());
            queue.push_back(b);
        }
    }

    for (auto& b : bfs_bb_) {
        b->ResetBits();
    }

    return true;
}

std::vector<BasicBlock*> BFS::GetBlocks()
{
    return bfs_bb_;
}