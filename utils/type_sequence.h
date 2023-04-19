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
// Boorawed from LISP
template <typename... Ts>
struct Head;

template <typename T, typename... Ts>
struct Head<TypeSequence<T, Ts...> >
{
    using type = T;
};

template <typename Seq>
using Head_t = typename Head<Seq>::type;

template <typename... Ts>
struct Tail;

template <typename T, typename... Ts>
struct Tail<TypeSequence<T, Ts...> >
{
    using type = TypeSequence<Ts...>;
};

template <typename Seq>
using Tail_t = typename Tail<Seq>::type;

template <typename... Ts>
struct Prepend;

template <typename T, typename... Ts>
struct Prepend<T, TypeSequence<Ts...> >
{
    using type = TypeSequence<T, Ts...>;
};

template <typename T, typename Seq>
using Prepend_t = typename Prepend<T, Seq>::type;

template <typename... Ts>
struct Append;

template <typename T, typename... Ts>
struct Append<T, TypeSequence<Ts...> >
{
    using type = TypeSequence<Ts..., T>;
};

template <typename T, typename Seq>
using Append_t = typename Append<T, Seq>::type;

template <typename Seq>
struct Length;

template <>
struct Length<EmptySequence> : std::integral_constant<size_t, 0>
{};

template <typename... Ts>
struct Length<TypeSequence<Ts...> >
    : std::integral_constant<size_t, Length<Tail_t<TypeSequence<Ts...> > >() + 1>
{};

template <typename T, typename Seq>
struct Find;

template <typename T, typename... Ts>
struct Find<T, TypeSequence<T, Ts...> > : std::true_type
{};

template <typename T, typename... Ts>
struct Find<T, TypeSequence<Ts...> > : Find<T, Tail_t<TypeSequence<Ts...> > >
{};

template <typename T>
struct Find<T, EmptySequence> : std::false_type
{};

template <typename Seq, template <typename...> typename Predicate, typename CompareTo>
struct GetIf;

template <typename T, typename... Ts, template <typename...> typename Predicate,
          typename CompareTo>
struct GetIf<TypeSequence<T, Ts...>, Predicate, CompareTo>
    : std::conditional_t<Predicate<T, CompareTo>::value, T,
                         GetIf<TypeSequence<Ts...>, Predicate, CompareTo> >
{};

template <typename Seq, template <typename...> typename Accumulator, typename Start>
struct Accumulate;

template <template <typename...> typename Accumulator, typename Res>
struct Accumulate<EmptySequence, Accumulator, Res> : Res
{};

template <typename T, template <typename...> typename Accumulator, typename Res>
struct Accumulate<TypeSequence<T>, Accumulator, Res> : Accumulator<T, Res>
{};

template <typename... Ts, template <typename...> typename Accumulator, typename Res>
struct Accumulate<TypeSequence<Ts...>, Accumulator, Res>
    : Accumulate<Tail_t<TypeSequence<Ts...> >, Accumulator,
                 Accumulator<Head_t<TypeSequence<Ts...> >, Res> >
{};

};

#endif
