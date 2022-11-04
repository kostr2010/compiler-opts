#ifndef __PASS_RPO_INCLUDED__
#define __PASS_RPO_INCLUDED__

#include "pass.h"

#include <vector>

class BasicBlock;

class RPO : public Pass
{
  public:
    enum MarkType
    {
        VISITED = 0,
        NUM_MARKS,
    };
    using Marks = Pass::MarksT<MarkType::NUM_MARKS>;

    RPO(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(RPO);

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void RunPass_(BasicBlock* cur_bb);
    void ResetStructs();
    void ClearMarks();

    std::vector<BasicBlock*> rpo_bb_{};
};

#endif
