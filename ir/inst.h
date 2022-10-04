#ifndef ___INST_H_INCLUDED___
#define ___INST_H_INCLUDED___

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ir_isa.h"

#include "typedefs.h"

enum class Opcode : uint8_t
{
    INVALID_OPCODE,
#define OPCODES(op, ...) op,
    INSTRUCTION_LIST(OPCODES)
#undef OPCODES
};

enum class InstType : uint8_t
{
    INVALID_TYPE,
#define TYPES(t) t,
    INSTRUCTION_TYPES(TYPES)
#undef TYPES
};

enum class DataType : uint8_t
{
    NO_TYPE,
    INT,
    FLOAT,
    DOUBLE
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
    // ...
};

class Input
{
    uint64_t id;
};

template <size_t N_INPUTS>
struct Data
{
    std::array<Input, N_INPUTS> inp;
};

class BasicBlock;

class Inst
{
  public:
    Inst(Opcode op, InstType t, size_t n_inp = 0) : opcode_(op), inst_type_(t), n_inputs_(n_inp)
    {
        inputs_.reserve(n_inputs_);
    };
    Inst() = default;
    ~Inst() = default;

    static constexpr Reg INVALID_REG = -1;
    static constexpr size_t MAX_INPUTS = std::numeric_limits<size_t>::max();

    Inst* GetPrev() const
    {
        return prev_;
    }

    Inst* GetNext() const
    {
        return next_;
    }

    void SetPrev(Inst* inst)
    {
        prev_ = inst;
    }

    void SetNext(Inst* inst)
    {
        next_ = inst;
    }

    void SetBasicBlock(BasicBlock* bb)
    {
        bb_ = bb;
    }

    BasicBlock* GetBasicBlock()
    {
        return bb_;
    }

    void SetId(IdType id)
    {
        id_ = id;
    }

    IdType GetId() const
    {
        return id_;
    }

    Opcode GetOpcode() const
    {
        return opcode_;
    }

    void SetOpcode(Opcode op)
    {
        opcode_ = op;
    }

    InstType GetInstType() const
    {
        return inst_type_;
    }

    void SetInstType(InstType i)
    {
        inst_type_ = i;
    }

    DataType GetDataType() const
    {
        return data_type_;
    }

    void SetDataType(DataType d)
    {
        data_type_ = d;
    }

    virtual Reg GetInputReg(RegNumType)
    {
        return INVALID_REG;
    }

    virtual void SetInputReg(RegNumType, Reg)
    {
        return;
    }

    virtual Reg GetOutputReg(RegNumType)
    {
        return GetOutputReg();
    }

    virtual void SetOutputReg(RegNumType, Reg reg)
    {
        SetOutputReg(reg);
    }

    Reg GetOutputReg()
    {
        return output_reg_;
    }

    void SetOutputReg(Reg reg)
    {
        output_reg_ = reg;
    }

    size_t GetNInputs() const
    {
        return n_inputs_;
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

    // used for fixed number of arguments
    void SetInput(size_t idx, Inst* inst)
    {
        assert(inst != nullptr);
        assert(idx < n_inputs_);

        inst->users_.insert(this);
        inputs_[idx] = inst;
    }

    // used for unfixed number of arguments
    void AddInput(Inst* inst)
    {
        assert(inst != nullptr);
        assert(inputs_.size() + 1 < MAX_INPUTS);

        inst->users_.insert(this);
        inputs_.push_back(inst);
    }

    template <typename IType, typename... Args>
    static IType* NewInst(Args&&... args);

    void Dump()
    {
        std::cout << "\tinst_id:....." << id_ << "\n";
        std::cout << "\tinst_opcode:." << (int)opcode_ << "\n";
        std::cout << "\tinst_type:..." << (int)inst_type_ << "\n";
        std::cout << "\tinst users:\n\t\t";

        for (auto i : users_) {
            std::cout << i->GetId() << " ";
        }
        std::cout << "\n";

        std::cout << "\tinst inputs:\n\t\t";
        for (auto i : inputs_) {
            std::cout << i->GetId() << " ";
        }
        std::cout << "\n";
    }

  private:
    Inst* prev_{ nullptr };
    Inst* next_{ nullptr };

    IdType id_;

    Opcode opcode_ = Opcode::INVALID_OPCODE;
    InstType inst_type_ = InstType::INVALID_TYPE;
    DataType data_type_ = DataType::NO_TYPE;
    BasicBlock* bb_ = nullptr;

    Reg output_reg_;
    size_t n_inputs_;

    std::set<Inst*> users_;
    std::vector<Inst*> inputs_;
};

class HasImm
{
  public:
    explicit HasImm(ImmType imm) : imm_(imm)
    {
    }
    HasImm() = default;
    ~HasImm() = default;

    void SetImm(ImmType imm)
    {
        imm_ = imm;
    }

