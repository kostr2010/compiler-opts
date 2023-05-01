#include <array>
#include <sstream>

#include "bb.h"
#include "inst.h"

// ====================
// InstBase

Input InstBase::GetInput(size_t idx) const
{
    assert(idx < GetNumInputs());

    return inputs_[idx];
}

void InstBase::SetInput(size_t idx, InstBase* inst)
{
    assert(inst != nullptr);
    assert(idx < GetNumInputs());

    inst->users_.push_back(User(this, idx));
    inputs_[idx] = Input(inst, inst->GetBasicBlock());
}

void InstBase::Dump() const
{
    static const std::array<std::string, isa::inst::Opcode::N_OPCODES> OP_TO_STR{
#define CREATE(OPCODE, ...) #OPCODE,
        ISA_INSTRUCTION_LIST(CREATE)
#undef CREATE
    };

    std::cout << "#\tinst_id:....." << id_ << "\n";
    std::cout << "#\tbb_id:.......";
    if (bb_ == nullptr) {
        std::cout << "NULL\n";
    } else {
        std::cout << bb_->GetId() << "\n";
    }
    std::cout << "#\tinst_opcode:." << OP_TO_STR[opcode_] << " (" << static_cast<int>(opcode_)
              << ")\n";
    std::cout << "#\tinst prev:...";
    if (prev_ == nullptr) {
        std::cout << "NULL\n";
    } else {
        std::cout << prev_->GetId() << "\n";
    }
    std::cout << "#\tinst next:...";
    if (next_ == nullptr) {
        std::cout << "NULL\n";
    } else {
        std::cout << next_->GetId() << "\n";
    }
    std::cout << "#\tinst users:\n#\t\t[";

    for (auto user : users_) {
        std::cout << user.GetInst()->GetId() << "(" << user.GetIdx() << ") ";
    }
    std::cout << "]\n";

    std::cout << "#\tinst inputs:\n#\t\t[";
    if (!IsPhi()) {
        for (auto input : inputs_) {
            std::cout << input.GetInst()->GetId() << " ";
        }
    } else {
        for (auto input : inputs_) {
            std::cout << input.GetInst()->GetId() << "(bb: " << input.GetSourceBB()->GetId() << ")"
                      << " ";
        }
    }
    std::cout << "]\n";
}

void InstBase::AddInput(InstBase* inst, BasicBlock* bb)
{
    assert(inst != nullptr);
    assert(bb != nullptr);
    assert(IsDynamic());

    inst->AddUser(this);
    inputs_.emplace_back(Input(inst, bb));
}

void InstBase::AddInput(const Input& input)
{
    assert(input.GetInst() != nullptr);
    assert(input.GetSourceBB() != nullptr);
    assert(IsDynamic());

    input.GetInst()->AddUser(this);
    inputs_.push_back(input);
}

void InstBase::ClearInputs()
{
    assert(IsDynamic());

    inputs_.clear();
}

void InstBase::RemoveInput(const Input& input)
{
    assert(IsDynamic());

    std::erase_if(inputs_, [input](const Input& i) {
        return (i.GetSourceBB()->GetId() == input.GetSourceBB()->GetId() &&
                i.GetInst()->GetId() == input.GetInst()->GetId());
    });
}

void InstBase::ReplaceInput(InstBase* old_inst, InstBase* new_inst)
{
    assert(old_inst != nullptr);
    assert(new_inst != nullptr);

    for (auto& input : inputs_) {
        assert(input.GetInst() != nullptr);
        if (old_inst->GetId() == input.GetInst()->GetId()) {
            if (!IsPhi()) {
                input.SetSourceBB(new_inst->GetBasicBlock());
            }
            input.SetInst(new_inst);
        }
    }
}

template <isa::inst::Opcode OP>
using VregValue = isa::InputValue<typename isa::inst::Inst<OP>::Type, isa::input::Type::VREG>;

size_t InstBase::GetNumInputs() const
{
    auto is_dynamic = IsDynamic();
    auto isa_num_inputs = isa::EvaluatePredicate<VregValue>(opcode_);
    return inputs_.size() * is_dynamic + isa_num_inputs * !is_dynamic;
}

