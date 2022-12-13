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

#include "ir_isa.h"
#include "macros.h"
#include "marker.h"
#include "typedefs.h"

enum Opcode : uint8_t
{
#define OPCODES(op, ...) op,
    INSTRUCTION_LIST(OPCODES)
#undef OPCODES
        N_OPCODES
};

enum InstFlags : uint8_t
{
    EMPTY = 0b0,
    NO_DCE = 0b00000001,
    SYMMETRY = 0b00000010,
};

enum class DataType : uint8_t
{
    NO_TYPE,
    INT,
    FLOAT,
    DOUBLE,
    VOID,
    ANY
};

enum class CondType : uint8_t
{
    COND_INVALID,
    COND_EQ,
    COND_NEQ,
    COND_LEQ,
    COND_GEQ,
    COND_L,
    CONSD_G
};

class Inst;
class BasicBlock;
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

    template <Opcode OPCODE, typename... Args>
    static std::unique_ptr<Inst> NewInst(Args&&... args);

    static constexpr size_t MAX_INPUTS = std::numeric_limits<size_t>::max();

    GETTER_SETTER(Prev, Inst*, prev_);
    GETTER_SETTER(BasicBlock, BasicBlock*, bb_);
    GETTER_SETTER(DataType, DataType, data_type_);
    GETTER(Inputs, inputs_);
    GETTER(Opcode, opcode_);
    GETTER(Users, users_);
    GETTER(Id, id_);

    void SetInput(size_t idx, Inst* inst);
    void ReplaceInput(Inst* old_inst, Inst* new_inst);
    void ClearInput(Inst* old_inst);

    void AddUser(Inst* inst);
    void AddUser(Inst* inst, size_t idx);
    void AddUser(const User& user);
    void RemoveUser(const User& user);
    void RemoveUser(Inst* user);
    void ReplaceUser(const User& user_old, const User& user_new);

    bool HasFlag(InstFlags flag) const
    {
        constexpr std::array<uint8_t, Opcode::N_OPCODES> FLAGS_MAP{
#define GET_FLAGS(OP, TYPE, FLAGS, ...) FLAGS,
            INSTRUCTION_LIST(GET_FLAGS)
#undef GET_FLAGS
        };
        return FLAGS_MAP[opcode_] & flag;
    }

    template <Opcode OPCODE, InstFlags FLAG>
    static constexpr bool HasFlag()
    {
        constexpr std::array<uint8_t, Opcode::N_OPCODES> FLAGS_MAP{
#define GET_FLAGS(OP, TYPE, FLAGS, ...) FLAGS,
            INSTRUCTION_LIST(GET_FLAGS)
#undef GET_FLAGS
        };
        return FLAGS_MAP[OPCODE] & FLAG;
    }

    Inst* GetNext() const
    {
        return next_.get();
    }

    void SetNext(std::unique_ptr<Inst> next)
    {
        next_ = std::move(next);
    }

    void SetNext(Inst* next)
    {
        next_.reset(next);
    }

    Inst* ReleaseNext()
    {
        return next_.release();
    }

    bool IsPhi() const
    {
        return opcode_ == Opcode::PHI;
    }

    bool IsConst() const
    {
        return opcode_ == Opcode::CONST;
    }

    bool IsParam() const
    {
        return opcode_ == Opcode::PARAM;
    }

    bool IsCond() const
    {
        return opcode_ == Opcode::IF || opcode_ == Opcode::IF_IMM || opcode_ == Opcode::CMP;
    }

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
    explicit Inst(Opcode op) : id_(RecieveId()), opcode_(op)
    {
    }

    static IdType RecieveId()
    {
        static IdType INST_ID_COUNTER = 0;
        return INST_ID_COUNTER++;
    }

    inline bool IsNotTypeSensitive() const
    {
        return opcode_ == Opcode::RETURN || opcode_ == Opcode::RETURN_VOID ||
               opcode_ == Opcode::PARAM || opcode_ == Opcode::CONST;
    }

    std::unique_ptr<Inst> next_{ nullptr };
    Inst* prev_{ nullptr };

    const IdType id_{};

    Opcode opcode_;
    DataType data_type_ = DataType::NO_TYPE;
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
        std::cout << "\timmediate: " << imm_ << "\n";
    }

  private:
    ImmType imm_{ 0 };
};

class HasCond
{
  public:
    explicit HasCond(CondType cc) : cc_(cc)
    {
    }

    GETTER_SETTER(Cond, CondType, cc_);

    void Dump() const
    {
        static const std::unordered_map<CondType, std::string> cond_to_str = {
            { CondType::COND_INVALID, "invalid" },
            { CondType::COND_EQ, "==" },
            { CondType::COND_NEQ, "!=" },
            { CondType::COND_LEQ, "<=" },
            { CondType::COND_GEQ, ">=" },
            { CondType::COND_L, "<" },
            { CondType::CONSD_G, ">" }
        };
        std::cout << "\tcondition type: " << cond_to_str.at(cc_) << "\n";
    }