    ImmType GetImm() const
    {
        return imm_;
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
    HasCond() = default;
    ~HasCond() = default;

    void SetCond(CondType cc)
    {
        cc_ = cc;
    }

    CondType GetCond() const
    {
        return cc_;
    }

  private:
    CondType cc_{ CondType::COND_INVALID };
};

// FIXME: now implemented as checking max arg number
// class HasVariableInputs
// {
//   public:
//     explicit HasVariableInputs(CondType cc)
//     {
//     }
//     HasVariableInputs() = default;
//     ~HasVariableInputs() = default;

//   private:
// };

// proxy for fixed input instructions
template <size_t N>
class FixedInputOp : public Inst
{
  public:
    FixedInputOp(Opcode op, InstType t) : Inst(op, t, N)
    {
        input_regs_.fill(INVALID_REG);
    }
    FixedInputOp() = default;
    ~FixedInputOp() = default;

    void SetInputReg(RegNumType idx, Reg reg) override
    {
        assert(idx < N);
        input_regs_[idx] = reg;
    }

    Reg GetInputReg(RegNumType idx) override
    {
        assert(idx < N);
        return input_regs_[idx];
    }

  private:
    std::array<Reg, N> input_regs_;
};

class FixedInputOp0 : public FixedInputOp<0>
{
  public:
    FixedInputOp0(Opcode op) : FixedInputOp(op, InstType::FixedInputOp0)
    {
    }
    ~FixedInputOp0() = default;
};

class FixedInputOp1 : public FixedInputOp<1>
{
  public:
    FixedInputOp1(Opcode op) : FixedInputOp(op, InstType::FixedInputOp1)
    {
    }
    ~FixedInputOp1() = default;
};

class BinaryOp : public FixedInputOp<2>
{
  public:
    BinaryOp(const Opcode op) : FixedInputOp(op, InstType::BinaryOp)
    {
    }
    BinaryOp() = default;
    ~BinaryOp() = default;

    // type ?
};

class BinaryImmOp : public FixedInputOp<1>, public HasImm
{
  public:
    BinaryImmOp(Opcode op, ImmType imm = 0) : FixedInputOp(op, InstType::BinaryImmOp), HasImm(imm)
    {
    }
    BinaryImmOp() = default;
    ~BinaryImmOp() = default;

    // type ?
};

class CompareOp : public FixedInputOp<2>, public HasCond
{
  public:
    CompareOp(Opcode, CondType cc = CondType::COND_INVALID)
        : FixedInputOp(Opcode::CMP, InstType::CompareOp), HasCond(cc)
    {
    }
    CompareOp() = default;
    ~CompareOp() = default;
};

class ConstantOp : public Inst
{
  public:
    ConstantOp(Opcode) : Inst(Opcode::CONST, InstType::ConstantOp)
    {
    }
    template <typename T>
    ConstantOp(Opcode, T val) : Inst(Opcode::CONST, InstType::ConstantOp)
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
    ConstantOp() = default;
    ~ConstantOp() = default;

    uint64_t GetValRaw() const
    {
        return val_;
    }

    uint64_t GetValInt() const
    {
        assert(GetDataType() == DataType::INT);
        return val_;
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
    ParamOp() = default;
    ~ParamOp() = default;

    ArgNumType GetArgNumber() const
    {
        return arg_n_;
    }

    void SetArgNumber(ArgNumType arg_n)
    {
        arg_n_ = arg_n;
    }

  private:
    ArgNumType arg_n_{ ARG_N_INVALID };
};

// FIXME: ???
class PhiOp : public Inst
{
  public:
    PhiOp(Opcode op) : Inst(op, InstType::PhiOp, 0)
    {
    }
    PhiOp() = default;
    ~PhiOp() = default;
};

class IfOp : public FixedInputOp<2>, public HasCond
{
  public:
    IfOp(Opcode, CondType cc = CondType::COND_INVALID)
        : FixedInputOp(Opcode::IF, InstType::IfOp), HasCond(cc)
    {
    }
    IfOp() = default;
    ~IfOp() = default;
};

class IfImmOp : public FixedInputOp<1>, public HasCond, public HasImm
{
  public:
    IfImmOp(Opcode, ImmType imm = 0, CondType cc = CondType::COND_INVALID)
        : FixedInputOp(Opcode::IF_IMM, InstType::IfImmOp), HasCond(cc), HasImm(imm)
    {
    }
    IfImmOp() = default;
    ~IfImmOp() = default;
};

template <typename IType, typename... Args>
IType* Inst::NewInst(Args&&... args)
{
    return new IType(std::forward<Args>(args)...);

    // switch (IType::N_INPUTS) {
    // case 0:

    // default:
    //     auto data = new Data<IType::N_INPUTS>;
    //     return new ((void*)(data + sizeof(Data<IType::N_INPUTS>)))
    //     IType(std::forward<Args>(args)...);
    // }
}

#endif