#ifndef __UTILS_TYPE_SEQUENCE_INCLUDED__
#define __UTILS_TYPE_SEQUENCE_INCLUDED__

#include <cstddef>
#include <cstdint>
#include <type_traits>

// type wrapper for type pack, which is not type in c++
namespace type_sequence {
template <typename... Ts>
struct TypeSequence
{};

using EmptySequence = TypeSequence<>;

// Trio of functions that allow to create aggregate types
// Borrowed from LISP
template <typename Seq>
struct head;

template <typename T, typename... Ts>
struct head<TypeSequence<T, Ts...> >
{
    using type = T;
};

template <typename Seq>
using head_t = typename head<Seq>::type;

template <typename Seq>
struct tail;

template <typename T, typename... Ts>
struct tail<TypeSequence<T, Ts...> >
{
    using type = TypeSequence<Ts...>;
};

template <typename Seq>
using tail_t = typename tail<Seq>::type;

template <typename Seq, typename T>
struct prepend;

template <typename T, typename... Ts>
struct prepend<TypeSequence<Ts...>, T>
{
    using type = TypeSequence<T, Ts...>;
};

template <typename Seq, typename T>
using prepend_t = typename prepend<Seq, T>::type;

template <typename Seq, typename T>
struct append;

template <typename T, typename... Ts>
struct append<TypeSequence<Ts...>, T>
{
    using type = TypeSequence<Ts..., T>;
};

template <typename Seq, typename T>
using append_t = typename append<Seq, T>::type;

template <typename Seq1, typename Seq2>
struct cat;

template <typename... Ts1, typename... Ts2>
struct cat<TypeSequence<Ts1...>, TypeSequence<Ts2...> >
{
    using type = TypeSequence<Ts1..., Ts2...>;
};

template <typename Seq1, typename Seq2>
using cat_t = typename cat<Seq1, Seq2>::type;

template <typename Seq>
struct len;

template <>
struct len<EmptySequence> : std::integral_constant<size_t, 0>
{};

template <typename... Ts>
struct len<TypeSequence<Ts...> >
    : std::integral_constant<size_t, len<tail_t<TypeSequence<Ts...> > >::value + 1>
{};

template <typename Seq, typename T>
struct find;

template <typename T, typename... Ts>
struct find<TypeSequence<Ts...>, T> : find<tail_t<TypeSequence<Ts...> >, T>
{};

template <typename T, typename... Ts>
struct find<TypeSequence<T, Ts...>, T>
{
    static constexpr decltype(true) value = true;
    using type = T;
};

template <typename T>
struct find<EmptySequence, T>
{
    static constexpr decltype(false) value = false;
};

namespace {
template <bool RES, typename Cur, typename Tail, template <typename> typename Predicate>
struct _find_if;

template <typename T, typename... Ts, template <typename> typename Predicate>
struct _find_if<true, T, TypeSequence<Ts...>, Predicate>
{
    static constexpr decltype(true) value = true;
    using type = T;
};

template <typename T, template <typename> typename Predicate>
struct _find_if<false, T, EmptySequence, Predicate>
{
    static constexpr decltype(false) value = false;
};

template <typename T, typename... Ts, template <typename> typename Predicate>
struct _find_if<false, T, TypeSequence<Ts...>, Predicate>
    : _find_if<Predicate<head_t<TypeSequence<Ts...> > >::value, head_t<TypeSequence<Ts...> >,
               tail_t<TypeSequence<Ts...> >, Predicate>
{};
};

template <typename Seq, template <typename> typename Predicate>
using find_if = _find_if<false, void, Seq, Predicate>;

template <typename Seq, template <typename> typename Predicate>
using find_if_t = typename find_if<Seq, Predicate>::type;

namespace {

template <typename Seq, typename Acc, typename From, typename To>
struct _replace;

template <typename... TsSeq, typename... TsAcc, typename From, typename To>
struct _replace<TypeSequence<TsSeq...>, TypeSequence<TsAcc...>, From, To>
    : _replace<tail_t<TypeSequence<TsSeq...> >,
               append_t<TypeSequence<TsAcc...>, head_t<TypeSequence<TsSeq...> > >, From, To>
{};

template <typename... TsSeq, typename... TsAcc, typename From, typename To>
struct _replace<TypeSequence<From, TsSeq...>, TypeSequence<TsAcc...>, From, To>
{
    using type = TypeSequence<TsAcc..., To, TsSeq...>;
};

};

template <typename Seq, typename From, typename To>
using replace = _replace<Seq, EmptySequence, From, To>;

template <typename Seq, typename From, typename To>
using replace_t = typename replace<Seq, From, To>::type;

template <typename Seq, size_t IDX>
struct get;

template <typename... Ts, size_t IDX>
struct get<TypeSequence<Ts...>, IDX> : get<tail_t<TypeSequence<Ts...> >, IDX - 1>
{
    STATIC_ASSERT(IDX < len<TypeSequence<Ts...> >::value);
};

template <typename... Ts>
struct get<TypeSequence<Ts...>, 0>
{
    STATIC_ASSERT(len<TypeSequence<Ts...> >::value != len<EmptySequence>::value);
    using type = head_t<TypeSequence<Ts...> >;
};

template <typename Seq, size_t IDX>
using get_t = typename get<Seq, IDX>::type;

template <typename Seq, template <typename...> typename Accumulator, typename Start>
struct accumulate;

template <template <typename...> typename Accumulator, typename Res>
struct accumulate<EmptySequence, Accumulator, Res> : Res
{};

template <typename... Ts, template <typename...> typename Accumulator, typename Res>
struct accumulate<TypeSequence<Ts...>, Accumulator, Res>
    : accumulate<tail_t<TypeSequence<Ts...> >, Accumulator,
                 Accumulator<head_t<TypeSequence<Ts...> >, Res> >
{};

};

#endif
