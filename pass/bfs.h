#ifndef __BFS_H_INCLUDED__
#define __BFS_H_INCLUDED__

#include "pass.h"
#include "utils/marker/marker.h"

#include <vector>

class BasicBlock;

class BFS : public Pass
{
  public:
    enum Marks
    {
        VISITED = 0,
        N_MARKS,
    };
    using Markers = marker::Markers<Marks::N_MARKS>;

    BFS(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void ResetState();

    std::vector<BasicBlock*> bfs_bb_{};
};

template <>
struct Pass::PassTraits<BFS>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
};

#endif
