#ifndef __REGALLOC_H_INCLUDED__
#define __REGALLOC_H_INCLUDED__

#include "pass.h"

class InstBase;
class BasicBlock;

class Regalloc : public Pass
{
  public:
    Regalloc(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override
    {
        return true;
    }

  private:
};

#endif