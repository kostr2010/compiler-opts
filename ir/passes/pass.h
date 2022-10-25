#ifndef __PASS_H_INCLUDED__
#define __PASS_H_INCLUDED__

#include "macros.h"

class Graph;

class Pass
{
  public:
    Pass(Graph* g) : graph_{ g }
    {
    }
    DEFAULT_DTOR(Pass);

    virtual bool RunPass() = 0;

  protected:
    Graph* graph_;
};

#endif