template <isa::inst::Opcode OP>
using ImmValue = isa::InputValue<typename isa::inst::Inst<OP>::Type, isa::input::Type::IMM>;

size_t InstBase::GetNumImms() const
{
    return isa::EvaluatePredicate<ImmValue>(opcode_);
}

size_t InstBase::GetNumUsers() const
{
    return users_.size();
}

void InstBase::AddUser(InstBase* inst)
{
    assert(inst->IsDynamic());
    users_.emplace_back(inst);
}

void InstBase::AddUser(InstBase* inst, size_t idx)
{
    assert(!inst->IsDynamic());
    users_.emplace_back(inst, idx);
}

void InstBase::AddUser(const User& user)
{
    users_.push_back(user);
}

void InstBase::RemoveUser(const User& user)
{
    std::erase_if(
        users_, [user](const User& u) { return u.GetInst()->GetId() == user.GetInst()->GetId(); });
}

void InstBase::RemoveUser(InstBase* user)
{
    std::erase_if(users_, [user](const User& u) { return u.GetInst()->GetId() == user->GetId(); });
}

void InstBase::ReplaceUser(const User& user_old, const User& user_new)
{
    std::replace_if(
        users_.begin(), users_.end(),
        [user_old](const User& u) { return user_old.GetInst()->GetId() == u.GetInst()->GetId(); },
        user_new);
}

InstBase* InstBase::GetNext() const
{
    return next_.get();
}

void InstBase::SetNext(std::unique_ptr<InstBase> next)
{
    next_ = std::move(next);
}

void InstBase::SetNext(InstBase* next)
{
    next_.reset(next);
}

InstBase* InstBase::ReleaseNext()
{
    return next_.release();
}

bool InstBase::IsPhi() const
{
    return opcode_ == isa::inst::Opcode::PHI;
}

bool InstBase::IsConst() const
{
    return opcode_ == isa::inst::Opcode::CONST;
}

bool InstBase::IsParam() const
{
    return opcode_ == isa::inst::Opcode::PARAM;
}

bool InstBase::IsCall() const
{
    return HasFlag<isa::flag::Type::CALL>();
}

bool InstBase::IsCheck() const
{
    return HasFlag<isa::flag::Type::CHECK>();
}

template <isa::inst::Opcode OP>
using NumBranches =
    isa::FlagValueOr<OP, isa::flag::Type::BRANCH,
                     isa::flag::Flag<isa::flag::Type::BRANCH>::Value::NO_SUCCESSORS>;

bool InstBase::IsReturn() const
{
    return HasFlag<isa::flag::Type::BRANCH>() &&
           isa::EvaluatePredicate<NumBranches>(opcode_) ==
               isa::flag::Flag<isa::flag::BRANCH>::Value::NO_SUCCESSORS;
}

bool InstBase::IsUnconditionalJump() const
{
    return HasFlag<isa::flag::Type::BRANCH>() &&
           isa::EvaluatePredicate<NumBranches>(opcode_) ==
               isa::flag::Flag<isa::flag::BRANCH>::Value::ONE_SUCCESSOR;
}

template <isa::inst::Opcode OP>
using DynValue = isa::InputValue<typename isa::inst::Inst<OP>::Type, isa::input::Type::DYN>;

bool InstBase::IsDynamic() const
{
    return isa::EvaluatePredicate<DynValue>(opcode_);
}

template <isa::inst::Opcode OP>
using CondFlag = isa::InputValue<typename isa::inst::Inst<OP>::Type, isa::input::Type::COND>;

bool InstBase::IsConditional() const
{
    return isa::EvaluatePredicate<CondFlag>(opcode_);
}

bool InstBase::Precedes(const InstBase* inst) const
{
    assert(inst != nullptr);
    assert(GetBasicBlock()->GetId() == inst->GetBasicBlock()->GetId());

    if (this->GetId() == inst->GetId()) {
        return true;
    }

    auto next = GetNext();
    while (next != nullptr) {
        if (next->GetId() == inst->GetId()) {
            return true;
        }
        next = next->GetNext();
    }

    return false;
}

