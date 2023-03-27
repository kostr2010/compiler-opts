#ifndef __ANALYZER_H_INCLUDED__
#define __ANALYZER_H_INCLUDED__

#include "macros.h"
#include "passes/pass_list.h"
#include "typedefs.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

class Analyser
{
  public:
    Analyser(Graph* graph);

    template <typename PassT>
    PassT* GetPass()
    {
        static_assert(ActivePasses::HasPass<PassT>());
        return static_cast<PassT*>(passes_[ActivePasses::GetPassIdx<PassT>()].get());
    }

    template <typename PassT>
    PassT* GetValidPass()
    {
        static_assert(ActivePasses::HasPass<PassT>());
        if (!IsValid<PassT>()) {
            assert(RunPass<PassT>());
        }
        return static_cast<PassT*>(passes_[ActivePasses::GetPassIdx<PassT>()].get());
    }

    template <typename PassT>
    bool RunPass()
    {
        static_assert(ActivePasses::HasPass<PassT>());
        return passes_[ActivePasses::GetPassIdx<PassT>()]->RunPass();
    }

    template <typename PassT>
    bool IsValid()
    {
        return GetPass<PassT>()->GetValid();
    }

    void InvalidateCFGSensitiveActivePasses();

  private:
    template <size_t... IDS>
    void InvalidateCFGSensitivePasses(std::index_sequence<IDS...>)
    {
        (([&] {
             using Type = ActivePasses::GetPass<IDS>::type;
             if constexpr (Pass::PassTraits<Type>::is_cfg_sensitive::value) {
                 GetPass<Type>()->SetValid(false);
             }
         }()),
         ...);
    }

    template <size_t... IDS>
    void Allocate(Graph* graph, std::index_sequence<IDS...>)
    {
        passes_.reserve(ActivePasses::NumPasses());
        (passes_.emplace_back(new ActivePasses::GetPass<IDS>::type(graph)), ...);
    }

    std::vector<std::unique_ptr<Pass> > passes_;
};

#endif
