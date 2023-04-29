#include "bfs.h"
#include "ir/bb.h"
#include "ir/graph.h"
#include "utils/marker/marker_factory.h"

#include <list>

bool BFS::RunPass()
{
    ResetState();

    Markers markers = { marker::MarkerFactory::AcquireMarker() };

    std::list<BasicBlock*> queue{};

    auto bb = graph_->GetStartBasicBlock();
    bb->SetMark(&markers[Marks::VISITED]);
    queue.push_back(bb);

    while (!queue.empty()) {
        bb = queue.front();
        bfs_bb_.push_back(bb);
        queue.pop_front();

        for (auto b : bb->GetSuccessors()) {
            if (b->SetMark(&markers[Marks::VISITED])) {
                continue;
            }

            queue.push_back(b);
        }
    }

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
