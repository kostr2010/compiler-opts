#include <algorithm>
#include <iostream>
#include <type_traits>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

// file for tomfoolery and experiments

template <typename... Types>
struct PassList
{
    template <typename Type, typename... Pack>
    struct is_one_of;

    template <typename Type>
    struct is_one_of<Type> : std::false_type
    {
    };

    using ActivePasses = std::tuple<Types...>;
};

struct Pass1
{
    static constexpr size_t Val()
    {
        return 1;
    }
};

struct Pass2
{
    static constexpr size_t Val()
    {
        return 2;
    }
};

struct Pass3
{
    static constexpr size_t Val()
    {
        return 3;
    }
};

using pss = PassList<Pass2, Pass1, Pass3>;

template <typename Target, size_t Acc, typename Tail>
struct get_mark_offset;

template <typename Target, size_t Acc, typename Head, class... Tail>
struct get_mark_offset<Target, Acc, std::tuple<Head, Tail...> >
    : get_mark_offset<Target, Acc + Head::Val(), std::tuple<Tail...> >
{
};

template <typename Target, size_t Acc, typename... Tail>
struct get_mark_offset<Target, Acc, std::tuple<Target, Tail...> >
    : std::integral_constant<size_t, Acc>
{
};

template <typename Type>
using GetMarkOffset = get_mark_offset<Type, 0, pss::ActivePasses>;

int main()
{
    std::cout << GetMarkOffset<Pass1>() << "\n";
}

#pragma GCC diagnostic pop
