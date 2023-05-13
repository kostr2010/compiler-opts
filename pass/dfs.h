#ifndef __PASS_DFS_INCLUDED__
#define __PASS_DFS_INCLUDED__

#include "pass.h"
#include "utils/marker/marker.h"

#include <vector>

class BasicBlock;

class DFS : public Pass
{
  public:
    using is_cfg_sensitive = std::true_type;

    enum Marks
    {
        VISITED = 0,
        N_MARKS,
    };
    using Markers = marker::Markers<Marks::N_MARKS>;

    DFS(Graph* graph) : Pass(graph)
    {
    }

    bool Run() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void Run_(BasicBlock* cur_bb, const Markers markers);
    void ResetState();

    std::vector<BasicBlock*> dfs_bb_{};
};

#endif
