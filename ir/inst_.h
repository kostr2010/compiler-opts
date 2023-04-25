#ifndef ___INSTRUCTION__H_INCLUDED___
#define ___INSTRUCTION__H_INCLUDED___

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

#include "bit_flag.h"
#include "macros.h"
#include "marker/markable.h"
#include "typedefs.h"

#include "isa/isa.h"

class InstBase;
class BasicBlock;
class Graph;
class Input
{
  public:
    explicit Input(InstBase* inst, BasicBlock* bb) : inst_(inst), bb_(bb)
    {
    }
    DEFAULT_CTOR(Input);

    GETTER_SETTER(Inst, InstBase*, inst_);
    GETTER_SETTER(SourceBB, BasicBlock*, bb_);

  private:
    InstBase* inst_{ nullptr };
    BasicBlock* bb_{ nullptr };
};

class User
{
  public:
    explicit User(InstBase* inst) : inst_(inst)
    {
    }
    explicit User(InstBase* inst, int idx) : inst_(inst), idx_(idx)
    {
    }

    GETTER_SETTER(Inst, InstBase*, inst_);
    GETTER_SETTER(Idx, int, idx_);

  private:
    InstBase* inst_{ nullptr };
    int idx_{ -1 };
};

class InstBase : public marker::Markable
{
  public:
    enum class DataType : uint8_t
    {
        INT,
        FLOAT,
        DOUBLE,
        VOID,
        ANY
    };

    template <isa::inst::Opcode OPCODE, typename... Args>
    static std::unique_ptr<InstBase> NewInst(Args&&... args);
    virtual ~InstBase() = default;

    GETTER_SETTER(Prev, InstBase*, prev_);
    GETTER_SETTER(BasicBlock, BasicBlock*, bb_);
    GETTER_SETTER(DataType, DataType, data_type_);
    GETTER(Opcode, opcode_);
    GETTER(Inputs, inputs_);
    GETTER(Users, users_);
    GETTER(Id, id_);

    InstBase* GetNext() const;
    void SetNext(std::unique_ptr<InstBase> next);
    void SetNext(InstBase* next);
    InstBase* ReleaseNext();

    size_t GetNumInputs() const;
    Input GetInput(size_t idx) const;
    void SetInput(size_t idx, InstBase* inst);
    void ReplaceInput(InstBase* old_inst, InstBase* new_inst);
    void ClearInputs();
    void RemoveInput(const Input& input);
    void AddInput(InstBase* inst, BasicBlock* bb);
    void AddInput(const Input& input);

    size_t GetNumUsers() const;
    void AddUser(InstBase* inst);
    void AddUser(InstBase* inst, size_t idx);
    void AddUser(const User& user);
    void RemoveUser(const User& user);
    void RemoveUser(InstBase* user);
    void ReplaceUser(const User& user_old, const User& user_new);

    template <isa::flag::Type FLAG>
    struct FlagPredicate
    {
        template <isa::inst::Opcode OP>
        using Predicate = isa::HasFlag<OP, FLAG>;
    };

    template <isa::flag::Type FLAG>
    bool HasFlag() const
    {
        return isa::EvaluatePredicate<FlagPredicate<FLAG>::template Predicate>(opcode_);
    }

    size_t GetNumImms() const;
    bool IsConditional() const;
    bool IsDynamic() const;
    bool IsPhi() const;
    bool IsConst() const;
    bool IsParam() const;
    bool IsCall() const;
    bool IsCheck() const;
    bool IsReturn() const;

    bool Precedes(const InstBase* inst) const;
    bool Dominates(const InstBase* inst) const;

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
    explicit InstBase(isa::inst::Opcode op) : id_(RecieveId()), opcode_(op)
    {
        inputs_.resize(GetNumInputs());
    }

    static IdType RecieveId()
    {
        static IdType INST_ID_COUNTER = 0;
        return INST_ID_COUNTER++;
    }

    inline bool IsNotTypeSensitive() const
    {
        // FIXME:
        return true;
    }

    std::unique_ptr<InstBase> next_{ nullptr };
    InstBase* prev_{ nullptr };

    const IdType id_{};

    isa::inst::Opcode opcode_;
    DataType data_type_{ DataType::VOID };
    BasicBlock* bb_{ nullptr };

    std::list<User> users_{};
    std::vector<Input> inputs_{};
};

class Conditional
{
  public:
    enum Type : uint8_t
    {
        UNSET,
        EQ,
        NEQ,
        LEQ,
        GEQ,
        L,
        G
    };

    Conditional(Type cond) : cond_{ cond }
    {
    }
    DEFAULT_CTOR(Conditional);

    GETTER_SETTER(Cond, Type, cond_);

    Type Invert();

    void Dump() const;

  private:
    Type cond_{ Type::UNSET };
};

