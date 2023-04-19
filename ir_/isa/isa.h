#ifndef __ISA_H_INCLUDED__
#define __ISA_H_INCLUDED__

#include "../../utils/macros.h"
#include "../../utils/type_sequence.h"

#include "instruction.h"
#include "isa_def.h"

#include <array>

namespace isa {

#define GEN_INSTRUCTION_CHAIN(OPCODE, ...) (OPCODE)
#define OPERATOR(ELEM) (inst::Inst<inst::Opcode::ELEM>)
using ISA = type_sequence::TypeSequence<CHAIN_COMMA(
    CHAIN_OPERATOR(ISA_INSTRUCTION_LIST(GEN_INSTRUCTION_CHAIN)))>;
#undef OPERATOR
#undef GEN_INSTRUCTION_CHAIN

template <inst::Opcode OPCODE, flag::Kind F>
using HasFlag = type_sequence::Find<flag::Flag<F>, typename inst::Inst<OPCODE>::Flags>;

template <typename g>
struct A;

template <inst::Opcode OPCODE, input::Kind I>
struct InputValue
{
    template <typename Value>
    using ValueHasSameInputKind = std::integral_constant<bool, Value::Type::value == I>;

    using Input = input::Input<I>;
    using Inputs = typename inst::Inst<OPCODE>::Signature::Inputs;
    using Values = typename inst::Inst<OPCODE>::Signature::InputsValues;
    using HasInput = type_sequence::Find<Input, Inputs>;
    using Res =
        std::conditional_t<HasInput::value, type_sequence::GetIf<Values, ValueHasSameInputKind>,
                           typename Input::DefaultValue>;

    static constexpr decltype(Res::value) value = Res::value;
};

template <inst::Opcode OP, flag::Kind F>
struct FlagValue
{
    template <typename V>
    using IsSameFlag = std::integral_constant<bool, V::Type::value == flag::Flag<F>::Type::value>;
    using Res = type_sequence::GetIf<typename inst::Inst<OP>::FlagValues, IsSameFlag>;

    static constexpr decltype(Res::value) value = Res::value;
};

template <template <inst::Opcode> typename Predicate>
auto EvaluatePredicate(const inst::Opcode opcode)
{
    static constexpr std::array TABLE = {
#define GEN_TRUTH_TABLE(OPCODE, ...) Predicate<inst::Opcode::OPCODE>::value,
        ISA_INSTRUCTION_LIST(GEN_TRUTH_TABLE)
#undef GEN_TRUTH_TABLE
    };

    return TABLE[opcode];
}

}; // namespace isa

#endif
