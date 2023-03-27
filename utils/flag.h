#ifndef __UTILS_FIELD_GEN_INCLUDED__
#define __UTILS_FIELD_GEN_INCLUDED__

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "macros.h"

template <typename Holder, size_t BIT_NUM = 0>
class Flag
{
    static_assert(std::is_same<uint8_t, Holder>::value || std::is_same<uint16_t, Holder>::value ||
                  std::is_same<uint32_t, Holder>::value || std::is_same<uint64_t, Holder>::value);
    static_assert(BIT_NUM < sizeof(Holder) * 8);

  public:
    using Type = Holder;
    using Next = Flag<Holder, BIT_NUM + 1>;
    using Value = std::integral_constant<Holder, 1 << BIT_NUM>;

  private:
    NO_DEFAULT_CTOR(Flag);
    NO_DEFAULT_DTOR(Flag);
};

#endif
