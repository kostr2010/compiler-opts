#ifndef __ISA_H_INCLUDED__
#define __ISA_H_INCLUDED__

#include "macros.h"
#include "type_sequence.h"

#include "instruction.h"
#include "isa_def.h"

#include <array>

namespace isa {

// ISA structure that can be utilized by the end user to extract info using type_sequence API
#define GEN_INSTRUCTION_CHAIN(OPCODE, ...) (OPCODE)
#define OPERATOR(ELEM) (inst::Inst<inst::Opcode::ELEM>)
using ISA = type_sequence::TypeSequence<CHAIN_COMMA(
    CHAIN_OPERATOR(ISA_INSTRUCTION_LIST(GEN_INSTRUCTION_CHAIN)))>;
#undef OPERATOR
#undef GEN_INSTRUCTION_CHAIN

template <typename InstType, input::Type I>
struct InputValue
{
    template <typename Value>
    using ValueHasSameInputKind = std::integral_constant<bool, Value::type == I>;
    using InstTypeInfo = inst_type::InstTypeInfo<InstType>;
    using Inputs = typename InstTypeInfo::Inputs;
    using Values = typename InstTypeInfo::InputsValues;
    using Input = input::Input<I>;
    using HasInput = type_sequence::Find<Input, Inputs>;
    using Res =
        std::conditional_t<HasInput::value, type_sequence::GetIf<Values, ValueHasSameInputKind>,
                           typename Input::DefaultValue>;

    static constexpr decltype(Res::value) value = Res::value;
};

template <inst::Opcode OPCODE, flag::Type F>
using HasFlag = type_sequence::Find<flag::Flag<F>, typename inst::Inst<OPCODE>::Flags>;

template <inst::Opcode OP, flag::Type F>
struct FlagValue
{
    template <typename V>
    using IsSameFlag = std::integral_constant<bool, V::type == F>;
    using Res = type_sequence::GetIf<typename inst::Inst<OP>::FlagValues, IsSameFlag>;

    static constexpr flag::ValueT value = Res::value;
};

template <inst::Opcode OP, flag::Type F, flag::ValueT VALUE>
struct FlagValueOr
{
    using HasFlag = isa::HasFlag<OP, F>;
    using FlagValue = isa::FlagValue<OP, F>;
    using Default = std::integral_constant<flag::ValueT, VALUE>;
    using Select = std::conditional_t<HasFlag::value, FlagValue, Default>;

    static constexpr flag::ValueT value = Select::value;
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
