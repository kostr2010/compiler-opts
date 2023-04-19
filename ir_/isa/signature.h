#ifndef __ISA_SIGNATURE_H_INCLDED__
#define __ISA_SIGNATURE_H_INCLDED__

#include "../../utils/macros.h"
#include "../../utils/type_sequence.h"

#include "input.h"
#include "isa_def.h"

namespace isa {

namespace signature {

#define GEN_INTRUCTION_TYPE_ENUM(NAME, ...) NAME,
enum InstructionType
{
    ISA_INSTRUCTION_TYPE_LIST(GEN_INTRUCTION_TYPE_ENUM)
};
#undef GEN_INTRUCTION_TYPE_ENUM

template <InstructionType T>
struct InstructionTypeInfo;

#define PREFIX(ELEM, ...) (input::Input<input::Kind::ELEM
#define SUFFIX(ELEM, ...) ELEM>)
#define OPERATOR(L, R) (input::Value<input::Kind::L, R>)
#define GEN_SPECIALIZED_INTRUCTION_TYPE_INFO_DEF(NAME, INPUTS, ...)                               \
    template <>                                                                                   \
    struct InstructionTypeInfo<InstructionType::NAME>                                             \
    {                                                                                             \
        using Type = std::integral_constant<InstructionType, InstructionType::NAME>;              \
        using Inputs = type_sequence::TypeSequence<CHAIN_COMMA(CHAIN_PREFIX_SUFFIX(INPUTS))>;     \
        using InputsValues = type_sequence::TypeSequence<CHAIN_COMMA(CHAIN_OPERATOR(INPUTS))>;    \
    };
ISA_INSTRUCTION_TYPE_LIST(GEN_SPECIALIZED_INTRUCTION_TYPE_INFO_DEF)
#undef PREFIX
#undef SUFFIX
#undef OPERATOR
#undef GEN_SPECIALIZED_INTRUCTION_TYPE_INFO_DEF

}; // namespace signature

}; // namespace isa

#endif
