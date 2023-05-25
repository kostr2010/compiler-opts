#ifndef __UTILS_FIELD_GEN_INCLUDED__
#define __UTILS_FIELD_GEN_INCLUDED__

#include <climits>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "utils/macros.h"

template <typename Holder, unsigned BIT_NUM = 0>
struct BitFlag : std::integral_constant<Holder, 1 << BIT_NUM>
{
    STATIC_ASSERT(std::is_same<uint8_t, Holder>::value || std::is_same<uint16_t, Holder>::value ||
                  std::is_same<uint32_t, Holder>::value || std::is_same<uint64_t, Holder>::value);
    STATIC_ASSERT(BIT_NUM < sizeof(Holder) * CHAR_BIT);

    using Next = BitFlag<Holder, BIT_NUM + 1>;
};

#endif
