#ifndef __UTILS_TYPE_SEQUENCE_INCLUDED__
#define __UTILS_TYPE_SEQUENCE_INCLUDED__

#include <cstddef>
#include <cstdint>
#include <type_traits>

// type wrapper for type pack, which is not type in c++
namespace type_sequence {
template <typename... Ts>
struct TypeSequence
{
};

using EmptySequence = TypeSequence<>;

// Trio of functions that allow to create aggregate types
// Borrowed from LISP
template <typename Seq>
struct Head;

template <typename T, typename... Ts>
struct Head<TypeSequence<T, Ts...> >
{
    using type = T;
};

template <typename Seq>
using Head_t = typename Head<Seq>::type;

template <typename Seq>
struct Tail;

template <typename T, typename... Ts>
struct Tail<TypeSequence<T, Ts...> >
{
    using type = TypeSequence<Ts...>;
};

template <typename Seq>
using Tail_t = typename Tail<Seq>::type;

template <typename T, typename Seq>
struct Prepend;

template <typename T, typename... Ts>
struct Prepend<T, TypeSequence<Ts...> >
{
    using type = TypeSequence<T, Ts...>;
};

template <typename T, typename Seq>
using Prepend_t = typename Prepend<T, Seq>::type;

template <typename T, typename Seq>
struct Append;

template <typename T, typename... Ts>
struct Append<T, TypeSequence<Ts...> >
{
    using type = TypeSequence<Ts..., T>;
};

template <typename T, typename Seq>
using Append_t = typename Append<T, Seq>::type;

template <typename Seq1, typename Seq2>
struct Concat;

template <typename... Ts1, typename... Ts2>
struct Concat<TypeSequence<Ts1...>, TypeSequence<Ts2...> >
{
    using type = TypeSequence<Ts1..., Ts2...>;
};

template <typename Seq1, typename Seq2>
using Concat_t = typename Concat<Seq1, Seq2>::type;

template <typename Seq>
struct Length;

template <>
struct Length<EmptySequence> : std::integral_constant<size_t, 0>
{
};

template <typename... Ts>
struct Length<TypeSequence<Ts...> >
    : std::integral_constant<size_t, Length<Tail_t<TypeSequence<Ts...> > >::value + 1>
{
};

template <typename T, typename Seq>
struct Find;

template <typename T, typename... Ts>
struct Find<T, TypeSequence<Ts...> > : Find<T, Tail_t<TypeSequence<Ts...> > >
{
};

template <typename T, typename... Ts>
struct Find<T, TypeSequence<T, Ts...> > : std::true_type
{
};

template <typename T>
struct Find<T, EmptySequence> : std::false_type
{
};

template <typename From, typename To, typename Seq>
struct Replace;

template <typename From, typename To, typename... Ts>
struct Replace<From, To, TypeSequence<Ts...> >
    : Replace<TypeSequence<From>, To, TypeSequence<Ts...> >
{
    static_assert(Find<From, TypeSequence<Ts...> >::value);
};

template <typename To, typename... Ts1, typename... Ts2>
struct Replace<TypeSequence<Ts1...>, To, TypeSequence<Ts2...> >
    : Replace<Append_t<Head_t<TypeSequence<Ts2...> >, TypeSequence<Ts1...> >, To,
              Tail_t<TypeSequence<Ts2...> > >
{
};

template <typename T, typename To, typename... Ts1, typename... Ts2>
struct Replace<TypeSequence<T, Ts1...>, To, TypeSequence<T, Ts2...> >
{
    using type = TypeSequence<Ts1..., To, Ts2...>;
};

template <typename From, typename To, typename Seq>
using Replace_t = typename Replace<From, To, Seq>::type;

template <size_t IDX, typename Seq>
struct Get;

template <size_t IDX, typename... Ts>
struct Get<IDX, TypeSequence<Ts...> > : Get<IDX - 1, Tail_t<TypeSequence<Ts...> > >
{
    static_assert(IDX < Length<TypeSequence<Ts...> >::value);
};

template <typename... Ts>
struct Get<0, TypeSequence<Ts...> >
{
    using Seq = TypeSequence<Ts...>;
    static_assert(Length<Seq>::value != Length<EmptySequence>::value);
    using type = Head_t<Seq>;
};

template <size_t IDX, typename Seq>
using Get_t = typename Get<IDX, Seq>::type;

template <typename Seq, template <typename> typename Predicate>
struct GetIf;

template <typename... Ts, template <typename> typename Predicate>
struct GetIf<TypeSequence<Ts...>, Predicate>
    : std::conditional_t<Predicate<Head_t<TypeSequence<Ts...> > >::value,
                         Head_t<TypeSequence<Ts...> >,
                         GetIf<Tail_t<TypeSequence<Ts...> >, Predicate> >
{
};

template <typename Seq, template <typename...> typename Accumulator, typename Start>
struct Accumulate;

template <template <typename...> typename Accumulator, typename Res>
struct Accumulate<EmptySequence, Accumulator, Res> : Res
{
};

template <typename T, template <typename...> typename Accumulator, typename Res>
struct Accumulate<TypeSequence<T>, Accumulator, Res> : Accumulator<T, Res>
{
};

template <typename... Ts, template <typename...> typename Accumulator, typename Res>
struct Accumulate<TypeSequence<Ts...>, Accumulator, Res>
    : Accumulate<Tail_t<TypeSequence<Ts...> >, Accumulator,
                 Accumulator<Head_t<TypeSequence<Ts...> >, Res> >
{
};

};

#endif
