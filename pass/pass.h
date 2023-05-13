#ifndef __PASS_H_INCLUDED__
#define __PASS_H_INCLUDED__

#include "utils/macros.h"
#include "utils/type_helpers.h"

#include <memory>

class Graph;

class Pass
{
    struct PassTraitsBase
    {
      protected:
        template <typename T>
        using __is_cfg_sensitive = typename T::is_cfg_sensitive;
    };

  public:
    template <typename P>
    using is_pass = std::is_base_of<Pass, P>;

    template <typename T>
    struct PassTraits : public PassTraitsBase
    {
        static_assert(Pass::is_pass<T>());
        using is_cfg_sensitive =
            type_helpers::detected_or_t<std::false_type, __is_cfg_sensitive, T>;
    };

    Pass(Graph* g) : graph_{ g }
    {
    }
    virtual ~Pass() = default;
    GETTER_SETTER(Valid, bool, is_valid_);

    virtual bool Run() = 0;

  protected:
    Graph* graph_;

    bool is_valid_ = false;
};

#endif
