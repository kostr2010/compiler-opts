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
};

enum class InstType : uint8_t
{
#define TYPES(t) t,
    INSTRUCTION_TYPES(TYPES)
#undef TYPES
};

enum InstFlags : uint8_t
{
    EMPTY = 0b0,
    NO_DCE = 0b00000001
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
    DEFAULT_CTOR(User);
    DEFAULT_DTOR(User);

    GETTER_SETTER(Inst, Inst*, inst_);
    GETTER_SETTER(Idx, int, idx_);

  private:
    Inst* inst_{ nullptr };
    int idx_{ -1 };
};

class Inst : public marking::Markable
{
  public:
    NO_DEFAULT_CTOR(Inst);
    DEFAULT_DTOR(Inst);

    template <typename IType, typename... Args>
    static std::unique_ptr<IType> NewInst(Args&&... args)
    {
        return std::unique_ptr<IType>(new IType(std::forward<Args>(args)...));
    }

    static constexpr size_t MAX_INPUTS = std::numeric_limits<size_t>::max();
    static constexpr size_t N_INPUTS = 0;

    GETTER_SETTER(Prev, Inst*, prev_);
    GETTER_SETTER(BasicBlock, BasicBlock*, bb_);
    GETTER(InstType, inst_type_);
    GETTER_SETTER(DataType, DataType, data_type_);
    GETTER(Inputs, inputs_);
    GETTER(Opcode, opcode_);
    GETTER(Flags, flags_);
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
    explicit Inst(Opcode op, InstType t) : id_(RecieveId()), opcode_(op), inst_type_(t)
    {
        static constexpr std::array FLAGS{
#define OPCODES(op, type, flags) flags,
            INSTRUCTION_LIST(OPCODES)
#undef OPCODES
        };

        flags_ = FLAGS[op];
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
    InstType inst_type_;
    DataType data_type_ = DataType::NO_TYPE;
    BasicBlock* bb_{ nullptr };

    uint8_t flags_ = 0;

    std::list<User> users_{};
    std::vector<Input> inputs_{};
};

class HasImm
{
  public:
    explicit HasImm(ImmType imm) : imm_(imm)
    {
    }
    DEFAULT_CTOR(HasImm);
    DEFAULT_DTOR(HasImm);

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
    DEFAULT_CTOR(HasCond);
    DEFAULT_DTOR(HasCond);

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
    FixedInputOp(Opcode op, InstType t) : Inst(op, t)
    {
        inputs_.resize(N);
    }
    DEFAULT_CTOR(FixedInputOp);
    DEFAULT_DTOR(FixedInputOp);

    static constexpr size_t N_INPUTS = N;

  private:
};

class FixedInputOp0 : public FixedInputOp<0>
{
  public:
    FixedInputOp0(Opcode op) : FixedInputOp(op, InstType::FixedInputOp0)
    {
    }
    DEFAULT_DTOR(FixedInputOp0);
};

class FixedInputOp1 : public FixedInputOp<1>
{
  public:
    FixedInputOp1(Opcode op) : FixedInputOp(op, InstType::FixedInputOp1)
    {
    }
    DEFAULT_DTOR(FixedInputOp1);
};

class BinaryOp : public FixedInputOp<2>
{
  public:
    BinaryOp(const Opcode op) : FixedInputOp(op, InstType::BinaryOp)
    {
    }
    DEFAULT_CTOR(BinaryOp);
    DEFAULT_DTOR(BinaryOp);
};

class BinaryImmOp : public FixedInputOp<1>, public HasImm
{
  public:
    BinaryImmOp(Opcode op, ImmType imm = 0) : FixedInputOp(op, InstType::BinaryImmOp), HasImm(imm)
    {
    }
    DEFAULT_CTOR(BinaryImmOp);
    DEFAULT_DTOR(BinaryImmOp);

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
        : FixedInputOp(Opcode::CMP, InstType::CompareOp), HasCond(cc)
    {
    }
    DEFAULT_CTOR(CompareOp);
    DEFAULT_DTOR(CompareOp);

    void Dump() const override
    {
        Inst::Dump();
        HasCond::Dump();
    }
};

class ConstantOp : public Inst
{
  public:
    ConstantOp() : Inst(Opcode::CONST, InstType::ConstantOp)
    {
    }
    template <typename T>
    ConstantOp(T val) : Inst(Opcode::CONST, InstType::ConstantOp)
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
    ConstantOp(uint64_t val_raw, DataType type)
        : Inst(Opcode::CONST, InstType::ConstantOp), val_(val_raw)
    {
        SetDataType(type);
    }
    DEFAULT_DTOR(ConstantOp);

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

    ParamOp(Opcode op, ArgNumType arg_n = ARG_N_INVALID) : Inst(op, InstType::ParamOp)
    {
        arg_n_ = arg_n;
    }
    DEFAULT_CTOR(ParamOp);
    DEFAULT_DTOR(ParamOp);

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
    PhiOp(Opcode op) : Inst(op, InstType::PhiOp)
    {
    }
    DEFAULT_CTOR(PhiOp);
    DEFAULT_DTOR(PhiOp);

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
    IfOp(Opcode, CondType cc = CondType::COND_INVALID)
        : FixedInputOp(Opcode::IF, InstType::IfOp), HasCond(cc)
    {
    }
    DEFAULT_CTOR(IfOp);
    DEFAULT_DTOR(IfOp);

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
        : FixedInputOp(Opcode::IF_IMM, InstType::IfImmOp), HasCond(cc), HasImm(imm)
    {
    }
    DEFAULT_CTOR(IfImmOp);
    DEFAULT_DTOR(IfImmOp);
};

#endif
