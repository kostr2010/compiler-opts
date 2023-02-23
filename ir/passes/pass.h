#ifndef __PASS_H_INCLUDED__
#define __PASS_H_INCLUDED__

#include "macros.h"
#include "typedefs.h"

class Graph;

class Pass
{
  public:
    Pass(Graph* g) : graph_{ g }
    {
    }
    GETTER_SETTER(Valid, bool, is_valid_);

    virtual bool RunPass() = 0;

    static constexpr size_t GetNumMarks()
    {
        return 0;
    }

  protected:
    Graph* graph_;

    bool is_valid_ = false;
};

#endif
