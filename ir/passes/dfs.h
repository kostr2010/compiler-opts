#ifndef __PASS_DFS_INCLUDED__
#define __PASS_DFS_INCLUDED__

#include "pass.h"

#include <vector>

class BasicBlock;

class DFS : public Pass
{
  public:
    using Marks = Pass::MarksT<1>;
    enum MarkType
    {
        VISITED = 0,
    };

    DFS(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(DFS);

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void RunPass_(BasicBlock* cur_bb);

    std::vector<BasicBlock*> dfs_bb_{};
};

#endif