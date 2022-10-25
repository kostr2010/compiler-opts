#ifndef __PASS_PO_INCLUDED__
#define __PASS_PO_INCLUDED__

#include "mark.h"
#include "pass.h"

#include <vector>

class BasicBlock;

// FIXME:
class PO : public Pass
{
    using BbVisited = Mark<>;

  public:
    PO(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(PO);

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void RunPass_(BasicBlock* cur_bb);

    std::vector<BasicBlock*> po_bb_{};
};

#endif