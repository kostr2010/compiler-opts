#ifndef __PASS_DFS_INCLUDED__
#define __PASS_DFS_INCLUDED__

#include "pass.h"

#include <vector>

class BasicBlock;

class DFS : public Pass
{
  public:
    enum Marks
    {
        VISITED = 0,
        N_MARKS,
    };

    static constexpr size_t GetNumMarks()
    {
        return Marks::N_MARKS;
    }

    DFS(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void RunPass_(BasicBlock* cur_bb);
    void ResetStructs();
    void ClearMarks();

    std::vector<BasicBlock*> dfs_bb_{};
};

#endif
