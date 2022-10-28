#ifndef __PASS_DOM_TREE_SLOW_INCLUDED__
#define __PASS_DOM_TREE_SLOW_INCLUDED__

#include "pass.h"

class DomTreeSlow : public Pass
{
  public:
    DomTreeSlow(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(DomTreeSlow);

    bool RunPass() override;

  private:
};

#endif