class WithImm : public InstBase
{
  public:
    WithImm(isa::inst::Opcode op) : InstBase(op)
    {
        imms_.resize(GetNumImms());
    }
    GETTER(Imms, imms_);

    ImmType GetImm(size_t idx) const;
    void SetImm(size_t idx, ImmType imm);

    void Dump() const override;

  protected:
    std::vector<ImmType> imms_{};
};

class isa::inst_type::NO_INPUT : public InstBase
{
    using Type = isa::inst_type::NO_INPUT;

  public:
    NO_INPUT(isa::inst::Opcode op) : InstBase(op)
    {
        using ValueDyn = isa::InputValue<Type, isa::input::Type::DYN>;
        using ValueVreg = isa::InputValue<Type, isa::input::Type::VREG>;
        using ValueImm = isa::InputValue<Type, isa::input::Type::IMM>;
        using ValueCond = isa::InputValue<Type, isa::input::Type::COND>;

        assert(ValueDyn::value == IsDynamic());
        assert(ValueVreg::value == GetNumInputs());
        assert(ValueImm::value == GetNumImms());
        assert(ValueCond::value == IsConditional());
    }

  private:
};

class isa::inst_type::UNARY : public InstBase
{
    using Type = isa::inst_type::UNARY;

  public:
    UNARY(isa::inst::Opcode op) : InstBase(op)
    {
        using ValueDyn = isa::InputValue<Type, isa::input::Type::DYN>;
        using ValueVreg = isa::InputValue<Type, isa::input::Type::VREG>;
        using ValueImm = isa::InputValue<Type, isa::input::Type::IMM>;
        using ValueCond = isa::InputValue<Type, isa::input::Type::COND>;

        assert(ValueDyn::value == IsDynamic());
        assert(ValueVreg::value == GetNumInputs());
        assert(ValueImm::value == GetNumImms());
        assert(ValueCond::value == IsConditional());
    }

  private:
};

class isa::inst_type::BINARY : public InstBase
{
    using Type = isa::inst_type::BINARY;

  public:
    BINARY(isa::inst::Opcode op) : InstBase(op)
    {
        using ValueDyn = isa::InputValue<Type, isa::input::Type::DYN>;
        using ValueVreg = isa::InputValue<Type, isa::input::Type::VREG>;
        using ValueImm = isa::InputValue<Type, isa::input::Type::IMM>;
        using ValueCond = isa::InputValue<Type, isa::input::Type::COND>;

        assert(ValueDyn::value == IsDynamic());
        assert(ValueVreg::value == GetNumInputs());
        assert(ValueImm::value == GetNumImms());
        assert(ValueCond::value == IsConditional());
    }

  private:
};

class isa::inst_type::BIN_IMM : public WithImm
{
    using Type = isa::inst_type::BIN_IMM;

  public:
    BIN_IMM(isa::inst::Opcode op) : WithImm(op)
    {
        using ValueDyn = isa::InputValue<Type, isa::input::Type::DYN>;
        using ValueVreg = isa::InputValue<Type, isa::input::Type::VREG>;
        using ValueImm = isa::InputValue<Type, isa::input::Type::IMM>;
        using ValueCond = isa::InputValue<Type, isa::input::Type::COND>;

        assert(ValueDyn::value == IsDynamic());
        assert(ValueVreg::value == GetNumInputs());
        assert(ValueImm::value == GetNumImms());
        assert(ValueCond::value == IsConditional());
    }

  private:
};

class isa::inst_type::PHI : public InstBase
{
    using Type = isa::inst_type::PHI;

  public:
    PHI(isa::inst::Opcode op) : InstBase(op)
    {
        using ValueDyn = isa::InputValue<Type, isa::input::Type::DYN>;
        using ValueVreg = isa::InputValue<Type, isa::input::Type::VREG>;
        using ValueImm = isa::InputValue<Type, isa::input::Type::IMM>;
        using ValueCond = isa::InputValue<Type, isa::input::Type::COND>;

        assert(ValueDyn::value == IsDynamic());
        assert(ValueVreg::value == GetNumInputs());
        assert(ValueImm::value == GetNumImms());
        assert(ValueCond::value == IsConditional());
    }

  private:
};

class isa::inst_type::CALL : public InstBase
{
    using Type = isa::inst_type::CALL;

  public:
    CALL(isa::inst::Opcode op, Graph* callee) : InstBase(op), callee_{ callee }
    {
        using ValueDyn = isa::InputValue<Type, isa::input::Type::DYN>;
        using ValueVreg = isa::InputValue<Type, isa::input::Type::VREG>;
        using ValueImm = isa::InputValue<Type, isa::input::Type::IMM>;
        using ValueCond = isa::InputValue<Type, isa::input::Type::COND>;

        assert(ValueDyn::value == IsDynamic());
        assert(ValueVreg::value == GetNumInputs());
        assert(ValueImm::value == GetNumImms());
        assert(ValueCond::value == IsConditional());
    }
    GETTER_SETTER(Callee, Graph*, callee_);

