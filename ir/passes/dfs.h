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
    using Markers = marker::Markers<Marks::N_MARKS>;

    DFS(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void RunPass_(BasicBlock* cur_bb, const Markers markers);
    void ResetState();

    std::vector<BasicBlock*> dfs_bb_{};
};

template <>
struct Pass::PassTraits<DFS>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
};

#endif
