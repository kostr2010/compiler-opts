#include "bb.h"
#include "graph.h"

bool BasicBlock::HasNoPredecessors() const
{
    return preds_.empty();
}

size_t BasicBlock::GetNumPredecessors() const
{
    return preds_.size();
}

BasicBlock* BasicBlock::GetPredecessor(size_t idx)
{
    ASSERT(idx < preds_.size());
    return preds_[idx];
}

bool BasicBlock::HasNoSuccessors() const
{
    for (size_t i = 0; i < GetNumSuccessors(); ++i) {
        if (succs_[i] != nullptr) {
            return false;
        }
    }
    return true;
}

size_t BasicBlock::GetNumSuccessors() const
{
    if (last_inst_ == nullptr) {
        return NumBranchesDefault::value;
    }
    return isa::EvaluatePredicate<GetNumBranches>(last_inst_->GetOpcode());
}

BasicBlock* BasicBlock::GetSuccessor(size_t idx)
{
    ASSERT(idx < GetNumSuccessors());
    return succs_[idx];
}

void BasicBlock::SetLoop(Loop* loop, bool is_header /* = false */)
{
    is_loop_header_ = is_header;
    loop_ = loop;
}

bool BasicBlock::IsLoopHeader() const
{
    return is_loop_header_;
}

InstBase* BasicBlock::GetFirstPhi() const
{
    return first_phi_.get();
}

InstBase* BasicBlock::GetFirstInst() const
{
    return first_inst_.get();
}

void BasicBlock::ClearImmDominator()
{
    imm_dominator_ = nullptr;
}

bool BasicBlock::Dominates(BasicBlock* bb) const
{
    ASSERT(bb != nullptr);

    if (bb->GetId() == GetId()) {
        return true;
    }

    auto dom = bb->GetImmDominator();
    while (dom != nullptr) {
        if (dom == this) {
            return true;
        }
        dom = dom->GetImmDominator();
    }
    return false;
}

void BasicBlock::SetSuccsessor(size_t pos, BasicBlock* bb)
{
    ASSERT(bb != nullptr);
    ASSERT(pos < GetNumSuccessors());

    succs_[pos] = bb;
}

bool BasicBlock::Precedes(IdType bb_id) const
{
    for (size_t i = 0; i < GetNumSuccessors(); ++i) {
        ASSERT(succs_[i] != nullptr);
        if (bb_id == succs_[i]->GetId()) {
            return true;
        }
    }
    return false;
}

bool BasicBlock::Precedes(BasicBlock* bb) const
{
    ASSERT(bb != nullptr);

    return Precedes(bb->GetId());
}

void BasicBlock::AddPredecessor(BasicBlock* bb)
{
    ASSERT(bb != nullptr);
    ASSERT(!Succeeds(bb));

    preds_.emplace_back(bb);
}

bool BasicBlock::Succeeds(IdType bb_id) const
{
    return std::find_if(preds_.begin(), preds_.end(),
                        [bb_id](BasicBlock* bb) { return bb->GetId() == bb_id; }) != preds_.end();
}

bool BasicBlock::Succeeds(BasicBlock* bb) const
{
    ASSERT(bb != nullptr);
    return std::find_if(preds_.begin(), preds_.end(),
                        [bb](BasicBlock* s) { return bb->GetId() == s->GetId(); }) != preds_.end();
}

void BasicBlock::RemovePredecessor(BasicBlock* bb)
{
    ASSERT(bb != nullptr);
    ASSERT(Succeeds(bb));

    std::erase_if(preds_, [bb](BasicBlock* p) { return bb->GetId() == p->GetId(); });
}

void BasicBlock::ReplaceSuccessor(BasicBlock* bb_old, BasicBlock* bb_new)
{
    ASSERT(bb_old != nullptr);

    for (size_t i = 0; i < GetNumSuccessors(); ++i) {
        ASSERT(succs_[i] != nullptr);

        if (succs_[i]->GetId() == bb_old->GetId()) {
            succs_[i] = bb_new;
            return;
        }
    }

    UNREACHABLE("Trying to replace unexisting successor.");
}

