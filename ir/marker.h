#ifndef MARKER_H_INCLUDED
#define MARKER_H_INCLUDED

#include "default_passes.h"
#include "macros.h"
#include "markable.h"
#include "typedefs.h"

namespace marking {

class Marker
{
  private:
    template <typename Type, size_t ACC, typename Tail>
    struct get_mark_offset;

    template <typename Type, size_t ACC, typename Head, class... Tail>
    struct get_mark_offset<Type, ACC, std::tuple<Head, Tail...> >
        : get_mark_offset<Type, ACC + typename Pass::PassTraits<Head>::num_marks(),
                          std::tuple<Tail...> >
    {};

    template <typename Type, size_t ACC, typename... Tail>
    struct get_mark_offset<Type, ACC, std::tuple<Type, Tail...> >
        : std::integral_constant<size_t, ACC>
    {};

  public:
    template <typename Type>
    using GetMarkOffset = get_mark_offset<Type, 0, DefaultPasses::Passes>;

  private:
    template <typename PassT, size_t N>
    using GetMarkBits =
        std::integral_constant<MarkHolder, (MarkHolder)(1) << (GetMarkOffset<PassT>() + N)>;

  public:
    template <typename PassT, size_t N>
    static bool ProbeMark(const Markable* obj)
    {
        static_assert(N < typename Pass::PassTraits<PassT>::num_marks());
        static_assert((GetMarkOffset<PassT>() + N) < (sizeof(MarkHolder) * BITS_IN_BYTE));
        return obj->PeekMarkHolder() & GetMarkBits<PassT, N>();
    }

    // true if mark was set, false otherwise
    template <typename PassT, size_t N>
    static bool SetMark(Markable* obj)
    {
        static_assert(N < typename Pass::PassTraits<PassT>::num_marks());
        static_assert((GetMarkOffset<PassT>() + N) < (sizeof(MarkHolder) * BITS_IN_BYTE));
        bool was_set = ProbeMark<PassT, N>(obj);
        *(obj->GetMarkHolder()) |= GetMarkBits<PassT, N>();
        return was_set;
    }

    // true if mark was set, false otherwise
    template <typename PassT, size_t N>
    static bool ClearMark(Markable* obj)
    {
        static_assert(N < typename Pass::PassTraits<PassT>::num_marks());
        static_assert((GetMarkOffset<PassT>() + N) < (sizeof(MarkHolder) * BITS_IN_BYTE));
        bool was_set = ProbeMark<PassT, N>(obj);
        *(obj->GetMarkHolder()) &= ~GetMarkBits<PassT, N>();
        return was_set;
    }
};

};

#endif