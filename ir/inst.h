#ifndef ___INST_H_INCLUDED___
#define ___INST_H_INCLUDED___

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <list>
#include <memory>
#include <numeric>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "flag.h"
#include "ir_isa.h"
#include "macros.h"
#include "markable.h"
#include "typedefs.h"

class Inst;
class BasicBlock;
class Graph;
class Input
{
  public:
    explicit Input(Inst* inst, BasicBlock* bb) : inst_(inst), bb_(bb)
    {
    }
    DEFAULT_CTOR(Input);

    GETTER_SETTER(Inst, Inst*, inst_);
    GETTER_SETTER(SourceBB, BasicBlock*, bb_);

  private:
    Inst* inst_{ nullptr };
    BasicBlock* bb_{ nullptr };
};

class User
{
  public:
    explicit User(Inst* inst) : inst_(inst)
    {
    }
    explicit User(Inst* inst, int idx) : inst_(inst), idx_(idx)
    {
    }

    GETTER_SETTER(Inst, Inst*, inst_);
    GETTER_SETTER(Idx, int, idx_);

  private:
    Inst* inst_{ nullptr };
    int idx_{ -1 };
};

#define FORWARD_DECL(T, ...) class T;
INSTRUCTION_TYPES(FORWARD_DECL);
#undef FORWARD_DECL

class Inst : public marking::Markable
{
  protected:
    static constexpr size_t MAX_INPUTS = std::numeric_limits<uint8_t>::max();

  private:
#define GET_TYPES(OP, T, ...) T,
    using InstTypes = std::tuple<INSTRUCTION_LIST(GET_TYPES) void>;
#undef GET_TYPES

    template <size_t I, typename TUPLE>
    struct get_inst_type_t;

    template <size_t I, typename Type, typename... Types>
    struct get_inst_type_t<I, std::tuple<Type, Types...> >
        : get_inst_type_t<I - 1, std::tuple<Types...> >
    {};

    template <typename Type, typename... Types>
    struct get_inst_type_t<0, std::tuple<Type, Types...> >
    {
        using type = Type;
    };

    template <typename InstType, typename = void>
    struct get_num_inputs_t : std::integral_constant<size_t, MAX_INPUTS>
    {
        static_assert(std::is_base_of<Inst, InstType>::value);
    };

    template <typename InstType>
    struct get_num_inputs_t<InstType, std::void_t<decltype(InstType::N_INPUTS)> >
        : std::integral_constant<size_t, InstType::N_INPUTS>
    {
        static_assert(std::is_base_of<Inst, InstType>::value);
    };

    using FlagType = uint8_t;
    using NoDCE = Flag<FlagType>;
    using Symmetry = NoDCE::Next;
    using Call = Symmetry::Next;
    using Check = Call::Next;

  public:
    enum Flags : FlagType
    {
        NO_DCE = NoDCE::Value(),
        SYMMETRY = Symmetry::Value(),
        CALL = Call::Value(),
        CHECK = Check::Value(),
    };

    enum Opcode : uint8_t
    {
#define OPCODES(op, ...) op,
        INSTRUCTION_LIST(OPCODES)
#undef OPCODES
            N_OPCODES
    };

    enum class DataType : uint8_t
    {
        INT,
        FLOAT,
        DOUBLE,
        VOID,
        ANY
    };

    enum class Cond : uint8_t
    {
        EQ,
        NEQ,
        LEQ,
        GEQ,
        L,
        G
    };

    template <typename InstType>
    using get_num_inputs = get_num_inputs_t<InstType>;

    template <Inst::Opcode OPCODE>
    using to_inst_type = typename get_inst_type_t<OPCODE, InstTypes>::type;

    template <Inst::Opcode OPCODE>
    using has_dynamic_operands = std::is_base_of<VariableInputOp, to_inst_type<OPCODE> >;

    template <Inst::Opcode OPCODE, typename... Args>
    static std::unique_ptr<Inst> NewInst(Args&&... args);

    GETTER_SETTER(Prev, Inst*, prev_);
    GETTER_SETTER(BasicBlock, BasicBlock*, bb_);
    GETTER_SETTER(DataType, Inst::DataType, data_type_);
    GETTER(Inputs, inputs_);
    GETTER(Opcode, opcode_);
    GETTER(Users, users_);
    GETTER(Id, id_);