void BasicBlock::ReplacePredecessor(BasicBlock* bb_old, BasicBlock* bb_new)
{
    for (size_t i = 0; i < GetNumPredecessors(); ++i) {
        ASSERT(preds_[i] != nullptr);

        if (preds_[i]->GetId() == bb_old->GetId()) {
            preds_[i] = bb_new;
            return;
        }
    }

    UNREACHABLE("Trying to replace unexisting predecessor.");
}

void BasicBlock::PushBackInst(std::unique_ptr<InstBase> inst)
{
    ASSERT(inst != nullptr);
    ASSERT(!inst->IsPhi());

    inst->SetBasicBlock(this);
    if (last_inst_ == nullptr) {
        SetFirstInst(std::move(inst));
    } else {
        inst->SetPrev(last_inst_);
        last_inst_->SetNext(std::move(inst));
        last_inst_ = last_inst_->GetNext();
    }
}

void BasicBlock::PushFrontInst(std::unique_ptr<InstBase> inst)
{
    ASSERT(inst != nullptr);
    ASSERT(!inst->IsPhi());

    inst->SetBasicBlock(this);
    if (first_inst_ == nullptr) {
        SetFirstInst(std::move(inst));
    } else {
        first_inst_->SetPrev(inst.get());
        inst->SetNext(first_inst_.release());
        first_inst_ = std::move(inst);
    }
}

void BasicBlock::InsertInst(std::unique_ptr<InstBase> inst, InstBase* left, InstBase* right)
{
    ASSERT(right != nullptr);
    ASSERT(left != nullptr);
    ASSERT(inst != nullptr);
    ASSERT(!left->IsPhi());
    ASSERT(!right->IsPhi());
    ASSERT(!inst->IsPhi());
    ASSERT(left->GetNext()->GetId() == right->GetId());
    ASSERT(right->GetPrev()->GetId() == left->GetId());

    inst->SetBasicBlock(this);
    inst->SetNext(left->ReleaseNext());
    inst->SetPrev(left);
    right->SetPrev(inst.get());
    left->SetNext(std::move(inst));
}

void BasicBlock::InsertInstAfter(std::unique_ptr<InstBase> inst, InstBase* after)
{
    ASSERT(after != nullptr);
    auto next = after->GetNext();

    if (next == nullptr) {
        PushBackInst(std::move(inst));
    } else {
        InsertInst(std::move(inst), after, next);
    }
}

void BasicBlock::InsertInstBefore(std::unique_ptr<InstBase> inst, InstBase* before)
{
    ASSERT(before != nullptr);
    auto prev = before->GetPrev();

    if (prev == nullptr) {
        PushFrontInst(std::move(inst));
    } else {
        InsertInst(std::move(inst), prev, before);
    }
}

void BasicBlock::PushBackPhi(std::unique_ptr<InstBase> inst)
{
    ASSERT(inst != nullptr);
    ASSERT(inst->IsPhi());

    inst->SetBasicBlock(this);
    if (first_phi_ == nullptr) {
        first_phi_ = std::move(inst);
        last_phi_ = first_phi_.get();
    } else {
        inst->SetPrev(last_phi_);
        last_phi_->SetNext(std::move(inst));
        last_phi_ = last_phi_->GetNext();
    }
}

void BasicBlock::PushBackPhi(InstBase* inst)
{
    ASSERT(inst != nullptr);
    ASSERT(inst->IsPhi());

    inst->SetBasicBlock(this);
    if (first_phi_ == nullptr) {
        first_phi_.reset(inst);
        last_phi_ = first_phi_.get();
    } else {
        inst->SetPrev(last_phi_);
        last_phi_->SetNext(std::move(inst));
        last_phi_ = last_phi_->GetNext();
    }
}

