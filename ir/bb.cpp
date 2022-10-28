#include "bb.h"

void BasicBlock::PushBackInst(std::unique_ptr<Inst> inst)
{
    assert(inst != nullptr);

    inst->SetBasicBlock(this);
    if (first_inst_ == nullptr) {
        SetFirstInst(std::move(inst));
    } else {
        inst->SetNext(std::unique_ptr<Inst>(nullptr));
        inst->SetPrev(last_inst_);
        last_inst_ = inst.get();
        last_inst_->SetNext(std::move(inst));
    }
}

void BasicBlock::PushFrontInst(std::unique_ptr<Inst> inst)
{
    assert(inst != nullptr);

    inst->SetBasicBlock(this);
    if (first_inst_ == nullptr) {
        SetFirstInst(std::move(inst));
    } else {
        inst->SetPrev(first_inst_->GetPrev());
        first_inst_->SetPrev(inst.get());
        inst->SetNext(std::move(first_inst_));
        first_inst_ = std::move(inst);
    }
}

void BasicBlock::PushBackPhi(std::unique_ptr<Inst> inst)
{
    assert(inst != nullptr);
    assert(inst->IsPhi());

    inst->SetBasicBlock(this);
    if (first_phi_ == nullptr) {
        inst->SetNext(std::unique_ptr<Inst>(nullptr));
        inst->SetPrev(nullptr);
        first_phi_ = std::move(inst);
    } else {
        inst->SetPrev(first_phi_.get());
        inst->SetNext(std::unique_ptr<Inst>(nullptr));
        first_phi_->SetNext(std::move(inst));
    }
}

void BasicBlock::Dump() const
{
    std::cout << "#########################\n";
    std::cout << "# BB id: " << id_ << "\n";

    std::cout << "# PREDS: [";
    for (auto bb : preds_) {
        std::cout << bb->GetId() << " ";
    }
    std::cout << "]\n";

    std::cout << "# SUCCS: [";
    for (auto bb : succs_) {
        std::cout << bb->GetId() << " ";
    }
    std::cout << "]\n";

    std::cout << "# PHI:\n";
    for (auto inst = first_phi_.get(); inst != nullptr; inst = inst->GetNext()) {
        inst->Dump();
        std::cout << "\n";
    }

    std::cout << "# INSTRUCTIONS:\n";
    for (auto inst = first_inst_.get(); inst != nullptr; inst = inst->GetNext()) {
        inst->Dump();
        std::cout << "\n";
    }

    std::cout << "#########################\n";
}
