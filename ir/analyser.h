#ifndef __ANALYZER_H_INCLUDED__
#define __ANALYZER_H_INCLUDED__

#include "macros.h"
#include "passes/bfs.h"
#include "passes/dfs.h"
#include "passes/dom_tree.h"
#include "passes/dom_tree_slow.h"
#include "passes/rpo.h"
#include "typedefs.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

template <typename... Types>
struct Passes
{
    static_assert((Types::Marks::LEN + ...) <= sizeof(MarkHolderT) * BITS_IN_BYTE);

    NO_DEFAULT_CTOR(Passes);
    NO_DEFAULT_DTOR(Passes);

    static std::vector<std::unique_ptr<Pass> > Allocate(Graph* graph)
    {
        std::vector<std::unique_ptr<Pass> > vec{};
        vec.reserve(sizeof...(Types));
        (vec.emplace_back(new Types(graph)), ...);
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

    template <typename MarkT>
    static constexpr bool HasMark()
    {
        return (std::is_same<MarkT, typename Types::Marks>::value || ...);
    }

    template <typename Type>
    static constexpr size_t GetMarkOffset()
    {
        static_assert(HasPass<Type>());
        static_assert(HasMark<typename Type::Marks>());
        size_t i = 0;
        size_t res = 0;
        (((std::is_same<Type, Types>::value) ? (res = i) : (i += Types::Marks::LEN)), ...);
        return res;
    }
};

using PassesList = Passes<DomTree, DomTreeSlow, DFS, BFS, RPO>;

class Analyser
{
  public:
    Analyser(Graph* graph)
    {
        passes_ = std::move(PassesList::Allocate(graph));
    }
    DEFAULT_DTOR(Analyser);

    template <typename PassT>
    PassT* GetPass()
    {
        static_assert(PassesList::HasPass<PassT>());

        return static_cast<PassT*>(passes_[PassesList::GetIndex<PassT>()].get());
    }

    template <typename PassT>
    PassT* GetValidPass()
    {
        static_assert(PassesList::HasPass<PassT>());
        if (!IsValid<PassT>()) {
            assert(RunPass<PassT>());
        }
        return static_cast<PassT*>(passes_[PassesList::GetIndex<PassT>()].get());
    }

    template <typename PassT>
    bool RunPass()
    {
        static_assert(PassesList::HasPass<PassT>());

        return passes_[PassesList::GetIndex<PassT>()]->RunPass();
    }

    template <typename PassT, size_t N>
    void SetMark(MarkHolderT* ptr)
    {
        static_assert(PassesList::HasPass<PassT>());
        static_assert(PassesList::HasMark<typename PassT::Marks>());
        static_assert(N < PassT::Marks::LEN);
        static_assert(PassesList::GetMarkOffset<PassT>() + N < sizeof(MarkHolderT) * BITS_IN_BYTE);

        constexpr auto OFFT = PassesList::GetMarkOffset<PassT>();
        PassT::Marks::template Set<OFFT, N>(ptr);
    }

    template <typename PassT, size_t N>
    void ClearMark(MarkHolderT* ptr)
    {
        static_assert(PassesList::HasPass<PassT>());
        static_assert(PassesList::HasMark<typename PassT::Marks>());
        static_assert(N < PassT::Marks::LEN);
        static_assert(PassesList::GetMarkOffset<PassT>() + N < sizeof(MarkHolderT) * BITS_IN_BYTE);

        constexpr auto OFFT = PassesList::GetMarkOffset<PassT>();
        PassT::Marks::template Clear<OFFT, N>(ptr);
    }

    template <typename PassT, size_t N>
    bool GetMark(const MarkHolderT& ptr)
    {
        static_assert(PassesList::HasPass<PassT>());
        static_assert(PassesList::HasMark<typename PassT::Marks>());
        static_assert(N < PassT::Marks::LEN);
        static_assert(PassesList::GetMarkOffset<PassT>() + N < sizeof(MarkHolderT) * BITS_IN_BYTE);

        constexpr auto OFFT = PassesList::GetMarkOffset<PassT>();
        return PassT::Marks::template Get<OFFT, N>(ptr);
    }

    template <typename PassT>
    bool IsValid()
    {
        return GetPass<PassT>()->GetValid();
    }

    void InvalidateCfgDependentPasses()
    {
        GetPass<DFS>()->SetValid(false);
        GetPass<BFS>()->SetValid(false);
        GetPass<RPO>()->SetValid(false);
        GetPass<DomTree>()->SetValid(false);
        GetPass<DomTreeSlow>()->SetValid(false);
    }

  private:
    std::vector<std::unique_ptr<Pass> > passes_;
};

#endif