    size_t GetNumInputs() const;
    Input GetInput(size_t idx);
    void SetInput(size_t idx, Inst* inst);
    void ReplaceInput(Inst* old_inst, Inst* new_inst);
    void ClearInput(Inst* old_inst);
    size_t AddInput(Inst* inst, BasicBlock* bb);
    size_t AddInput(const Input& input);

    void ReserveInputs(size_t n);
    void ClearInputs();
    void RemoveInput(const Input& input);

    void AddUser(Inst* inst);
    void AddUser(Inst* inst, size_t idx);
    void AddUser(const User& user);
    void RemoveUser(const User& user);
    void RemoveUser(Inst* user);
    void ReplaceUser(const User& user_old, const User& user_new);

    template <Inst::Opcode OPCODE, Inst::Flags FLAG>
    static constexpr bool HasFlag()
    {
        constexpr std::array<uint8_t, Inst::Opcode::N_OPCODES> FLAGS_MAP{
#define GET_FLAGS(OP, TYPE, FLAGS, ...) FLAGS,
            INSTRUCTION_LIST(GET_FLAGS)
#undef GET_FLAGS
        };
        return FLAGS_MAP[OPCODE] & FLAG;
    }

    bool HasFlag(Inst::Flags flag) const;
    bool HasDynamicOperands() const;

    Inst* GetNext() const;
    void SetNext(std::unique_ptr<Inst> next);
    void SetNext(Inst* next);
    Inst* ReleaseNext();

    bool IsPhi() const;
    bool IsConst() const;
    bool IsParam() const;
    bool IsCall() const;
    bool IsReturn() const;
    bool IsCond() const;

    bool Precedes(const Inst* inst) const;
    bool Dominates(const Inst* inst) const;

    bool IsTypeSensitive() const
    {
        return !IsNotTypeSensitive();
    }

    bool CheckInputType()
    {
        // FIXME:
        return true;
    }

    virtual void Dump() const;

  protected:
    explicit Inst(Inst::Opcode op) : id_(RecieveId()), opcode_(op)
    {
    }

    static IdType RecieveId()
    {
        static IdType INST_ID_COUNTER = 0;
        return INST_ID_COUNTER++;
    }

    inline bool IsNotTypeSensitive() const
    {
        return opcode_ == Inst::Opcode::RETURN || opcode_ == Inst::Opcode::RETURN_VOID ||
               opcode_ == Inst::Opcode::PARAM || opcode_ == Inst::Opcode::CONST;
    }

    std::unique_ptr<Inst> next_{ nullptr };
    Inst* prev_{ nullptr };

    const IdType id_{};

    Inst::Opcode opcode_;
    Inst::DataType data_type_ = Inst::DataType::VOID;
    BasicBlock* bb_{ nullptr };

    std::list<User> users_{};
    std::vector<Input> inputs_{};
};

class HasImm
{
  public:
    explicit HasImm(ImmType imm) : imm_(imm)
    {
    }

    GETTER_SETTER(Imm, ImmType, imm_);

    void Dump() const
    {
        std::cout << "#\timmediate: " << imm_ << "\n";
    }

  private:
    ImmType imm_{ 0 };
};

class HasCond
{
  public:
    explicit HasCond(Inst::Cond cc) : cc_(cc)
    {
    }

    GETTER_SETTER(Cond, Inst::Cond, cc_);

    void Dump() const
    {
        static const std::unordered_map<Inst::Cond, std::string> cond_to_str = {
            { Inst::Cond::EQ, "==" },  { Inst::Cond::NEQ, "!=" }, { Inst::Cond::LEQ, "<=" },
            { Inst::Cond::GEQ, ">=" }, { Inst::Cond::L, "<" },    { Inst::Cond::G, ">" }
        };
        std::cout << "#\tcondition type: " << cond_to_str.at(cc_) << "\n";
    }

  private:
    Inst::Cond cc_;
};
template <size_t N>
class FixedInputOp : public Inst
{
  public:
    FixedInputOp(Inst::Opcode op) : Inst(op)
    {
        inputs_.resize(N);
    }

    static constexpr size_t N_INPUTS = N;
};

class FixedInputOp0 : public FixedInputOp<0>
{
  public:
    FixedInputOp0(Inst::Opcode op) : FixedInputOp(op)
    {
    }
};

class FixedInputOp1 : public FixedInputOp<1>
{
  public:
    FixedInputOp1(Inst::Opcode op) : FixedInputOp(op)
    {
    }
};

class BinaryOp : public FixedInputOp<2>
{
  public:
    BinaryOp(const Inst::Opcode op) : FixedInputOp(op)
    {
    }
};

