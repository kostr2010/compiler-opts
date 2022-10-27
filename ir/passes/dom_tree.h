#ifndef __PASS_DOM_TREE_INCLUDED__
#define __PASS_DOM_TREE_INCLUDED__

#include "pass.h"

#include <vector>

class BasicBlock;

// FIXME:
class DomTree : public Pass
{
  public:
    using Marks = Pass::MarksT<0>;

    DomTree(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(DomTree);

    bool RunPass() override;

  private:
};

#endif