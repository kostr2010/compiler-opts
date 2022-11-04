#include "inst.h"
#include "bb.h"

void Inst::SetInput(size_t idx, Inst* inst)
{
    assert(inst != nullptr);
    assert(idx < inputs_.size());
    assert(!IsPhi());

    inst->users_.emplace_back(User(this, idx));
    inputs_[idx] = Input(inst);
}

size_t Inst::AddInput(Inst* inst)
{
    assert(inst != nullptr);
    assert(inputs_.size() + 1 < MAX_INPUTS);
    assert(IsPhi());

    size_t idx = inputs_.size();

    inst->users_.emplace_back(User(this, idx));
    inputs_.emplace_back(Input(inst));

    return idx;
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
        std::cout << user.GetInst()->GetId() << " ";
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
