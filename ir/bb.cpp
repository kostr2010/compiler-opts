#include "bb.h"

void BasicBlock::PushBackInst(Inst* inst)
{
    assert(inst != nullptr);

    inst->SetBasicBlock(this);
    if (first_inst_ == nullptr) {
        SetFirstInst(inst);
    } else {
        inst->SetNext(nullptr);
        inst->SetPrev(last_inst_);
        last_inst_->SetNext(inst);
        last_inst_ = inst;
    }
}

void BasicBlock::PushFrontInst(Inst* inst)
{
    assert(inst != nullptr);

    inst->SetBasicBlock(this);
    if (first_inst_ == nullptr) {
        SetFirstInst(inst);
    } else {
        inst->SetNext(first_inst_);
        inst->SetPrev(first_inst_->GetPrev());
        first_inst_->SetPrev(inst);
        if (first_phi_ != nullptr) {
            first_phi_->SetNext(inst);
        }
        first_inst_ = inst;
    }
}

void BasicBlock::PushBackPhi(Inst* inst)
{
    assert(inst != nullptr);
    assert(inst->IsPhi());

    inst->SetBasicBlock(this);
    if (first_phi_ == nullptr) {
        // all phi's go in front of actual instructions
        inst->SetNext(first_inst_);
        if (first_inst_ != nullptr) {
            first_inst_->SetPrev(inst);
        }
    } else {
        if (first_inst_ != nullptr) {
            auto prev = first_inst_->GetPrev();
            assert(prev != nullptr);
            assert(prev->IsPhi());
            assert(prev == first_phi_);

            inst->SetPrev(prev);
            inst->SetNext(first_inst_);
            prev->SetNext(inst);
            first_inst_->SetPrev(inst);
        } else {
            inst->SetPrev(first_phi_);
            first_phi_->SetNext(inst);
        }
    }
    first_phi_ = inst;
}

void BasicBlock::Dump() const
{
    std::cout << "#########################\n";
    std::cout << "# BB id: " << id_ << "\n";

    std::cout << "# PREDS: ";
    for (auto bb : preds_) {
        std::cout << bb->GetId() << " ";
    }
    std::cout << "\n";

    std::cout << "# SUCCS: ";
    for (auto bb : succs_) {
        std::cout << bb->GetId() << " ";
    }
    std::cout << "\n";

    std::cout << "# PHI:\n";
    for (auto inst = first_phi_; inst != nullptr; inst = inst->GetPrev()) {
        inst->Dump();
        std::cout << "\n";
    }

    std::cout << "# INSTRUCTIONS:\n";
    for (auto inst = first_inst_; inst != nullptr; inst = inst->GetNext()) {
        inst->Dump();
        std::cout << "\n";
    }

    std::cout << "#########################\n";
}