  private:
    CondType cc_{ CondType::COND_INVALID };
};
template <size_t N>
class FixedInputOp : public Inst
{
  public:
    FixedInputOp(Opcode op) : Inst(op)
    {
        inputs_.resize(N);
    }

    static constexpr size_t N_INPUTS = N;
};

class FixedInputOp0 : public FixedInputOp<0>
{
  public:
    FixedInputOp0(Opcode op) : FixedInputOp(op)
    {
    }
};

class FixedInputOp1 : public FixedInputOp<1>
{
  public:
    FixedInputOp1(Opcode op) : FixedInputOp(op)
    {
    }
};

class BinaryOp : public FixedInputOp<2>
{
  public:
    BinaryOp(const Opcode op) : FixedInputOp(op)
    {
    }
};

class BinaryImmOp : public FixedInputOp<1>, public HasImm
{
  public:
    BinaryImmOp(Opcode op, ImmType imm = 0) : FixedInputOp(op), HasImm(imm)
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
    CompareOp(Opcode, CondType cc = CondType::COND_INVALID)
        : FixedInputOp(Opcode::CMP), HasCond(cc)
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
    ConstantOp(Opcode) : Inst(Opcode::CONST)
    {
    }
    template <typename T>
    ConstantOp(Opcode, T val) : Inst(Opcode::CONST)
    {
        if constexpr (std::numeric_limits<T>::is_integer) {
            SetDataType(DataType::INT);
            val_ = val;
        } else if constexpr (std::is_same_v<T, float>) {
            SetDataType(DataType::FLOAT);
            val_ = std::bit_cast<uint32_t, float>(val);
        } else if constexpr (std::is_same_v<T, double>) {
            SetDataType(DataType::DOUBLE);
            val_ = std::bit_cast<uint64_t, double>(val);
        } else {
            SetDataType(DataType::NO_TYPE);
            assert(false);
        }
    }
    ConstantOp(Opcode, uint64_t val_raw, DataType type) : Inst(Opcode::CONST), val_(val_raw)
    {
        SetDataType(type);
    }

    uint64_t GetValRaw() const
    {
        return val_;
    }

    int64_t GetValInt() const
    {
        assert(GetDataType() == DataType::INT);
        return (int64_t)val_;
    }

    double GetValDouble() const
    {
        assert(GetDataType() == DataType::DOUBLE);
        return std::bit_cast<float, uint32_t>((uint32_t)val_);
    }

    float GetValFloat() const
    {
        assert(GetDataType() == DataType::FLOAT);
        return std::bit_cast<double, uint64_t>(val_);
    }

    void Dump() const override
    {
        Inst::Dump();
        std::cout << "\tconst value: " << GetValRaw() << "\n";
    }

  private:
    uint64_t val_{ 0 };
};

class ParamOp : public Inst
{
  public:
    static constexpr ArgNumType ARG_N_INVALID = std::numeric_limits<ArgNumType>::max();

    ParamOp(Opcode, ArgNumType arg_n = ARG_N_INVALID) : Inst(Opcode::PARAM)
    {
        arg_n_ = arg_n;
    }

    GETTER_SETTER(ArgNumber, ArgNumType, arg_n_);

    void Dump() const override
    {
        Inst::Dump();
        std::cout << "\tparam number: " << arg_n_ << "\n";
    }

  private:
    ArgNumType arg_n_{ ARG_N_INVALID };
};

class PhiOp : public Inst
{
  public:
    PhiOp(Opcode) : Inst(Opcode::PHI)
    {
    }

    void Dump() const override
    {
        Inst::Dump();
    }

    size_t AddInput(Inst* inst, BasicBlock* bb);
    size_t AddInput(const Input& input);

    void ClearInputs()
    {
        for (const auto& in : inputs_) {
            in.GetInst()->RemoveUser(this);
        }
        inputs_.clear();
    }

    void RemoveInput(const Input& input);
    Input GetInput(const BasicBlock* bb);
};

class IfOp : public FixedInputOp<2>, public HasCond
{
  public:
    IfOp(Opcode, CondType cc = CondType::COND_INVALID) : FixedInputOp(Opcode::IF), HasCond(cc)
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
    IfImmOp(Opcode, ImmType imm = 0, CondType cc = CondType::COND_INVALID)
        : FixedInputOp(Opcode::IF_IMM), HasCond(cc), HasImm(imm)
    {
    }
};

template <Opcode OPCODE, typename... Args>
std::unique_ptr<Inst> Inst::NewInst(Args&&... args)
{
    return std::unique_ptr<Inst::ToInstType<OPCODE> >(
        new Inst::ToInstType<OPCODE>(OPCODE, std::forward<Args>(args)...));
}

#endif
