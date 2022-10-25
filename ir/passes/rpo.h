#ifndef __PASS_RPO_INCLUDED__
#define __PASS_RPO_INCLUDED__

#include "mark.h"
#include "pass.h"

#include <vector>

class BasicBlock;

class RPO : public Pass
{
    using BbVisited = Mark<>;

  public:
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