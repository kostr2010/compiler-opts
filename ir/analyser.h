#ifndef __ANALYZER_H_INCLUDED__
#define __ANALYZER_H_INCLUDED__

#include "macros.h"
#include "passes/bfs.h"
#include "passes/dfs.h"
#include "passes/dom_tree_slow.h"
#include "passes/rpo.h"

#include <iostream>
#include <utility>
#include <vector>

template <typename... Types>
class Passes
{
  public:
    NO_DEFAULT_CTOR(Passes);
    NO_DEFAULT_DTOR(Passes);

    template <typename... Args>
    static std::vector<Pass*> Allocate(Graph* graph)
    {
        std::vector<Pass*> vec{};
        vec.reserve(sizeof...(Types));
        (vec.push_back(new Types(graph)), ...);
        return vec;
    }

    template <typename Type>
    static constexpr size_t GetIndex()
    {
        static_assert(HasPass<Type>());

        size_t i = 0;
        size_t res = 0;
        (((std::is_same<Type, Types>::value) ? (res = i) : (++i)), ...);
        return res;
    }

    template <typename Type>
    static constexpr bool HasPass()
    {
        return (std::is_same<Type, Types>::value || ...);
    }

  private:
};

using PassesList = Passes<DFS, BFS, RPO, DomTreeSlow>;

class Analyser
{
  public:
    Analyser(Graph* graph)
    {
        passes_ = PassesList::Allocate(graph);
    }
    DEFAULT_DTOR(Analyser);

    template <typename PassT>
    PassT* GetPass()
    {
        static_assert(PassesList::HasPass<PassT>());

        return static_cast<PassT*>(passes_[PassesList::GetIndex<PassT>()]);
    }

    template <typename PassT>
    bool RunPass()
    {
        static_assert(PassesList::HasPass<PassT>());

        return passes_[PassesList::GetIndex<PassT>()]->RunPass();
    }

  private:
    std::vector<Pass*> passes_;
};

#endif