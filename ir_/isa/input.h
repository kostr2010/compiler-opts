#ifndef __ISA_INPUT_H_INCLUDED__
#define __ISA_INPUT_H_INCLUDED__

#include "../../utils/macros.h"
#include "../../utils/type_sequence.h"

#include "isa_def.h"

namespace isa {

namespace input {

#define GEN_INPUT_TYPE_ENUM(NAME, ...) NAME,
enum InputType
{
    ISA_INPUT_TYPE_LIST(GEN_INPUT_TYPE_ENUM)
};
#undef GEN_INPUT_TYPE_ENUM

template <InputType T>
struct Input;

#define OPERATOR(ELEM) ELEM,
#define GEN_INPUT_STRUCT_DEF(NAME, DEFAULT_VALUE, ...)                                            \
    template <>                                                                                   \
    struct Input<InputType::NAME>                                                                 \
    {                                                                                             \
        using ValueType = decltype(DEFAULT_VALUE);                                                \
        using DefaultValue = std::integral_constant<ValueType, DEFAULT_VALUE>;                    \
    };
ISA_INPUT_TYPE_LIST(GEN_INPUT_STRUCT_DEF)
#undef OPERATOR
#undef GEN_INPUT_STRUCT_DEF

template <InputType T, typename Input<T>::ValueType VAL>
struct Value : std::integral_constant<typename Input<T>::ValueType, VAL>
{
    static constexpr InputType type = T;
};

}; // namespace input

}; // namespace isa

#endif
