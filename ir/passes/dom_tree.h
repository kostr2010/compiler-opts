#ifndef __PASS_DOM_TREE_INCLUDED__
#define __PASS_DOM_TREE_INCLUDED__

#include "mark.h"
#include "pass.h"

#include <vector>

class BasicBlock;

// FIXME:
class DomTree : public Pass
{
    using BbVisited = Mark<>;

  public:
    DomTree(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(DomTree);

    bool RunPass() override;

  private:
};

#endif