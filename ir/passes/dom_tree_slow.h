#ifndef __PASS_DOM_TREE_SLOW_INCLUDED__
#define __PASS_DOM_TREE_SLOW_INCLUDED__

#include "mark.h"
#include "pass.h"

#include <vector>

class BasicBlock;

class DomTreeSlow : public Pass
{
    using BbVisited = Mark<>;

  public:
    DomTreeSlow(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(DomTreeSlow);

    bool RunPass() override;

  private:
};

#endif