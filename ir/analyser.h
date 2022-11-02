#ifndef __ANALYZER_H_INCLUDED__
#define __ANALYZER_H_INCLUDED__

#include "macros.h"
#include "passes/bfs.h"
#include "passes/dfs.h"
#include "passes/dom_tree.h"
#include "passes/dom_tree_slow.h"
#include "passes/pass.h"
#include "passes/rpo.h"
#include "typedefs.h"

#include <cassert>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

template <typename... Types>
class Passes
{
  private:
    template <typename Type, typename... Pack>
    struct is_one_of;

    template <typename Type>
    struct is_one_of<Type> : std::false_type
    {
    };

    template <typename Type, typename... Pack>
    struct is_one_of<Type, Type, Pack...> : std::true_type
    {
    };

    template <typename Type, typename T0, typename... Pack>
    struct is_one_of<Type, T0, Pack...> : is_one_of<Type, Pack...>
    {
    };

    template <typename, typename = void>
    struct has_marks_field : std::false_type
    {
    };

    template <typename Type>
    struct has_marks_field<Type, std::void_t<typename Type::Marks> > : std::true_type
    {
    };

    template <typename Type, size_t IDX, typename... Pack>
    struct get_pass_idx;

    template <typename Type, size_t IDX, typename T, typename... Pack>
    struct get_pass_idx<Type, IDX, T, Pack...> : get_pass_idx<Type, IDX + 1, Pack...>
    {
    };

    template <typename Type, size_t IDX, typename... Pack>
    struct get_pass_idx<Type, IDX, Type, Pack...> : std::integral_constant<size_t, IDX>
    {
    };

    template <typename, typename = void>
    struct get_mark_length : std::integral_constant<size_t, 0>
    {
    };

    template <typename Type>
    struct get_mark_length<Type, std::void_t<typename Type::Marks> >
        : std::integral_constant<size_t, Type::Marks::LEN>
    {
    };

    template <typename Type, size_t IDX, typename T, typename... Pack>
    struct get_mark_offt : get_mark_offt<Type, IDX + get_mark_length<T>(), Pack...>
    {
    };

    template <typename Type, size_t IDX, typename... Pack>
    struct get_mark_offt<Type, IDX, Type, Pack...> : std::integral_constant<size_t, IDX>
    {
    };

    static_assert((get_mark_length<Types>() + ...) <= sizeof(MarkHolderT) * BITS_IN_BYTE);

  public:
    NO_DEFAULT_CTOR(Passes);
    NO_DEFAULT_DTOR(Passes);

    template <typename Type>
    using HasPass = is_one_of<Type, Types...>;
    template <typename Type>
    using HasMarks = has_marks_field<Type>;
    template <typename Type>
    using GetPassIdx = get_pass_idx<Type, 0, Types...>;
    template <typename Type>
    using GetMarkOfft = get_mark_offt<Type, 0, Types...>;

    static auto Allocate(Graph* graph)
    {
        std::vector<std::unique_ptr<Pass> > vec{};
        vec.reserve(sizeof...(Types));
        (vec.emplace_back(new Types(graph)), ...);
        return vec;
    }
};

using PassesList = Passes<DomTree, DomTreeSlow, DFS, BFS, RPO>;

class Analyser
{
  public:
    Analyser(Graph* graph) : passes_(std::move(PassesList::Allocate(graph)))
    {
    }
    DEFAULT_DTOR(Analyser);

    template <typename PassT>
    PassT* GetPass()
    {
        static_assert(PassesList::HasPass<PassT>());

        return static_cast<PassT*>(passes_[PassesList::GetPassIdx<PassT>()].get());
    }

    template <typename PassT>
    PassT* GetValidPass()
    {
        static_assert(PassesList::HasPass<PassT>());
        if (!IsValid<PassT>()) {
            assert(RunPass<PassT>());
        }
        return static_cast<PassT*>(passes_[PassesList::GetPassIdx<PassT>()].get());
    }

    template <typename PassT>
    bool RunPass()
    {
        static_assert(PassesList::HasPass<PassT>());

        return passes_[PassesList::GetPassIdx<PassT>()]->RunPass();
    }

    template <typename PassT, size_t N>
    void SetMark(MarkHolderT* ptr)
    {
        static_assert(PassesList::HasPass<PassT>());
        static_assert(PassesList::HasMarks<PassT>());
        static_assert(N < PassT::Marks::LEN);
        static_assert(PassesList::GetMarkOfft<PassT>() + N < sizeof(MarkHolderT) * BITS_IN_BYTE);

        constexpr auto OFFT = PassesList::GetMarkOfft<PassT>();
        PassT::Marks::template Set<OFFT, N>(ptr);
    }

    template <typename PassT, size_t N>
    void ClearMark(MarkHolderT* ptr)
    {
        static_assert(PassesList::HasPass<PassT>());
        static_assert(PassesList::HasMarks<PassT>());
        static_assert(N < PassT::Marks::LEN);
        static_assert(PassesList::GetMarkOfft<PassT>() + N < sizeof(MarkHolderT) * BITS_IN_BYTE);

        constexpr auto OFFT = PassesList::GetMarkOfft<PassT>();
        PassT::Marks::template Clear<OFFT, N>(ptr);
    }

    template <typename PassT, size_t N>
    bool GetMark(const MarkHolderT& ptr)
    {
        static_assert(PassesList::HasPass<PassT>());
        static_assert(PassesList::HasMarks<PassT>());
        static_assert(N < PassT::Marks::LEN);
        static_assert(PassesList::GetMarkOfft<PassT>() + N < sizeof(MarkHolderT) * BITS_IN_BYTE);

        constexpr auto OFFT = PassesList::GetMarkOfft<PassT>();
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
    const std::vector<std::unique_ptr<Pass> > passes_;
};

#endif