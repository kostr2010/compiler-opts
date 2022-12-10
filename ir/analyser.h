#ifndef __ANALYZER_H_INCLUDED__
#define __ANALYZER_H_INCLUDED__

#include "macros.h"
#include "passes/pass_list.h"
#include "typedefs.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

class Analyser
{
  public:
    Analyser(Graph* graph) : passes_(std::move(ActivePasses::Allocate(graph)))
    {
    }
    DEFAULT_DTOR(Analyser);

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

    void InvalidateCfgDependentActivePasses();

  private:
    const std::vector<std::unique_ptr<Pass> > passes_;
};

#endif
