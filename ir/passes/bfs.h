#ifndef __BFS_H_INCLUDED__
#define __BFS_H_INCLUDED__

#include "mark.h"
#include "pass.h"

class BasicBlock;

// FIXME:
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
    void RunPass_(BasicBlock* cur_bb);

    std::vector<BasicBlock*> bfs_bb_{};
};

#endif