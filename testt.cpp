#include <algorithm>
#include <iostream>
#include <limits>
#include <type_traits>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

// file for tomfoolery and experiments

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <tuple>

#include "ir_/isa/isa.h"

template <typename INST, typename ACC>
struct BranchNumAccumulator
{
    using Opcode = typename INST::Type;
    using BranchFlagType = isa::flag::Flag<isa::flag::Kind::BRANCH>::Type;
    using HasBranch = isa::HasFlag<Opcode::value, BranchFlagType::value>;
    using BranchValue = isa::FlagValue<Opcode::value, BranchFlagType::value>;
    using FlagValueZero = std::integral_constant<isa::flag::ValueT, 0>;
    using BranchNumOrZero = std::conditional_t<HasBranch::value, BranchValue, FlagValueZero>;
    using IsNewMax = std::integral_constant < bool, ACC::value<BranchNumOrZero::value>;

    static constexpr isa::flag::ValueT value =
        (ACC::value * !IsNewMax::value) + (BranchNumOrZero::value * IsNewMax::value);
};

void PrintConditional(isa::inst::Opcode opcode)
{
#define GEN_CONDITIONAL_DISPATCH(OPCODE, TYPE, ...)                                               \
    if constexpr (isa::InputValue<isa::inst::Opcode::OPCODE, isa::input::Kind::COND>::value) {    \
        if (opcode == isa::inst::Opcode::OPCODE) {                                                \
            std::cout << #TYPE << "\n";                                                           \
            return;                                                                               \
        }                                                                                         \
    }
    ISA_INSTRUCTION_LIST(GEN_CONDITIONAL_DISPATCH)
#undef GEN_CONDITIONAL_DISPATCH

    assert(false);
}

template <isa::inst::Opcode OPCODE>
using IsDynamic = isa::InputValue<OPCODE, isa::input::Kind::DYN>;

template <isa::inst::Opcode OPCODE>
using IsCond = isa::InputValue<OPCODE, isa::input::Kind::COND>;

int main()
{
    std::cout
        << (int)type_sequence::Accumulate<isa::ISA, BranchNumAccumulator,
                                          std::integral_constant<isa::flag::ValueT, 0> >::value
        << "\n";

    std::cout << isa::EvaluatePredicate<IsDynamic>(isa::inst::Opcode::CALL_STATIC) << "\n";
    std::cout << isa::InputValue<isa::inst::Opcode::ADD, isa::input::Kind::VREG>::value << "\n";

    return 0;
}

#pragma GCC diagnostic pop
