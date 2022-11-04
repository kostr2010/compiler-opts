#ifndef __PASS_DFS_INCLUDED__
#define __PASS_DFS_INCLUDED__

#include "pass.h"

#include <vector>

class BasicBlock;

class DFS : public Pass
{
  public:
    enum MarkType
    {
        VISITED = 0,
        NUM_MARKS,
    };
    using Marks = Pass::MarksT<MarkType::NUM_MARKS>;

    DFS(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(DFS);

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void RunPass_(BasicBlock* cur_bb);
    void ResetStructs();
    void ClearMarks();

    std::vector<BasicBlock*> dfs_bb_{};
};

#endif
