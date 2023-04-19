#ifndef __ISA_FLAG_H_INCLUDED__
#define __ISA_FLAG_H_INCLUDED__

#include "../../utils/bit_flag.h"
#include "../../utils/macros.h"
#include "../../utils/type_sequence.h"

#include "isa_def.h"

namespace isa {

namespace flag {

using Holder = uint8_t;
using ValueT = uint8_t;
using EmptyBitMask = std::integral_constant<Holder, 0>;

#define GEN_FLAG_ENUM(NAME, ...) NAME,
enum Kind : Holder
{
    ISA_FLAG_LIST(GEN_FLAG_ENUM)
};
#undef GEN_FLAG_ENUM

template <Kind FLAG>
struct Flag;

#define OPERATOR(ELEM) ELEM,
#define GEN_FLAG_STRUCT_DEF(NAME, VALUES, ...)                                                    \
    template <>                                                                                   \
    struct Flag<Kind::NAME>                                                                       \
    {                                                                                             \
        enum Values : ValueT                                                                      \
        {                                                                                         \
            CHAIN_OPERATOR(VALUES) NUM_VALUES                                                     \
        };                                                                                        \
        using HasValue = std::integral_constant<bool, Values::NUM_VALUES != 0>;                   \
        using BitMask = BitFlag<Holder, Kind::NAME>;                                              \
        using Type = std::integral_constant<Kind, Kind::NAME>;                                    \
    };
ISA_FLAG_LIST(GEN_FLAG_STRUCT_DEF)
#undef OPERATOR
#undef GEN_FLAG_STRUCT_DEF

template <Kind FLAG, ValueT VALUE>
struct Value : std::integral_constant<ValueT, VALUE>
{
    static_assert(Flag<FLAG>::HasValue::value);
    static_assert(VALUE < Flag<FLAG>::Values::NUM_VALUES);

    using Type = std::integral_constant<Kind, FLAG>;
};

}; // namespace flag

}; // namespace isa

#endif
