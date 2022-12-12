#include "inst.h"
#include "bb.h"

void Inst::SetInput(size_t idx, Inst* inst)
{
    assert(inst != nullptr);
    assert(idx < inputs_.size());
    assert(!IsPhi());

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

    std::cout << "\tinst_id:....." << id_ << "\n";
    std::cout << "\tinst_opcode:." << op_to_str.at(opcode_) << " (" << (int)opcode_ << ")\n";
    std::cout << "\tinst_type:..." << (int)inst_type_ << "\n";
    std::cout << "\tinst users:\n\t\t[";

    for (auto user : users_) {
        std::cout << user.GetInst()->GetId() << "(" << user.GetIdx() << ") ";
    }
    std::cout << "]\n";

    std::cout << "\tinst inputs:\n\t\t[";
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

size_t PhiOp::AddInput(Inst* inst, BasicBlock* bb)
{
    assert(inst != nullptr);
    assert(bb != nullptr);
    assert(inputs_.size() + 1 < MAX_INPUTS);
    assert(IsPhi());

    size_t idx = inputs_.size();

    inst->AddUser(this);
    inputs_.emplace_back(Input(inst, bb));

    return idx;
}

size_t PhiOp::AddInput(const Input& input)
{
    assert(input.GetInst() != nullptr);
    assert(input.GetSourceBB() != nullptr);
    assert(inputs_.size() + 1 < MAX_INPUTS);
    assert(IsPhi());

    size_t idx = inputs_.size();

    input.GetInst()->AddUser(this);
    inputs_.push_back(input);

    return idx;
}

void PhiOp::RemoveInput(const Input& input)
{
    input.GetInst()->RemoveUser(User(this));
    inputs_.erase(std::find_if(inputs_.begin(), inputs_.end(), [input](const Input& i) {
        return (i.GetSourceBB()->GetId() == input.GetSourceBB()->GetId());
    }));
}

Input PhiOp::GetInput(const BasicBlock* bb)
{
    auto it = std::find_if(inputs_.begin(), inputs_.end(), [bb](const Input& i) {
        return (i.GetSourceBB()->GetId() == bb->GetId());
    });

    assert(it != inputs_.end());

    return *it;
}