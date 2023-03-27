#ifndef __BFS_H_INCLUDED__
#define __BFS_H_INCLUDED__

#include "pass.h"

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

    BFS(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void ResetState();
    void ClearMarks();

    std::vector<BasicBlock*> bfs_bb_{};
};

template <>
struct Pass::PassTraits<BFS>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
    using num_marks = std::integral_constant<size_t, BFS::Marks::N_MARKS>;
};

#endif
