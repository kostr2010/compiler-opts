#ifndef __ISA_INSTRCTION_H_INCLUDED__
#define __ISA_INSTRCTION_H_INCLUDED__

#include "../../utils/macros.h"
#include "../../utils/type_sequence.h"

#include "flag.h"
#include "isa_def.h"
#include "signature.h"

namespace isa {

namespace inst {

#define GEN_OPCODE_ENUM(OPCODE, ...) OPCODE,
enum Opcode
{
    ISA_INSTRUCTION_LIST(GEN_OPCODE_ENUM) NUM_OPCODES
};
#undef GEN_OPCODE_ENUM

template <Opcode OPCODE>
struct Inst;

#define PREFIX(ELEM, ...) (flag::Flag<flag::Kind::ELEM
#define SUFFIX(ELEM, ...) ELEM>)
// the following magic dispathes one of two macro handlers for flag: with or without value
// if flag has no value, we do not generate specialization
// if flag has value, we generate specialization and check bounds for the given value
#define OPERATOR_NO_VALUE(ITEM) EMPTY()
#define OPERATOR_VALUE(ITEM, VALUE)                                                               \
    flag::Value<flag::Kind::ITEM, flag::Flag<flag::Kind::ITEM>::Values::VALUE>
#define SELECT_OPERATOR(_1, _2, NAME, ...) NAME
#define OPERATOR(...) SELECT_OPERATOR(__VA_ARGS__, OPERATOR_VALUE, OPERATOR_NO_VALUE)(__VA_ARGS__)
#define GEN_INSTRUCTION_DEF(OPCODE, SIGNATURE, FLAGS, ...)                                        \
    template <>                                                                                   \
    class Inst<OPCODE>                                                                            \
    {                                                                                             \
      private:                                                                                    \
        template <typename T, typename ACC>                                                       \
        using BitMaskAccumulator =                                                                \
            std::integral_constant<size_t, ACC::value | T::BitMask::value>;                       \
                                                                                                  \
      public:                                                                                     \
        /* produces code like                                                                     \
         * type_sequence::TypeSequence<Flag<Kind::FLAG1>, Flag<Kind::FLAG2>, ...> */              \
        using Flags = type_sequence::TypeSequence<CHAIN_COMMA(CHAIN_PREFIX_SUFFIX(FLAGS))>;       \
        using BitMask = type_sequence::Accumulate<Flags, BitMaskAccumulator, flag::EmptyBitMask>; \
        using FlagValues = type_sequence::TypeSequence<CHAIN_OPERATOR(FLAGS)>;                    \
        using Signature = signature::InstructionTypeInfo<signature::InstructionType::SIGNATURE>;  \
        using Type = std::integral_constant<Opcode, OPCODE>;                                      \
    };
ISA_INSTRUCTION_LIST(GEN_INSTRUCTION_DEF)
#undef OPERATOR
#undef PREFIX
#undef SUFFIX
#undef GEN_INSTRUCTION_DEF

}; // namespace inst

}; // namespace isa

#endif
