#include <algorithm>
#include <iostream>
#include <limits>
#include <type_traits>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

// file for tomfoolery and experiments

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>

constexpr char* my_copy(char* dst, const char* src, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        std::cout << "<" << src[i] << ">\n";
        dst[i] = src[i];
        if (dst[i] == '\0') {
            std::cout << i + 1 << "&&\n";
            return &dst[i + 1];
        }
    }

    std::cout << "*\n";

    return nullptr;
}

template <size_t N1, size_t N2>
constexpr auto concat(char const (&a)[N1], char const (&b)[N2])
{
    std::array<char, N1 + N2 - 1> result = {};
    std::cout << N1 + N2 - 1 << "\n";
    char* next = my_copy(result.data(), a, N1 - 1);
    std::cout << "> " << N1 - 1 << "\n";
    char* nextdst = next ? next : &result[N1 - 1];
    my_copy(nextdst, b, N2);
    result.back() = '\0';
    return result;
}

#include "ir/ir_isa.h"

enum Opcode : uint8_t
{
#define OPCODES(op, ...) op,
    INSTRUCTION_LIST(OPCODES)
#undef OPCODES
};

#define FORWARD_DECL(T, ...) class T;
INSTRUCTION_TYPES(FORWARD_DECL);
#undef FORWARD_DECL

struct Inst
{
#define GET_TYPES(OP, T, ...) T,
    using InstTypes = std::tuple<INSTRUCTION_LIST(GET_TYPES) void>;
#undef GET_TYPES

    template <size_t I, typename TUPLE>
    struct GetInstTypeT;

    template <size_t I, typename Type, typename... Types>
    struct GetInstTypeT<I, std::tuple<Type, Types...> >
        : GetInstTypeT<I - 1, std::tuple<Types...> >
    {};

    template <typename Type, typename... Types>
    struct GetInstTypeT<0, std::tuple<Type, Types...> >
    {
        using type = Type;
    };

  public:
    template <Opcode OPCODE>
    using ToInstType = typename GetInstTypeT<OPCODE, InstTypes>::type;

    template <typename InstType, typename = void>
    struct GetNumInputs : std::integral_constant<size_t, std::numeric_limits<size_t>::max()>
    {};

    template <typename InstType>
    struct GetNumInputs<InstType, std::void_t<decltype(InstType::N_INPUTS)> >
        : std::integral_constant<size_t, InstType::N_INPUTS>
    {};

    int a{};
};

template <size_t N>
class FixedInputOp : public Inst
{

  public:
    FixedInputOp()
    {
    }

  private:
    friend Inst;
    template <typename T>
    friend struct Inst::GetNumInputs;
    static constexpr size_t N_INPUTS = N;

  public:
};

class FixedInputOp0 : public FixedInputOp<0>
{
  public:
    FixedInputOp0() : FixedInputOp()
    {
    }
};

class FixedInputOp1 : public FixedInputOp<1>
{
  public:
    FixedInputOp1() : FixedInputOp()
    {
    }
};

class kek : public FixedInputOp1
{
  public:
    kek() : FixedInputOp1()
    {
    }
};

class kek2 : public FixedInputOp<2>
{
  public:
    kek2()
    {
    }
};

int main()
{
    auto t = concat("Hello, ", "world!");
    for (const auto& c : t) {
        std::cout << "<" << c << ">\n";
    }

    std::cout << Inst::GetNumInputs<kek2>() << "\n";
    std::cout << Inst::GetNumInputs<kek>() << "\n";
    std::cout << Inst::GetNumInputs<FixedInputOp1>() << "\n";
    std::cout << Inst::GetNumInputs<FixedInputOp0>() << "\n";

    return 0;
}

#pragma GCC diagnostic pop
