#include "inst.h"
#include "bb.h"

void Inst::SetInput(size_t idx, Inst* inst)
{
    assert(inst != nullptr);
    assert(idx < inputs_.size());

    inst->users_.push_back(User(this, idx));
    inputs_[idx] = Input(inst, inst->GetBasicBlock());
}

void Inst::Dump() const
{
    static const std::unordered_map<Opcode, std::string> op_to_str{
#define CREATE(OPCODE, ...) { Opcode::OPCODE, #OPCODE },
        INSTRUCTION_LIST(CREATE)
#undef CREATE
    };

    std::cout << "#\tinst_id:....." << id_ << "\n";
    std::cout << "#\tbb_id:.......";
    if (bb_ == nullptr) {
        std::cout << "NULL\n";
    } else {
        std::cout << bb_->GetId() << "\n";
    }
    std::cout << "#\tinst_opcode:." << op_to_str.at(opcode_) << " (" << (int)opcode_ << ")\n";
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

size_t Inst::AddInput(Inst* inst, BasicBlock* bb)
{
    assert(inst != nullptr);
    assert(bb != nullptr);
    assert(HasDynamicOperands());
    assert(inputs_.size() + 1 < MAX_INPUTS);

    size_t idx = inputs_.size();

    inst->AddUser(this);
    inputs_.emplace_back(Input(inst, bb));

    return idx;
}

size_t Inst::AddInput(const Input& input)
{
    assert(input.GetInst() != nullptr);
    assert(input.GetSourceBB() != nullptr);
    assert(HasDynamicOperands());
    assert(inputs_.size() + 1 < MAX_INPUTS);

    size_t idx = inputs_.size();

    input.GetInst()->AddUser(this);
    inputs_.push_back(input);

    return idx;
}

void Inst::ReserveInputs(size_t n)
{
    inputs_.reserve(n);
}

void Inst::ClearInputs()
{
    inputs_.clear();
}

void Inst::RemoveInput(const Input& input)
{
    std::erase_if(inputs_, [input](const Input& i) {
        return (i.GetSourceBB()->GetId() == input.GetSourceBB()->GetId() &&
                i.GetInst()->GetId() == input.GetInst()->GetId());
    });
}

void Inst::ReplaceInput(Inst* old_inst, Inst* new_inst)
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

void Inst::ClearInput(Inst* old_inst)
{
    assert(old_inst != nullptr);

    auto it = std::find_if(inputs_.begin(), inputs_.end(), [old_inst](const Input& i) {
        return (i.GetInst() != nullptr) && (old_inst->GetId() == i.GetInst()->GetId());
    });

    if (it != inputs_.end()) {
        it->SetInst(nullptr);
        it->SetSourceBB(nullptr);
    }
}

bool Inst::HasFlag(InstFlags flag) const
{
    constexpr std::array<uint8_t, Opcode::N_OPCODES> FLAGS_MAP{
#define GET_FLAGS(OP, TYPE, FLAGS, ...) FLAGS,
        INSTRUCTION_LIST(GET_FLAGS)
#undef GET_FLAGS
    };
    return FLAGS_MAP[opcode_] & flag;
}

bool Inst::HasDynamicOperands() const
{
    constexpr std::array<uint8_t, Opcode::N_OPCODES> DYN_OPS{
#define GET_FLAGS(OP, TYPE, FLAGS, ...) has_dynamic_operands<Opcode::OP>(),
        INSTRUCTION_LIST(GET_FLAGS)
#undef GET_FLAGS
    };
    return DYN_OPS[opcode_];
}

size_t Inst::GetNumInputs() const
{
    constexpr std::array<size_t, Opcode::N_OPCODES> NUM_INPUTS{
#define GET_FLAGS(OP, TYPE, FLAGS, ...) get_num_inputs<TYPE>(),
        INSTRUCTION_LIST(GET_FLAGS)
#undef GET_FLAGS
    };
    return NUM_INPUTS[opcode_];
}

void Inst::AddUser(Inst* inst)
{
    assert(inst->HasDynamicOperands());
    users_.emplace_back(inst);
}

void Inst::AddUser(Inst* inst, size_t idx)
{
    assert(!inst->HasDynamicOperands());
    users_.emplace_back(inst, idx);
}

void Inst::AddUser(const User& user)
{
    users_.push_back(user);
}

void Inst::RemoveUser(const User& user)
{
    std::erase_if(
        users_, [user](const User& u) { return u.GetInst()->GetId() == user.GetInst()->GetId(); });
}

void Inst::RemoveUser(Inst* user)
{
    std::erase_if(users_, [user](const User& u) { return u.GetInst()->GetId() == user->GetId(); });
}

void Inst::ReplaceUser(const User& user_old, const User& user_new)
{
    std::replace_if(
        users_.begin(), users_.end(),
        [user_old](const User& u) { return user_old.GetInst()->GetId() == u.GetInst()->GetId(); },
        user_new);
}