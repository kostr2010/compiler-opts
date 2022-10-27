#ifndef __BFS_H_INCLUDED__
#define __BFS_H_INCLUDED__

#include "pass.h"

#include <vector>

class BasicBlock;

class BFS : public Pass
{
  public:
    using Marks = Pass::MarksT<1>;
    enum MarkType
    {
        VISITED = 0,
    };

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