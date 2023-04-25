#ifndef __ISA_SIGNATURE_H_INCLDED__
#define __ISA_SIGNATURE_H_INCLDED__

#include "macros.h"
#include "type_sequence.h"

#include "input.h"
#include "isa_def.h"

namespace isa {

namespace inst_type {

// ISA declares instruction types that must be implemented
#define DECLARE_INSTRUCTION_TYPES(INST_TYPE, ...) class INST_TYPE;
ISA_INSTRUCTION_TYPE_LIST(DECLARE_INSTRUCTION_TYPES)
#undef DECLARE_INSTRUCTION_TYPES

template <typename InstType>
struct InstTypeInfo;

#define PREFIX(ELEM, ...) (input::Input<input::Type::ELEM
#define SUFFIX(ELEM, ...) ELEM>)
#define OPERATOR(L, R) (input::Value<input::Type::L, R>)
#define GEN_SPECIALIZED_INTRUCTION_TYPE_INFO_DEF(NAME, INPUTS, ...)                               \
    template <>                                                                                   \
    struct InstTypeInfo<NAME>                                                                     \
    {                                                                                             \
        using Inputs = type_sequence::TypeSequence<CHAIN_COMMA(CHAIN_PREFIX_SUFFIX(INPUTS))>;     \
        using InputsValues = type_sequence::TypeSequence<CHAIN_COMMA(CHAIN_OPERATOR(INPUTS))>;    \
    };
ISA_INSTRUCTION_TYPE_LIST(GEN_SPECIALIZED_INTRUCTION_TYPE_INFO_DEF)
#undef PREFIX
#undef SUFFIX
#undef OPERATOR
#undef GEN_SPECIALIZED_INTRUCTION_TYPE_INFO_DEF

}; // namespace inst_type

}; // namespace isa

#endif
