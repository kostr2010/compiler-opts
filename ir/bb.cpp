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
    assert(idx < preds_.size());
    return preds_[idx];
}

bool BasicBlock::HasNoSuccessors() const
{
    return succs_.empty();
}

size_t BasicBlock::GetNumSuccessors() const
{
    if (last_inst_ == nullptr) {
        return 0;
    }
    return isa::EvaluatePredicate<GetNumBranches>(last_inst_->GetOpcode());
}

BasicBlock* BasicBlock::GetSuccessor(size_t idx)
{
    assert(idx < succs_.size());
    return succs_[idx];
}

void BasicBlock::SetLoop(Loop* loop, bool is_header /* = false */)
{
    assert(loop != nullptr);

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
    assert(bb != nullptr);

    auto dom = bb->GetImmDominator();
    while (dom != nullptr) {
        if (dom == this) {
            return true;
        }
        dom = dom->GetImmDominator();
    }
    return false;
}

void BasicBlock::SetSucc(BasicBlock* bb, size_t pos /* = 0 */)
{
    assert(bb != nullptr);

    if (IsInSucc(bb)) {
        return;
    }

    succs_[pos] = bb;
}

void BasicBlock::AddSucc(BasicBlock* bb)
{
    assert(bb != nullptr);

    if (IsInSucc(bb)) {
        return;
    }

    for (size_t i = 0; i < GetNumSuccessors(); ++i) {
        if (succs_[i] == nullptr) {
            succs_[i] = bb;
        }
    }

    UNREACHABLE("Trying to add too many successors to the basic block.");
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

    for (auto succ : succs_) {
        if (succ->GetId() == bb->GetId()) {
            succ = nullptr;
            break;
        }
    }
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

void BasicBlock::PushBackInst(std::unique_ptr<InstBase> inst)
{
    assert(inst != nullptr);
    assert(!inst->IsPhi());

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
}

void BasicBlock::InsertInst(std::unique_ptr<InstBase> inst, InstBase* left, InstBase* right)
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
}

void BasicBlock::InsertInstAfter(std::unique_ptr<InstBase> inst, InstBase* after)
{
    assert(after != nullptr);
    auto next = after->GetNext();

    if (next == nullptr) {
        PushBackInst(std::move(inst));
    } else {
        InsertInst(std::move(inst), after, next);
    }
}

void BasicBlock::InsertInstBefore(std::unique_ptr<InstBase> inst, InstBase* before)
{
    assert(before != nullptr);
    auto prev = before->GetPrev();

    if (prev == nullptr) {
        PushFrontInst(std::move(inst));
    } else {
        InsertInst(std::move(inst), prev, before);
    }
}

void BasicBlock::PushBackPhi(std::unique_ptr<InstBase> inst)
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
}

void BasicBlock::PushBackPhi(InstBase* inst)
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
}

void BasicBlock::UnlinkInst(InstBase* inst)
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
    for (auto bb : preds_) {
        std::cout << bb->GetId() << " ";
    }
    std::cout << "]\n";

    std::cout << "# SUCCS: [";
    for (auto bb : succs_) {
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
    assert(inst != nullptr);
    assert(first_inst_ == nullptr);
    assert(last_inst_ == nullptr);

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
    return id_ == Graph::BB_START_ID;
}

bool BasicBlock::IsEndBlock() const
{
    return succs_.empty();
}