class BinaryImmOp : public FixedInputOp<1>, public HasImm
{
  public:
    BinaryImmOp(Inst::Opcode op, ImmType imm = 0) : FixedInputOp(op), HasImm(imm)
    {
    }

    void Dump() const override
    {
        Inst::Dump();
        HasImm::Dump();
    }
};

class CompareOp : public FixedInputOp<2>, public HasCond
{
  public:
    CompareOp(Inst::Opcode, Inst::Cond cc) : FixedInputOp(Inst::Opcode::CMP), HasCond(cc)
    {
    }

    void Dump() const override
    {
        Inst::Dump();
        HasCond::Dump();
    }
};

class ConstantOp : public Inst
{
  public:
    template <typename T>
    ConstantOp(Inst::Opcode, T val) : Inst(Inst::Opcode::CONST)
    {
        if constexpr (std::numeric_limits<T>::is_integer) {
            SetDataType(Inst::DataType::INT);
            val_ = val;
        } else if constexpr (std::is_same_v<T, float>) {
            SetDataType(Inst::DataType::FLOAT);
            val_ = std::bit_cast<uint32_t, float>(val);
        } else if constexpr (std::is_same_v<T, double>) {
            SetDataType(Inst::DataType::DOUBLE);
            val_ = std::bit_cast<uint64_t, double>(val);
        } else {
            SetDataType(Inst::DataType::VOID);
            assert(false);
        }
    }

    uint64_t GetValRaw() const
    {
        return val_;
    }

    int64_t GetValInt() const
    {
        assert(GetDataType() == Inst::DataType::INT);
        return (int64_t)val_;
    }

    double GetValDouble() const
    {
        assert(GetDataType() == Inst::DataType::DOUBLE);
        return std::bit_cast<float, uint32_t>((uint32_t)val_);
    }

    float GetValFloat() const
    {
        assert(GetDataType() == Inst::DataType::FLOAT);
        return std::bit_cast<double, uint64_t>(val_);
    }

    void Dump() const override
    {
        Inst::Dump();
        std::cout << "#\tconst value: " << GetValRaw() << "\n";
    }

  private:
    uint64_t val_{ 0 };
};

class ParamOp : public Inst
{
  public:
    static constexpr size_t ARG_N_INVALID = std::numeric_limits<size_t>::max();

    ParamOp(Inst::Opcode, size_t arg_n = ARG_N_INVALID) : Inst(Inst::Opcode::PARAM)
    {
        arg_n_ = arg_n;
    }

    GETTER_SETTER(ArgNumber, size_t, arg_n_);

    void Dump() const override
    {
        Inst::Dump();
        std::cout << "#\tparam number: " << arg_n_ << "\n";
    }

  private:
    size_t arg_n_{ ARG_N_INVALID };
};

class VariableInputOp : public Inst
{
  public:
    VariableInputOp(Inst::Opcode opcode) : Inst(opcode)
    {
    }
};

class CallOp : public VariableInputOp
{
  public:
    CallOp(Inst::Opcode opcode, Graph* callee) : VariableInputOp(opcode), callee_(callee)
    {
        assert(HasFlag(Inst::Flags::CALL));
    }
    GETTER_SETTER(Callee, Graph*, callee_);

  private:
    Graph* callee_;
};

class PhiOp : public VariableInputOp
{
  public:
    PhiOp(Inst::Opcode) : VariableInputOp(Inst::Opcode::PHI)
    {
    }
};

class IfOp : public FixedInputOp<2>, public HasCond
{
  public:
    IfOp(Inst::Opcode, Inst::Cond cc) : FixedInputOp(Inst::Opcode::IF), HasCond(cc)
    {
    }

    void Dump() const override
    {
        Inst::Dump();
        HasCond::Dump();
    }
};

class IfImmOp : public FixedInputOp<1>, public HasCond, public HasImm
{
  public:
    IfImmOp(Inst::Opcode, ImmType imm, Inst::Cond cc)
        : FixedInputOp(Inst::Opcode::IF_IMM), HasCond(cc), HasImm(imm)
    {
    }
};

template <Inst::Opcode OPCODE, typename... Args>
std::unique_ptr<Inst> Inst::NewInst(Args&&... args)
{
    return std::unique_ptr<Inst::to_inst_type<OPCODE> >(
        new Inst::to_inst_type<OPCODE>(OPCODE, std::forward<Args>(args)...));
}

#endif
