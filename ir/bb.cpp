#include "bb.h"
#include "graph.h"

void BasicBlock::AddSucc(BasicBlock* bb)
{
    assert(bb != nullptr);

    if (IsInSucc(bb)) {
        return;
    }

    succs_.emplace_back(bb);
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

    std::erase_if(succs_, [bb](BasicBlock* s) { return bb->GetId() == s->GetId(); });
}

void BasicBlock::AddPred(BasicBlock* bb)
{
    assert(bb != nullptr);

    if (IsInPred(bb)) {
        return;
    }

    preds_.emplace_back(bb);
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

    std::erase_if(preds_, [bb](BasicBlock* p) { return bb->GetId() == p->GetId(); });
}

void BasicBlock::ReplaceSucc(BasicBlock* bb_old, BasicBlock* bb_new)
{
    std::replace_if(
        succs_.begin(), succs_.end(),
        [bb_old](BasicBlock* bb) { return bb->GetId() == bb_old->GetId(); }, bb_new);
}

void BasicBlock::ReplacePred(BasicBlock* bb_old, BasicBlock* bb_new)
{
    std::replace_if(
        preds_.begin(), preds_.end(),
        [bb_old](BasicBlock* bb) { return bb->GetId() == bb_old->GetId(); }, bb_new);
}

void BasicBlock::PushBackInst(std::unique_ptr<Inst> inst)
{
    assert(inst != nullptr);
    assert(!inst->IsPhi());

    inst->SetBasicBlock(this);
    if (first_inst_ == nullptr) {
        SetFirstInst(std::move(inst));
    } else {
        inst->SetPrev(last_inst_);
        last_inst_->SetNext(std::move(inst));
        last_inst_ = last_inst_->GetNext();
    }

    ++num_instructions_;
}

void BasicBlock::PushFrontInst(std::unique_ptr<Inst> inst)
{
    assert(inst != nullptr);
    assert(!inst->IsPhi());

    inst->SetBasicBlock(this);
    if (first_inst_ == nullptr) {
        SetFirstInst(std::move(inst));
    } else {
        first_inst_->SetPrev(inst.get());
        inst->SetNext(first_inst_.release());
        first_inst_ = std::move(inst);
    }

    ++num_instructions_;
}

void BasicBlock::InsertInst(std::unique_ptr<Inst> inst, Inst* left, Inst* right)
{
    assert(right != nullptr);
    assert(left != nullptr);
    assert(inst != nullptr);
    assert(!left->IsPhi());
    assert(!right->IsPhi());
    assert(!inst->IsPhi());
    assert(left->GetNext()->GetId() == right->GetId());
    assert(right->GetPrev()->GetId() == left->GetId());

    inst->SetBasicBlock(this);
    inst->SetNext(left->ReleaseNext());
    inst->SetPrev(left);
    right->SetPrev(inst.get());
    left->SetNext(std::move(inst));

    ++num_instructions_;
}

void BasicBlock::InsertInstAfter(std::unique_ptr<Inst> inst, Inst* after)
{
    assert(after != nullptr);
    auto next = after->GetNext();

    if (next == nullptr) {
        PushBackInst(std::move(inst));
    } else {
        InsertInst(std::move(inst), after, next);
    }
}

void BasicBlock::InsertInstBefore(std::unique_ptr<Inst> inst, Inst* before)
{
    assert(before != nullptr);
    auto prev = before->GetPrev();

    if (prev == nullptr) {
        PushFrontInst(std::move(inst));
    } else {
        InsertInst(std::move(inst), prev, before);
    }
}

void BasicBlock::PushBackPhi(std::unique_ptr<Inst> inst)
{
    assert(inst != nullptr);
    assert(inst->IsPhi());

    inst->SetBasicBlock(this);
    if (first_phi_ == nullptr) {
        first_phi_ = std::move(inst);
        last_phi_ = first_phi_.get();
    } else {
        inst->SetPrev(last_phi_);
        last_phi_->SetNext(std::move(inst));
        last_phi_ = last_phi_->GetNext();
    }

    ++num_instructions_;
}

void BasicBlock::PushBackPhi(Inst* inst)
{
    assert(inst != nullptr);
    assert(inst->IsPhi());

    inst->SetBasicBlock(this);
    if (first_phi_ == nullptr) {
        first_phi_.reset(inst);
        last_phi_ = first_phi_.get();
    } else {
        inst->SetPrev(last_phi_);
        last_phi_->SetNext(std::move(inst));
        last_phi_ = last_phi_->GetNext();
    }

    ++num_instructions_;
}

void BasicBlock::UnlinkInst(Inst* inst)
{
    assert(inst != nullptr);

    auto prev = inst->GetPrev();
    auto next = inst->GetNext();

    if (prev != nullptr && next != nullptr) {
        next->SetPrev(prev);
        prev->SetNext(inst->ReleaseNext());
    } else {
        if (prev == nullptr) {
            if (next != nullptr) {
                next->SetPrev(nullptr);
            }
            if (inst->IsPhi()) {
                first_phi_.reset(inst->ReleaseNext());
            } else {
                first_inst_.reset(inst->ReleaseNext());
            }
        }

        if (next == nullptr) {
            if (prev != nullptr) {
                prev->SetNext(nullptr);
            }

            if (inst->IsPhi()) {
                last_phi_ = prev;
            } else {
                last_inst_ = prev;
            }
        }
    }

    --num_instructions_;
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

    first_inst_ = std::move(inst);
    last_inst_ = first_inst_.get();
}

bool BasicBlock::IsStartBlock() const
{
    return id_ == Graph::BB_START_ID;
}

bool BasicBlock::IsEndBlock() const
{
    return succs_.empty();
}
