#ifndef __BFS_H_INCLUDED__
#define __BFS_H_INCLUDED__

#include "pass.h"

#include <vector>

class BasicBlock;

class BFS : public Pass
{
  public:
    enum MarkType
    {
        VISITED = 0,
        NUM_MARKS,
    };
    using Marks = Pass::MarksT<MarkType::NUM_MARKS>;

    BFS(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(BFS);

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void ResetStructs();
    void ClearMarks();

    std::vector<BasicBlock*> bfs_bb_{};
};

#endif
