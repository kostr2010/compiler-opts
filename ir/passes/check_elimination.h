#ifndef __CHECK_ELIMINATION_H_INCLUDED__
#define __CHECK_ELIMINATION_H_INCLUDED__

#include "pass.h"

class BasicBlock;
class Inst;

class CheckElimination : public Pass
{
  public:
    CheckElimination(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

  private:
};

#endif