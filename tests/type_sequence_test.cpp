#include "utils/macros.h"
#include "utils/type_sequence.h"

#include "gtest/gtest.h"

TEST(TestTypeSequence, TestHeadTail)
{
    using seq = type_sequence::TypeSequence<int, void, char>;

    ASSERT_TRUE((std::is_same_v<type_sequence::head_t<seq>, int>));
    ASSERT_TRUE(
        (std::is_same_v<type_sequence::tail_t<seq>, type_sequence::TypeSequence<void, char> >));
}

TEST(TestTypeSequence, TestAppend)
{
    using seq = type_sequence::EmptySequence;
    ASSERT_TRUE((std::is_same_v<seq, type_sequence::EmptySequence>));

    using seq1 = type_sequence::append_t<seq, int>;
    ASSERT_TRUE((std::is_same_v<seq1, type_sequence::TypeSequence<int> >));

    using seq2 = type_sequence::append_t<seq1, int>;
    ASSERT_TRUE((std::is_same_v<seq2, type_sequence::TypeSequence<int, int> >));

    using seq3 = type_sequence::append_t<seq2, void>;
    ASSERT_TRUE((std::is_same_v<seq3, type_sequence::TypeSequence<int, int, void> >));
}

TEST(TestTypeSequence, TestPrepend)
{
    using seq = type_sequence::EmptySequence;
    ASSERT_TRUE((std::is_same_v<seq, type_sequence::EmptySequence>));

    using seq1 = type_sequence::prepend_t<seq, int>;
    ASSERT_TRUE((std::is_same_v<seq1, type_sequence::TypeSequence<int> >));

    using seq2 = type_sequence::prepend_t<seq1, int>;
    ASSERT_TRUE((std::is_same_v<seq2, type_sequence::TypeSequence<int, int> >));

    using seq3 = type_sequence::prepend_t<seq2, void>;
    ASSERT_TRUE((std::is_same_v<seq3, type_sequence::TypeSequence<void, int, int> >));
}

TEST(TestTypeSequence, TestCat)
{
    using seq = type_sequence::EmptySequence;
    ASSERT_TRUE((std::is_same_v<seq, type_sequence::EmptySequence>));

    using seq1 = type_sequence::cat_t<seq, seq>;
    ASSERT_TRUE((std::is_same_v<seq1, type_sequence::EmptySequence>));

    using seq2 =
        type_sequence::cat_t<type_sequence::TypeSequence<int>, type_sequence::TypeSequence<char> >;
    ASSERT_TRUE((std::is_same_v<seq2, type_sequence::TypeSequence<int, char> >));

    using seq3 =
        type_sequence::cat_t<type_sequence::TypeSequence<char>, type_sequence::TypeSequence<int> >;
    ASSERT_TRUE((std::is_same_v<seq3, type_sequence::TypeSequence<char, int> >));
}

TEST(TestTypeSequence, TestLen)
{
    ASSERT_EQ((type_sequence::len<type_sequence::EmptySequence>::value), 0);

    using seq1 = type_sequence::TypeSequence<int>;
    ASSERT_EQ((type_sequence::len<seq1>::value), 1);

    using seq2 = type_sequence::TypeSequence<int, char>;
    ASSERT_EQ((type_sequence::len<seq2>::value), 2);

    using seq3 = type_sequence::TypeSequence<int, char, void>;
    ASSERT_EQ((type_sequence::len<seq3>::value), 3);
}

TEST(TestTypeSequence, TestFind)
{
    using seq = type_sequence::TypeSequence<int, float, void>;
    ASSERT_EQ((type_sequence::find<seq, int>::value), true);
    ASSERT_EQ((type_sequence::find<seq, float>::value), true);
    ASSERT_EQ((type_sequence::find<seq, void>::value), true);
    ASSERT_EQ((type_sequence::find<seq, double>::value), false);
    ASSERT_EQ((type_sequence::find<seq, char>::value), false);
}

template <typename T>
using is_int = std::is_same<T, int>;
template <typename T>
using is_void = std::is_same<T, void>;
template <typename T>
using is_char = std::is_same<T, char>;
template <typename T>
using is_double = std::is_same<T, double>;

TEST(TestTypeSequence, TestFindIf)
{
    using seq = type_sequence::TypeSequence<int, void, char>;

    ASSERT_TRUE(is_int<int>::value);
    ASSERT_FALSE(is_int<void>::value);
    ASSERT_FALSE(is_int<char>::value);

    ASSERT_FALSE(is_void<int>::value);
    ASSERT_TRUE(is_void<void>::value);
    ASSERT_FALSE(is_void<char>::value);

    ASSERT_FALSE(is_char<int>::value);
    ASSERT_FALSE(is_char<void>::value);
    ASSERT_TRUE(is_char<char>::value);

    ASSERT_FALSE(is_double<int>::value);
    ASSERT_FALSE(is_double<void>::value);
    ASSERT_FALSE(is_double<char>::value);

    ASSERT_TRUE((type_sequence::find_if<seq, is_int>::value));
    ASSERT_TRUE((std::is_same_v<type_sequence::find_if_t<seq, is_int>, int>));

    ASSERT_TRUE((type_sequence::find_if<seq, is_void>::value));
    ASSERT_TRUE((std::is_same_v<type_sequence::find_if_t<seq, is_void>, void>));

    ASSERT_TRUE((type_sequence::find_if<seq, is_char>::value));
    ASSERT_TRUE((std::is_same_v<type_sequence::find_if_t<seq, is_char>, char>));

    ASSERT_FALSE((type_sequence::find_if<seq, is_double>::value));
}

TEST(TestTypeSequence, TestReplace)
{
    using seq = type_sequence::TypeSequence<int, void, char, float>;

    ASSERT_TRUE((std::is_same_v<type_sequence::replace_t<seq, int, double>,
                                type_sequence::TypeSequence<double, void, char, float> >));
    ASSERT_TRUE((std::is_same_v<type_sequence::replace_t<seq, void, double>,
                                type_sequence::TypeSequence<int, double, char, float> >));
    ASSERT_TRUE((std::is_same_v<type_sequence::replace_t<seq, char, double>,
                                type_sequence::TypeSequence<int, void, double, float> >));
    ASSERT_TRUE((std::is_same_v<type_sequence::replace_t<seq, float, double>,
                                type_sequence::TypeSequence<int, void, char, double> >));
}

TEST(TestTypeSequence, TestGet)
{
    using seq = type_sequence::TypeSequence<int, void, char>;

    ASSERT_TRUE((std::is_same_v<type_sequence::get_t<seq, 0>, int>));
    ASSERT_TRUE((std::is_same_v<type_sequence::get_t<seq, 2>, char>));
    ASSERT_TRUE((std::is_same_v<type_sequence::get_t<seq, 1>, void>));
}

template <typename, typename COUNT>
using Count = std::integral_constant<typename COUNT::value_type, COUNT::value + 1>;

template <typename T, typename COUNT>
using CountV = std::integral_constant<typename COUNT::value_type,
                                      COUNT::value + 1 * std::is_same_v<void, T> >;

TEST(TestTypeSequence, TestAccumulate)
{
    using seq = type_sequence::TypeSequence<int, void, char, void>;

    ASSERT_EQ((type_sequence::accumulate<seq, Count, std::integral_constant<int, 0> >::value), 4);
    ASSERT_EQ((type_sequence::accumulate<seq, CountV, std::integral_constant<int, 0> >::value), 2);
}
