#include <algorithm>
#include <iostream>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

// file for tomfoolery and experiments

template <size_t L>
struct Mark
{
    static constexpr size_t LEN = L;
};

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

    template <size_t COUNT, typename... Pack>
    struct count_mark_bits;

    template <size_t COUNT, typename Type, typename... Pack>
    struct count_mark_bits<COUNT, Type, Pack...>
        : count_mark_bits<COUNT + get_mark_length<Type>(), Pack...>
    {
    };

    template <size_t COUNT>
    struct count_mark_bits<COUNT> : std::integral_constant<size_t, COUNT>
    {
    };

  public:
    template <typename Type>
    using HasType = is_one_of<Type, Types...>;

    template <typename Type>
    using HasMarks = has_marks_field<Type>;

    template <typename Type>
    using GetPassIdx = get_pass_idx<Type, 0, Types...>;

    template <typename Type>
    using GetMarkOfft = get_mark_offt<Type, 0, Types...>;

    template <typename Type>
    using GetMarkOfftStep = get_mark_length<Type>;

    static constexpr size_t BITS_COUNT = count_mark_bits<0, Types...>();
};

struct test2
{
    using Marks = Mark<2>;
};

struct test4
{
    using Marks = Mark<3>;
};

int main()
{
    // std::vector<int> a = { 1, 4, 7, 6, 5, 4, 3, 7, 8, 4, 7, 0 };

    // for (auto elem = a.rbegin() + a.size() - 5; elem != a.rend(); ++elem) {
    //     std::cout << *elem << "\n";
    // }

    using Tps = Passes<int, double, float, test2, test4, void>;
    std::cout << Tps::HasType<float>() << "\n";
    std::cout << Tps::HasType<void>() << "\n";
    std::cout << Tps::HasType<size_t>() << "\n";
    std::cout << Tps::HasType<test2>() << "\n";
    std::cout << Tps::HasType<test4>() << "\n";
    std::cout << Tps::HasType<int>() << "\n";

    std::cout << "=======================\n";

    std::cout << Tps::HasMarks<int>() << "\n";
    std::cout << Tps::HasMarks<double>() << "\n";
    std::cout << Tps::HasMarks<float>() << "\n";
    std::cout << Tps::HasMarks<test2>() << "\n";
    std::cout << Tps::HasMarks<test4>() << "\n";
    std::cout << Tps::HasMarks<void>() << "\n";
    std::cout << Tps::HasMarks<size_t>() << "\n";
    std::cout << Tps::HasMarks<int[]>() << "\n";
    std::cout << "=======================\n";

    std::cout << Tps::GetPassIdx<int>() << "\n";
    std::cout << Tps::GetPassIdx<double>() << "\n";
    std::cout << Tps::GetPassIdx<float>() << "\n";
    std::cout << Tps::GetPassIdx<test2>() << "\n";
    std::cout << Tps::GetPassIdx<test4>() << "\n";
    std::cout << Tps::GetPassIdx<void>() << "\n";
    // std::cout << Tps::GetPassIdx<size_t>() << "\n";
    // std::cout << Tps::GetPassIdx<int[]>() << "\n";

    std::cout << "=======================\n";

    std::cout << Tps::GetMarkOfftStep<int>() << "\n";
    std::cout << Tps::GetMarkOfftStep<double>() << "\n";
    std::cout << Tps::GetMarkOfftStep<float>() << "\n";
    std::cout << Tps::GetMarkOfftStep<test2>() << "\n";
    std::cout << Tps::GetMarkOfftStep<test4>() << "\n";
    std::cout << Tps::GetMarkOfftStep<void>() << "\n";
    std::cout << "=======================\n";

    std::cout << Tps::GetMarkOfft<int>() << "\n";
    std::cout << Tps::GetMarkOfft<double>() << "\n";
    std::cout << Tps::GetMarkOfft<float>() << "\n";
    std::cout << Tps::GetMarkOfft<test2>() << "\n";
    std::cout << Tps::GetMarkOfft<test4>() << "\n";
    std::cout << Tps::GetMarkOfft<void>() << "\n";

    std::cout << "cnt: " << Tps::BITS_COUNT << "\n";
}

#pragma GCC diagnostic pop
