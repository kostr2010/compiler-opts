#ifndef __ANALYZER_H_INCLUDED__
#define __ANALYZER_H_INCLUDED__

#include "default_passes.h"
#include "macros.h"
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
        static_assert(DefaultPasses::HasPass<PassT>());
        return static_cast<PassT*>(passes_[DefaultPasses::GetPassIdx<PassT>()].get());
    }

    template <typename PassT>
    PassT* GetValidPass()
    {
        static_assert(DefaultPasses::HasPass<PassT>());
        if (!IsValid<PassT>()) {
            assert(RunPass<PassT>());
        }
        return static_cast<PassT*>(passes_[DefaultPasses::GetPassIdx<PassT>()].get());
    }

    template <typename PassT>
    bool RunPass()
    {
        static_assert(DefaultPasses::HasPass<PassT>());
        return passes_[DefaultPasses::GetPassIdx<PassT>()]->RunPass();
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
             using Type = DefaultPasses::GetPass<IDS>::type;
             if constexpr (Pass::PassTraits<Type>::is_cfg_sensitive::value) {
                 GetPass<Type>()->SetValid(false);
             }
         }()),
         ...);
    }

    template <size_t... IDS>
    void Allocate(Graph* graph, std::index_sequence<IDS...>)
    {
        passes_.reserve(DefaultPasses::NumPasses());
        (passes_.emplace_back(new DefaultPasses::GetPass<IDS>::type(graph)), ...);
    }

    std::vector<std::unique_ptr<Pass> > passes_;
};

#endif