bool InstBase::Dominates(const InstBase* inst) const
{
    assert(inst != nullptr);

    if (this->GetId() == inst->GetId()) {
        return true;
    }

    auto bb = GetBasicBlock();
    auto bb_inst = inst->GetBasicBlock();
    auto in_same_bb = bb->GetId() == bb_inst->GetId();

    if (in_same_bb) {
        return Precedes(inst);
    }

    return bb->Dominates(bb_inst);
}

// InstBase
// ====================

// ====================
// Conditional

void Conditional::Invert()
{
    switch (cond_) {
    case Type::EQ: {
        cond_ = Type::NEQ;
        return;
    }
    case Type::NEQ: {
        cond_ = EQ;
        return;
    }
    case Type::LEQ: {
        cond_ = Type::G;
        return;
    }
    case Type::GEQ: {
        cond_ = Type::L;
        return;
    }
    case Type::L: {
        cond_ = Type::GEQ;
        return;
    }
    case Type::G: {
        cond_ = Type::LEQ;
        return;
    }
    default: {
        UNREACHABLE("unhandled conditional code");
    }
    }
}

void Conditional::Dump() const
{
    std::stringstream ss;
    ss << "#\tcondition type: ";

    switch (cond_) {
    case Type::EQ: {
        ss << "==";
        break;
    }
    case Type::NEQ: {
        ss << "!=";
        break;
    }
    case Type::LEQ: {
        ss << "<=";
        break;
    }
    case Type::GEQ: {
        ss << ">=";
        break;
    }
    case Type::L: {
        ss << "<";
        break;
    }
    case Type::G: {
        ss << ">";
        break;
    }
    default: {
        UNREACHABLE("unhandled conditional code");
    }
    }

    std::cout << ss.str() << "\n";
}

// Conditional
// ====================

// ====================
// WithImm

// WithImm
// ====================

// ====================
// NO_INPUT

// NO_INPUT
// ====================

// ====================
// UNARY

// UNARY
// ====================

// ====================
// BINARY

// BINARY
// ====================

// ====================
// BIN_IMM

// BIN_IMM
// ====================

// ====================
// PHI

// PHI
// ====================

// ====================
// CALL

// CALL
// ====================

// ====================
// IF

// IF
// ====================

// ====================
// IF_IMM

// IF_IMM
// ====================

// ====================
// COMPARE

// COMPARE
// ====================

// ====================
// CONST

bool isa::inst_type::CONST::Compare(const InstBase* i1, const InstBase* i2)
{
    assert(i1->IsConst());
    assert(i2->IsConst());
    using T = isa::inst::Inst<isa::inst::Opcode::CONST>::Type;

    return (static_cast<const T*>(i1)->GetDataType() ==
            static_cast<const T*>(i2)->GetDataType()) &&
           (static_cast<const T*>(i1)->GetValRaw() == static_cast<const T*>(i2)->GetValRaw());
}

bool isa::inst_type::CONST::IsZero()
{
    return GetValRaw() == 0;
}

bool isa::inst_type::CONST::IsNull()
{
    return GetValRaw() == 0;
}

uint64_t isa::inst_type::CONST::GetValRaw() const
{
    return val_;
}

int64_t isa::inst_type::CONST::GetValInt() const
{
    assert(GetDataType() == InstBase::DataType::INT);
    return static_cast<int64_t>(val_);
}

double isa::inst_type::CONST::GetValDouble() const
{
    assert(GetDataType() == InstBase::DataType::DOUBLE);
    return std::bit_cast<float, uint32_t>(static_cast<uint32_t>(val_));
}

float isa::inst_type::CONST::GetValFloat() const
{
    assert(GetDataType() == InstBase::DataType::FLOAT);
    return std::bit_cast<double, uint64_t>(val_);
}

void isa::inst_type::CONST::Dump() const
{
    InstBase::Dump();
    std::cout << "#\tconst value: " << GetValRaw() << "\n";
}

// CONST
// ====================
