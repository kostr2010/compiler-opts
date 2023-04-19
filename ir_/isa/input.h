#ifndef __ISA_INPUT_H_INCLUDED__
#define __ISA_INPUT_H_INCLUDED__

#include "../../utils/macros.h"
#include "../../utils/type_sequence.h"

#include "isa_def.h"

namespace isa {

namespace input {

#define GEN_INPUT_TYPE_ENUM(NAME, ...) NAME,
enum Kind
{
    ISA_INPUT_TYPE_LIST(GEN_INPUT_TYPE_ENUM)
};
#undef GEN_INPUT_TYPE_ENUM

template <Kind T>
struct Input;

#define OPERATOR(ELEM) ELEM,
#define GEN_INPUT_STRUCT_DEF(NAME, DEFAULT_VALUE, ...)                                            \
    template <>                                                                                   \
    struct Input<Kind::NAME>                                                                      \
    {                                                                                             \
        using ValueType = decltype(DEFAULT_VALUE);                                                \
        using DefaultValue = std::integral_constant<ValueType, DEFAULT_VALUE>;                    \
        using Type = std::integral_constant<Kind, Kind::NAME>;                                    \
    };
ISA_INPUT_TYPE_LIST(GEN_INPUT_STRUCT_DEF)
#undef OPERATOR
#undef GEN_INPUT_STRUCT_DEF

template <Kind T, typename Input<T>::ValueType VAL>
struct Value : std::integral_constant<typename Input<T>::ValueType, VAL>
{
    using Type = std::integral_constant<Kind, T>;
};

}; // namespace input

}; // namespace isa

#endif