void BasicBlock::UnlinkInst(InstBase* inst)
{
    ASSERT(inst != nullptr);

    auto prev = inst->GetPrev();
    auto next = inst->GetNext();

    if (prev != nullptr && next != nullptr) {
        ASSERT(prev != nullptr);
        ASSERT(next != nullptr);

        next->SetPrev(prev);
        prev->SetNext(inst->ReleaseNext());
    } else if (prev == nullptr && next != nullptr) {
        ASSERT(prev == nullptr);
        ASSERT(next != nullptr);

        if (inst->IsPhi()) {
            ASSERT(first_phi_ != nullptr);
            ASSERT(first_phi_->GetId() == inst->GetId());
            first_phi_.reset(inst->ReleaseNext());
        } else {
            ASSERT(first_inst_ != nullptr);
            ASSERT(first_inst_->GetId() == inst->GetId());
            first_inst_.reset(inst->ReleaseNext());
        }

        next->SetPrev(nullptr);
    } else if (prev != nullptr && next == nullptr) {
        ASSERT(prev != nullptr);
        ASSERT(next == nullptr);

        if (inst->IsPhi()) {
            ASSERT(last_phi_ != nullptr);
            ASSERT(last_phi_->GetId() == inst->GetId());
            last_phi_ = prev;
        } else {
            ASSERT(last_inst_ != nullptr);
            ASSERT(last_inst_->GetId() == inst->GetId());
            last_inst_ = prev;
        }

        prev->SetNext(nullptr);
    } else {
        ASSERT(prev == nullptr);
        ASSERT(next == nullptr);

        if (inst->IsPhi()) {
            first_phi_.reset();
            last_phi_ = nullptr;
        } else {
            first_inst_.reset();
            last_inst_ = nullptr;
        }
    }
}

InstBase* BasicBlock::TransferInst()
{
    last_inst_ = nullptr;
    return first_inst_.release();
}

InstBase* BasicBlock::TransferPhi()
{
    last_phi_ = nullptr;
    return first_phi_.release();
}

void BasicBlock::Dump() const
{
    std::cout << "#########################\n";
    std::cout << "# BB id: " << id_ << "\n";

    std::cout << "# PREDS: [";
    for (auto bb : GetPredecessors()) {
        std::cout << bb->GetId() << " ";
    }
    std::cout << "]\n";

    std::cout << "# SUCCS: [";
    for (auto bb : GetSuccessors()) {
        std::cout << bb->GetId() << " ";
    }
    std::cout << "]\n";

    if (IsEmpty()) {
        std::cout << "#########################\n";
        return;
    }

    std::cout << "# FIRST PHI: ";
    if (first_phi_ != nullptr) {
        std::cout << first_phi_->GetId() << "\n";
    } else {
        std::cout << "NULL\n";
    }

    std::cout << "# LAST PHI: ";
    if (last_phi_ != nullptr) {
        std::cout << last_phi_->GetId() << "\n";
    } else {
        std::cout << "NULL\n";
    }

    std::cout << "# PHI:\n";
    for (auto inst = first_phi_.get(); inst != nullptr; inst = inst->GetNext()) {
        inst->Dump();
        std::cout << "#\n";
    }

    std::cout << "# FIRST INST: ";
    if (first_inst_ != nullptr) {
        std::cout << first_inst_->GetId() << "\n";
    } else {
        std::cout << "NULL\n";
    }

    std::cout << "# LAST INST: ";
    if (last_inst_ != nullptr) {
        std::cout << last_inst_->GetId() << "\n";
    } else {
        std::cout << "NULL\n";
    }

    std::cout << "# INSTRUCTIONS:\n";
    for (auto inst = first_inst_.get(); inst != nullptr; inst = inst->GetNext()) {
        inst->Dump();
        std::cout << "#\n";
    }

    std::cout << "#########################\n";
}

void BasicBlock::SetFirstInst(std::unique_ptr<InstBase> inst)
{
    ASSERT(inst != nullptr);
    ASSERT(first_inst_ == nullptr);
    ASSERT(last_inst_ == nullptr);

    first_inst_ = std::move(inst);
    last_inst_ = first_inst_.get();
}

bool BasicBlock::IsEmpty() const
{
    return first_inst_.get() == nullptr && first_phi_.get() == nullptr && last_inst_ == nullptr &&
           last_phi_ == nullptr;
}

bool BasicBlock::IsStartBlock() const
{
    return HasNoPredecessors();
}

bool BasicBlock::IsEndBlock() const
{
    return HasNoSuccessors();
}
