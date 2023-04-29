#ifndef __ISA_FLAG_H_INCLUDED__
#define __ISA_FLAG_H_INCLUDED__

#include "utils/bit_flag.h"
#include "utils/macros.h"
#include "utils/type_sequence.h"

#include "isa_def.h"

namespace isa {

namespace flag {

using Holder = uint8_t;
using ValueT = uint8_t;
using EmptyBitMask = std::integral_constant<Holder, 0>;

#define GEN_FLAG_ENUM(NAME, ...) NAME,
enum Type : Holder
{
    ISA_FLAG_LIST(GEN_FLAG_ENUM)
};
#undef GEN_FLAG_ENUM

template <Type FLAG>
struct Flag;

#define OPERATOR(ELEM) ELEM,
#define GEN_FLAG_STRUCT_DEF(NAME, VALUES, ...)                                                    \
    template <>                                                                                   \
    struct Flag<Type::NAME>                                                                       \
    {                                                                                             \
        enum Value : ValueT                                                                       \
        {                                                                                         \
            CHAIN_OPERATOR(VALUES) NUM_VALUES                                                     \
        };                                                                                        \
        using HasValue = std::integral_constant<bool, Value::NUM_VALUES != 0>;                    \
        using BitMask = BitFlag<Holder, Type::NAME>;                                              \
        static constexpr Type type = Type::NAME;                                                  \
    };
ISA_FLAG_LIST(GEN_FLAG_STRUCT_DEF)
#undef OPERATOR
#undef GEN_FLAG_STRUCT_DEF

template <Type FLAG, typename Flag<FLAG>::Value VALUE>
struct Value : std::integral_constant<ValueT, VALUE>
{
    static_assert(Flag<FLAG>::HasValue::value);
    static_assert(VALUE < Flag<FLAG>::Value::NUM_VALUES);
    static constexpr Type type = FLAG;
};

}; // namespace flag
}; // namespace isa

#endif