  private:
    Graph* callee_;
};

class isa::inst_type::IF : public InstBase, public Conditional
{
    using Type = isa::inst_type::IF;

  public:
    IF(isa::inst::Opcode op, Conditional::Type cc) : InstBase(op), Conditional(cc)
    {
        using ValueDyn = isa::InputValue<Type, isa::input::Type::DYN>;
        using ValueVreg = isa::InputValue<Type, isa::input::Type::VREG>;
        using ValueImm = isa::InputValue<Type, isa::input::Type::IMM>;
        using ValueCond = isa::InputValue<Type, isa::input::Type::COND>;

        assert(ValueDyn::value == IsDynamic());
        assert(ValueVreg::value == GetNumInputs());
        assert(ValueImm::value == GetNumImms());
        assert(ValueCond::value == IsConditional());
    }

    void Dump() const override
    {
        InstBase::Dump();
        Conditional::Dump();
    }

  private:
};

class isa::inst_type::IF_IMM : public WithImm, public Conditional
{
    using Type = isa::inst_type::IF_IMM;

  public:
    IF_IMM(isa::inst::Opcode op, Conditional::Type cc) : WithImm(op), Conditional(cc)
    {
        using ValueDyn = isa::InputValue<Type, isa::input::Type::DYN>;
        using ValueVreg = isa::InputValue<Type, isa::input::Type::VREG>;
        using ValueImm = isa::InputValue<Type, isa::input::Type::IMM>;
        using ValueCond = isa::InputValue<Type, isa::input::Type::COND>;

        assert(ValueDyn::value == IsDynamic());
        assert(ValueVreg::value == GetNumInputs());
        assert(ValueImm::value == GetNumImms());
        assert(ValueCond::value == IsConditional());
    }

    void Dump() const override
    {
        WithImm::Dump();
        Conditional::Dump();
    }

  private:
};

class isa::inst_type::COMPARE : public InstBase, public Conditional
{
    using Type = isa::inst_type::COMPARE;

  public:
    COMPARE(isa::inst::Opcode, Conditional::Type cc)
        : InstBase(isa::inst::Opcode::CMP), Conditional(cc)
    {
        using ValueDyn = isa::InputValue<Type, isa::input::Type::DYN>;
        using ValueVreg = isa::InputValue<Type, isa::input::Type::VREG>;
        using ValueImm = isa::InputValue<Type, isa::input::Type::IMM>;
        using ValueCond = isa::InputValue<Type, isa::input::Type::COND>;

        assert(ValueDyn::value == IsDynamic());
        assert(ValueVreg::value == GetNumInputs());
        assert(ValueImm::value == GetNumImms());
        assert(ValueCond::value == IsConditional());
    }

    void Dump() const override
    {
        InstBase::Dump();
        Conditional::Dump();
    }

  private:
};

class isa::inst_type::CONST : public InstBase
{
    using Type = isa::inst_type::CONST;

  public:
    template <typename T>
    CONST(isa::inst::Opcode, T val) : InstBase(isa::inst::Opcode::CONST)
    {
        using ValueDyn = isa::InputValue<Type, isa::input::Type::DYN>;
        using ValueVreg = isa::InputValue<Type, isa::input::Type::VREG>;
        using ValueImm = isa::InputValue<Type, isa::input::Type::IMM>;
        using ValueCond = isa::InputValue<Type, isa::input::Type::COND>;

        assert(ValueDyn::value == IsDynamic());
        assert(ValueVreg::value == GetNumInputs());
        assert(ValueImm::value == GetNumImms());
        assert(ValueCond::value == IsConditional());

        if constexpr (std::numeric_limits<T>::is_integer) {
            SetDataType(InstBase::DataType::INT);
            val_ = val;
        } else if constexpr (std::is_same_v<T, float>) {
            SetDataType(InstBase::DataType::FLOAT);
            val_ = std::bit_cast<uint32_t, float>(val);
        } else if constexpr (std::is_same_v<T, double>) {
            SetDataType(InstBase::DataType::DOUBLE);
            val_ = std::bit_cast<uint64_t, double>(val);
        } else {
            SetDataType(InstBase::DataType::VOID);
            assert(false);
        }
    }

    static bool Compare(const InstBase* i1, const InstBase* i2);

    bool IsZero();
    bool IsNull();

    uint64_t GetValRaw() const;
    int64_t GetValInt() const;
    double GetValDouble() const;
    float GetValFloat() const;

    void Dump() const override;

  private:
    uint64_t val_{ 0 };
};

template <isa::inst::Opcode OPCODE, typename... Args>
std::unique_ptr<InstBase> InstBase::NewInst(Args&&... args)
{
    using Type = typename isa::inst::Inst<OPCODE>::Type;
    return std::unique_ptr<Type>(new Type(OPCODE, std::forward<Args>(args)...));
}

#endif