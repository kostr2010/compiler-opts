#include <algorithm>
#include <iostream>
#include <limits>
#include <type_traits>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

// file for tomfoolery and experiments

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>

struct Base
{
    template <typename P>
    using IsBase = std::is_base_of<Base, P>;

    template <typename T>
    struct BaseTraits
    {
        using is_cfg_sensitive = std::integral_constant<bool, false>;
        using num_marks = std::integral_constant<size_t, 0>;
    };

    template <typename P>
    using Markers = marking::Marker[P::BaseTraits::num_marks::value];
};

// struct D : public Base
// {
//     template <>
//     struct BaseTraits<D>
//     {
//         using is_cfg_sensitive = std::integral_constant<bool, true>;
//         using num_marks = std::integral_constant<size_t, 1>;
//     };
// };

// struct D2 : public Base
// {};

int main()
{
    // std::cout << D2::BaseTra
    return 0;
}

#pragma GCC diagnostic pop
