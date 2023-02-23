#ifndef __PASS_H_INCLUDED__
#define __PASS_H_INCLUDED__

#include "macros.h"
#include "typedefs.h"

#include <type_traits>

class Graph;

class Pass
{
  public:
    template <typename P>
    using IsPass = std::is_base_of<Pass, P>;

    Pass(Graph* g) : graph_{ g }
    {
    }
    GETTER_SETTER(Valid, bool, is_valid_);

    virtual bool RunPass() = 0;

  protected:
    Graph* graph_;

    bool is_valid_ = false;
};

template <typename T>
struct PassTraits
{
    static_assert(Pass::IsPass<T>());
    using is_cfg_sensitive = std::integral_constant<bool, false>;
    using num_marks = std::integral_constant<size_t, 0>;
};

#endif
