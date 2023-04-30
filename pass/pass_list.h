#ifndef __PASS_LIST_H_INCLUDED__
#define __PASS_LIST_H_INCLUDED__

#include "pass.h"

#include <cstddef>
#include <tuple>

template <typename... Types>
class PassList
{
    static_assert((Pass::IsPass<Types>() && ...));

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

  public:
    using Passes = std::tuple<Types...>;
    using NumPasses = std::tuple_size<Passes>;

    template <typename Type>
    using HasPass = is_one_of<Type, Types...>;

    template <typename Type>
    using GetPassIdx = get_pass_idx<Type, 0, Types...>;

    template <size_t IDX>
    using GetPass = std::tuple_element<IDX, Passes>;
};

#endif
