#include "bfs.h"
#include "bb.h"
#include "graph.h"

#include <list>

bool BFS::RunPass()
{
    ResetState();

    std::list<BasicBlock*> queue{};

    auto bb = graph_->GetStartBasicBlock();

    marking::Marker::SetMark<BFS, Marks::VISITED>(bb);
    queue.push_back(bb);

    while (!queue.empty()) {
        bb = queue.front();
        bfs_bb_.push_back(bb);
        queue.pop_front();

        for (auto b : bb->GetSuccessors()) {
            if (marking::Marker::GetMark<BFS, Marks::VISITED>(b)) {
                continue;
            }

            marking::Marker::SetMark<BFS, Marks::VISITED>(b);
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

void BFS::ResetState()
{
    bfs_bb_.clear();
}

void BFS::ClearMarks()
{
    for (auto& bb : bfs_bb_) {
        marking::Marker::ClearMark<DFS, Marks::VISITED>(bb);
    }
}
