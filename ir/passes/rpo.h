#ifndef __PASS_RPO_INCLUDED__
#define __PASS_RPO_INCLUDED__

#include "pass.h"

#include <vector>

class BasicBlock;

class RPO : public Pass
{
  public:
    using Marks = Pass::MarksT<1>;
    enum MarkType
    {
        VISITED = 0,
    };

    RPO(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(RPO);

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void RunPass_(BasicBlock* cur_bb);

    std::vector<BasicBlock*> rpo_bb_{};
};

#endif