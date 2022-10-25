#ifndef __PASS_DFS_INCLUDED__
#define __PASS_DFS_INCLUDED__

#include "mark.h"
#include "pass.h"

#include <vector>

class BasicBlock;

// FIXME:
class DFS : public Pass
{
    using BbVisited = Mark<>;

  public:
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