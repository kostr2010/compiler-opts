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

    template <typename PassT, size_t N>
    static void SetMark(Markable* mark)
    {
        static_assert(N < typename Pass::PassTraits<PassT>::num_marks());
        static_assert((GetMarkOffset<PassT>() + N) < (sizeof(MarkHolder) * BITS_IN_BYTE));
        (*(mark->GetMarkHolder())) |= (1ULL << (GetMarkOffset<PassT>() + N));
    }

    template <typename PassT, size_t N>
    static void ClearMark(Markable* mark)
    {
        static_assert(N < typename Pass::PassTraits<PassT>::num_marks());
        static_assert((GetMarkOffset<PassT>() + N) < (sizeof(MarkHolder) * BITS_IN_BYTE));
        (*(mark->GetMarkHolder())) &= ~(1ULL << (GetMarkOffset<PassT>() + N));
    }

    template <typename PassT, size_t N>
    static bool GetMark(const Markable* mark)
    {
        static_assert(N < typename Pass::PassTraits<PassT>::num_marks());
        static_assert((GetMarkOffset<PassT>() + N) < (sizeof(MarkHolder) * BITS_IN_BYTE));
        return mark->PeekMarkHolder() & (1ULL << (GetMarkOffset<PassT>() + N));
    }
};

};

#endif