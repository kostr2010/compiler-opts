#ifndef __TYPE_HELPERS_H_INCLUDED__
#define __TYPE_HELPERS_H_INCLUDED__

#include <type_traits>

namespace type_helpers {

template <typename Default, typename VoidT, template <typename...> typename Op, typename... Args>
struct validator
{
    STATIC_ASSERT(std::is_same<void, VoidT>::value);

    using value_t = std::false_type;
    using type = Default;
};

template <typename Default, template <typename...> typename Op, typename... Args>
struct validator<Default, std::void_t<Op<Args...> >, Op, Args...>
{
    using value_t = std::true_type;
    using type = Op<Args...>;
};

// detect whether Op<Args...> is a valid type, use Default if not.
template <typename Default, template <typename...> typename Op, typename... Args>
using valid_or = validator<Default, void, Op, Args...>;

// Op<Args> if that is a valid type, otherwise Default.
template <typename Default, template <typename...> typename Op, typename... Args>
using valid_or_t = typename valid_or<Default, Op, Args...>::type;

}; // namespace type_helpers

#endif
