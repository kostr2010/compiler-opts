#include "utils/macros.h"
#include "utils/type_helpers.h"

#include "gtest/gtest.h"

template <typename T>
using TestType = typename T::value_type;

TEST(TestTypeHelpers, TestValidOr)
{
    ASSERT_TRUE((std::is_same_v<type_helpers::valid_or_t<void, TestType, int>, void>));
    ASSERT_TRUE((std::is_same_v<type_helpers::valid_or_t<void, TestType, std::true_type>, bool>));
}
