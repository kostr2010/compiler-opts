#include "linear_order.h"
#include "ir/bb.h"
#include "ir/graph.h"
#include "ir/loop.h"

using BranchFlag = isa::flag::Flag<isa::flag::Type::BRANCH>;

void LinearOrder::AppendJump(BasicBlock* bb)
{
    ASSERT(bb != nullptr);

    auto inst = bb->GetLastInst();

    if (inst == nullptr || !inst->IsUnconditionalJump()) {
        ASSERT(inst == nullptr || !inst->HasFlag<isa::flag::Type::BRANCH>());
        bb->PushBackInst(InstBase::NewInst<isa::inst::Opcode::JMP>());
    }
}

BasicBlock* LinearOrder::InsertJumpBasicBlock(BasicBlock* prev, BasicBlock* next)
{
    ASSERT(prev != nullptr);
    ASSERT(next != nullptr);
    ASSERT(prev->GetLoop() != nullptr);

    auto loop = prev->GetLoop();
    auto bb_new = graph_->NewBasicBlock();
    ASSERT(bb_new != nullptr);
    AppendJump(bb_new);

    loop->AddBlock(bb_new);
    bb_new->SetLoop(loop);

    graph_->InsertBasicBlock(bb_new, prev, next);
    return bb_new;
}

bool LinearOrder::Run()
{
    ResetState();

    graph_->GetPassManager()->GetValidPass<DomTree>();
    graph_->GetPassManager()->GetValidPass<LoopAnalysis>();

    Linearize();
    Check();
    SetValid(true);

    return true;
}

void LinearOrder::Linearize()
{
    BasicBlock* prev = nullptr;
    for (const auto& bb : graph_->GetPassManager()->GetValidPass<RPO>()->GetBlocks()) {
        if (prev != nullptr) {
            switch (prev->GetNumSuccessors()) {
            case BranchFlag::Value::NO_SUCCESSORS: {
                break;
            }
            case BranchFlag::Value::ONE_SUCCESSOR: {
                ProcessSingleSuccessor(bb, prev);
                break;
            }
            case BranchFlag::Value::TWO_SUCCESSORS: {
                ProcessTwoSuccessors(bb, prev);
                break;
            }
            default: {
                ProcessMultipleSuccessors(bb, prev);
                break;
            }
            }
        }

        linear_bb_.push_back(bb);
        prev = bb;
    }

    ProcessLast(prev);
}

void LinearOrder::ProcessLast(BasicBlock* bb)
{
    if (bb == nullptr) {
        return;
    }

    switch (bb->GetNumSuccessors()) {
    case BranchFlag::Value::NO_SUCCESSORS: {
        break;
    }
    case BranchFlag::Value::ONE_SUCCESSOR: {
        AppendJump(bb);
        break;
    }
    case BranchFlag::Value::TWO_SUCCESSORS: {
        auto bb_jmp = InsertJumpBasicBlock(bb, bb->GetSuccessor(Conditional::Branch::FALLTHROUGH));
        linear_bb_.push_back(bb_jmp);
        break;
    }
    default: {
        UNREACHABLE("invalid number of successors in end block! check the algorithm");
    }
    }
}

void LinearOrder::ProcessSingleSuccessor(BasicBlock* bb, BasicBlock* prev)
{
    ASSERT(bb != nullptr);
    ASSERT(prev != nullptr);
    ASSERT(prev->GetNumSuccessors() == BranchFlag::Value::ONE_SUCCESSOR);

    bool is_bb_after_prev =
        bb->GetId() == prev->GetSuccessor(Conditional::Branch::FALLTHROUGH)->GetId();
    bool is_last_inst_jump =
        (prev->GetLastInst() == nullptr) ? (false) : (prev->GetLastInst()->IsUnconditionalJump());

    if (!is_bb_after_prev && !is_last_inst_jump) {
        AppendJump(prev);
    }
}

void LinearOrder::ProcessTwoSuccessors(BasicBlock* bb, BasicBlock* prev)
{
    ASSERT(bb != nullptr);
    ASSERT(prev != nullptr);
    ASSERT(prev->GetNumSuccessors() == BranchFlag::Value::TWO_SUCCESSORS);
    ASSERT(prev->GetSuccessor(Conditional::Branch::FALLTHROUGH) != nullptr);
    ASSERT(prev->GetSuccessor(Conditional::Branch::BRANCH_TRUE) != nullptr);

    auto bb_f = prev->GetSuccessor(Conditional::Branch::FALLTHROUGH);
    auto bb_t = prev->GetSuccessor(Conditional::Branch::BRANCH_TRUE);

    if (bb->GetId() == bb_t->GetId()) {
        graph_->InvertCondition(prev);
    } else if (bb->GetId() != bb_f->GetId()) {
        auto bb_jmp = InsertJumpBasicBlock(prev, bb_f);
        linear_bb_.push_back(bb_jmp);
    }
}

void LinearOrder::ProcessMultipleSuccessors(BasicBlock* bb, BasicBlock* prev)
{
    // conservatively insert jmp basic block in every branch
    auto bb_jmp = InsertJumpBasicBlock(prev, bb);
    linear_bb_.push_back(bb_jmp);
}

std::vector<BasicBlock*> LinearOrder::GetBlocks()
{
    return linear_bb_;
}

void LinearOrder::Check()
{
#ifndef RELEASE_BUILD
    auto sz = linear_bb_.size();
    ASSERT(sz == graph_->GetPassManager()->GetValidPass<RPO>()->GetBlocks().size());

    for (unsigned i = 0; i < sz; ++i) {
        auto bb = linear_bb_[i];
        if (bb->GetNumSuccessors() > BranchFlag::Value::ONE_SUCCESSOR) {
            for (unsigned s = 0; s < bb->GetNumSuccessors(); ++s) {
                if (s == Conditional::Branch::FALLTHROUGH) {
                    ASSERT(i + 1 < sz);
                    ASSERT(bb->GetSuccessor(s)->GetId() == linear_bb_[i + 1]->GetId());
                } else {
                    ASSERT(i + 1 < sz);
                    ASSERT(bb->GetSuccessor(s)->GetId() != linear_bb_[i + 1]->GetId());
                }
            }
        } else if (bb->GetNumSuccessors() == BranchFlag::Value::ONE_SUCCESSOR) {
            auto last_inst = bb->GetLastInst();
            if (last_inst == nullptr || !last_inst->IsUnconditionalJump()) {
                auto succ = bb->GetSuccessor(Conditional::Branch::FALLTHROUGH);
                ASSERT(succ->GetId() == linear_bb_[i + 1]->GetId());
            }
        }
    }
#endif
}

void LinearOrder::ResetState()
{
    linear_bb_.clear();
}
