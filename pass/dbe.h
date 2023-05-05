#ifndef __DBE_H_INCLUDED__
#define __DBE_H_INCLUDED__

#include "pass.h"

class BasicBlock;
class InstBase;

class DBE : public Pass
{
  public:
    DBE(Graph* graph) : Pass(graph)
    {
    }

    bool Run() override;

  private:
    bool RemoveEmpty(BasicBlock* bb);
    bool RemoveUnlinked(BasicBlock* bb);
};

#endif