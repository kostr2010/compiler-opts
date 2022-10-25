#ifndef __DOMINATOR_TREE_H_INCLUDED__
#define __DOMINATOR_TREE_H_INCLUDED__

#include "macros.h"

class DominatorTree
{
  public:
    SINGLETON_GET_INSTANCE_IMPLEMENT(DominatorTree);
    DEFAULT_DTOR(DominatorTree);

    private:
    DEFAULT_CTOR(DominatorTree);
};

#endif