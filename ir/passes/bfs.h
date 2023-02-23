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

    static constexpr size_t GetNumMarks()
    {
        return Marks::N_MARKS;
    }

    BFS(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void ResetStructs();
    void ClearMarks();

    std::vector<BasicBlock*> bfs_bb_{};
};

#endif
