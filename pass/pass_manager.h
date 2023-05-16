#ifndef __PASS_MANAGER_H_INCLUDED__
#define __PASS_MANAGER_H_INCLUDED__

#include "pass_manager_default_passes.h"
#include "utils/macros.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

class PassManager
{
  public:
    PassManager(Graph* graph);

    template <typename PassT>
    PassT* GetPass()
    {
        STATIC_ASSERT(DefaultPasses::HasPass<PassT>());
        return static_cast<PassT*>(passes_[DefaultPasses::GetPassIdx<PassT>()].get());
    }

    template <typename PassT>
    PassT* GetValidPass()
    {
        STATIC_ASSERT(DefaultPasses::HasPass<PassT>());
        if (!IsValid<PassT>()) {
            ASSERT(Run<PassT>());
        }
        return static_cast<PassT*>(passes_[DefaultPasses::GetPassIdx<PassT>()].get());
    }

    template <typename PassT>
    bool Run()
    {
        STATIC_ASSERT(DefaultPasses::HasPass<PassT>());
        return passes_[DefaultPasses::GetPassIdx<PassT>()]->Run();
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
             using Type = typename DefaultPasses::GetPass<IDS>::type;
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
        (passes_.emplace_back(new typename DefaultPasses::GetPass<IDS>::type(graph)), ...);
    }

    std::vector<std::unique_ptr<Pass> > passes_;
};

#endif
