#ifndef ___INST_H_INCLUDED___
#define ___INST_H_INCLUDED___

#include <array>
#include <cassert>
#include <map>
#include <set>
#include <string>
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
    UINT,
    FLOAT
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
    Inst(Opcode op, InstType t) : opcode_(op), inst_type_(t){};
    ~Inst() = default;

    static constexpr int N_INPUTS = -1;

    Inst* GetPrev() const
    {
        return next_;
    }

    Inst* GetNext() const
    {
        return prev_;
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

    template <typename IType, typename... Args>
    static IType* NewInst(Opcode op, Args&&... args);

    virtual void SetImm(__attribute_maybe_unused__ ImmType imm)
    {
    }

  private:
    Inst* prev_;
    Inst* next_;

    IdType id_;

    Opcode opcode_ = Opcode::INVALID_OPCODE;
    InstType inst_type_ = InstType::INVALID_TYPE;
    DataType data_type_ = DataType::NO_TYPE;
    BasicBlock* bb_ = nullptr;

    // users
    // inputs
};

class BinaryOp : public Inst
{
  public:
    static constexpr int N_INPUTS = 0;

    BinaryOp(const Opcode& op) : Inst(op, InstType::BinaryOp)
    {
    }
};

class BinaryImmOp : public Inst
{
  public:
    static constexpr int N_INPUTS = 0;

    BinaryImmOp(Opcode op) : Inst(op, InstType::BinaryImmOp)
    {
    }

    void SetImm(ImmType imm) override
    {
        imm_ = imm;
    }

  private:
    ImmType imm_;
};

class CompareOp : public Inst
{
  public:
    CompareOp(Opcode op) : Inst(op, InstType::CompareOp)
    {
    }
    static constexpr int N_INPUTS = 0;
};

class ConstantOp : public Inst
{
  public:
    ConstantOp(Opcode op) : Inst(op, InstType::ConstantOp)
    {
    }
    static constexpr int N_INPUTS = 0;
};

class ParamOp : public Inst
{
  public:
    ParamOp(Opcode op) : Inst(op, InstType::ParamOp)
    {
    }
    static constexpr int N_INPUTS = 0;
};

class FixedInputOp1 : public Inst
{
  public:
    FixedInputOp1(Opcode op) : Inst(op, InstType::FixedInputOp1)
    {
    }
    static constexpr int N_INPUTS = 0;
};

class FixedInputOp0 : public Inst
{
  public:
    FixedInputOp0(Opcode op) : Inst(op, InstType::FixedInputOp0)
    {
    }
    static constexpr int N_INPUTS = 0;
};

class PhiOp : public Inst
{
  public:
    PhiOp(Opcode& op) : Inst(op, InstType::PhiOp)
    {
    }
    static constexpr int N_INPUTS = 0;
};

class IfOp : public Inst
{
  public:
    IfOp(Opcode& op) : Inst(op, InstType::IfOp)
    {
    }
    static constexpr int N_INPUTS = 0;
};

template <typename IType, typename... Args>
IType* Inst::NewInst(Opcode op, Args&&... args)
{
    static_assert(IType::N_INPUTS >= 0);

    return new IType(op, args...);

    // switch (IType::N_INPUTS) {
    // case 0:

    // default:
    //     auto data = new Data<IType::N_INPUTS>;
    //     return new ((void*)(data + sizeof(Data<IType::N_INPUTS>))) IType(args...);
    // }
}

#endif