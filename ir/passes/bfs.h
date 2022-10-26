#ifndef __BFS_H_INCLUDED__
#define __BFS_H_INCLUDED__

#include "mark.h"
#include "pass.h"

#include <vector>

class BasicBlock;

class BFS : public Pass
{
    using BbVisited = Mark<>;

  public:
    BFS(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(BFS);

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    std::vector<BasicBlock*> bfs_bb_{};
};

#endif