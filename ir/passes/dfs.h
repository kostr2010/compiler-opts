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

template <>
struct PassTraits<DFS>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
    using num_marks = std::integral_constant<size_t, DFS::Marks::N_MARKS>;
};

#endif
