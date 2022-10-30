#include "bb.h"
#include "graph.h"

void BasicBlock::AddSucc(BasicBlock* bb)
{
    assert(bb != nullptr);

    if (IsInSucc(bb)) {
        return;
    }

    succs_.push_back(bb);
}

bool BasicBlock::IsInSucc(IdType bb_id) const
{
    return std::find_if(succs_.begin(), succs_.end(),
                        [bb_id](BasicBlock* bb) { return bb->GetId() == bb_id; }) != succs_.end();
}

bool BasicBlock::IsInSucc(BasicBlock* bb) const
{
    assert(bb != nullptr);
    return std::find_if(succs_.begin(), succs_.end(),
                        [bb](BasicBlock* s) { return bb->GetId() == s->GetId(); }) != succs_.end();
}

void BasicBlock::RemoveSucc(BasicBlock* bb)
{
    assert(bb != nullptr);

    if (!IsInSucc(bb)) {
        return;
    }

    succs_.erase(std::find_if(succs_.begin(), succs_.end(),
                              [bb](BasicBlock* s) { return bb->GetId() == s->GetId(); }));
}

void BasicBlock::AddPred(BasicBlock* bb)
{
    assert(bb != nullptr);

    if (IsInPred(bb)) {
        return;
    }

    preds_.push_back(bb);
}

bool BasicBlock::IsInPred(IdType bb_id) const
{
    return std::find_if(preds_.begin(), preds_.end(),
                        [bb_id](BasicBlock* bb) { return bb->GetId() == bb_id; }) != preds_.end();
}

bool BasicBlock::IsInPred(BasicBlock* bb) const
{
    assert(bb != nullptr);
    return std::find_if(preds_.begin(), preds_.end(),
                        [bb](BasicBlock* s) { return bb->GetId() == s->GetId(); }) != preds_.end();
}

void BasicBlock::RemovePred(BasicBlock* bb)
{
    assert(bb != nullptr);

    if (!IsInPred(bb)) {
        return;
    }

    preds_.erase(std::find_if(preds_.begin(), preds_.end(),
                              [bb](BasicBlock* p) { return bb->GetId() == p->GetId(); }));
}

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

void BasicBlock::SetFirstInst(std::unique_ptr<Inst> inst)
{
    assert(inst != nullptr);
    assert(first_inst_ == nullptr);
    assert(last_inst_ == nullptr);

    inst->SetNext(std::unique_ptr<Inst>(nullptr));
    inst->SetPrev(nullptr);

    first_inst_ = std::move(inst);
    last_inst_ = inst.get();
}

bool BasicBlock::IsStartBlock() const
{
    return id_ == Graph::BB_START_ID;
}

bool BasicBlock::IsEndBlock() const
{
    return succs_.empty();
}