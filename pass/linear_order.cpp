#include "linear_order.h"
#include "ir/bb.h"
#include "ir/graph.h"
#include "ir/loop.h"
#include "utils/marker/marker_factory.h"

using BranchFlag = isa::flag::Flag<isa::flag::Type::BRANCH>;

void LinearOrder::AppendJump(BasicBlock* bb)
{
    assert(bb != nullptr);

    auto inst = bb->GetLastInst();

    if (inst == nullptr || !inst->IsUnconditionalJump()) {
        assert(inst == nullptr || !inst->HasFlag<isa::flag::Type::BRANCH>());
        bb->PushBackInst(InstBase::NewInst<isa::inst::Opcode::JMP>());
    }
}

BasicBlock* LinearOrder::InsertJumpBasicBlock(BasicBlock* prev, BasicBlock* next)
{
    assert(prev != nullptr);
    assert(next != nullptr);
    assert(prev->GetLoop() != nullptr);

    auto loop = prev->GetLoop();
    auto bb_new = graph_->NewBasicBlock();
    assert(bb_new != nullptr);
    AppendJump(bb_new);

    loop->AddBlock(bb_new);
    bb_new->SetLoop(loop);

    graph_->InsertBasicBlock(bb_new, prev, next);
    return bb_new;
}

bool LinearOrder::RunPass()
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
        // LOG("APPEND JMP TO " << bb->GetId());
        AppendJump(bb);
        break;
    }
    case BranchFlag::Value::TWO_SUCCESSORS: {
        // LOG("INSERT JMP BB BETWEEN: "
        //     << bb->GetId() << " " <<
        //     bb->GetSuccessor(Conditional::Branch::FALLTHROUGH)->GetId());
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
    assert(bb != nullptr);
    assert(prev != nullptr);
    assert(prev->GetNumSuccessors() == BranchFlag::Value::ONE_SUCCESSOR);

    bool is_bb_after_prev =
        bb->GetId() == prev->GetSuccessor(Conditional::Branch::FALLTHROUGH)->GetId();
    bool is_last_inst_jump =
        (prev->GetLastInst() == nullptr) ? (false) : (prev->GetLastInst()->IsUnconditionalJump());

    if (!is_bb_after_prev && !is_last_inst_jump) {
        // LOG("APPEND JMP TO " << prev->GetId());
        AppendJump(prev);
    }
}

void LinearOrder::ProcessTwoSuccessors(BasicBlock* bb, BasicBlock* prev)
{
    assert(bb != nullptr);
    assert(prev != nullptr);
    assert(prev->GetNumSuccessors() == BranchFlag::Value::TWO_SUCCESSORS);
    assert(prev->GetSuccessor(Conditional::Branch::FALLTHROUGH) != nullptr);
    assert(prev->GetSuccessor(Conditional::Branch::BRANCH_TRUE) != nullptr);

    auto bb_f = prev->GetSuccessor(Conditional::Branch::FALLTHROUGH);
    auto bb_t = prev->GetSuccessor(Conditional::Branch::BRANCH_TRUE);

    if (bb->GetId() == bb_t->GetId()) {
        // LOG("INVERTED: " << prev->GetId());
        graph_->InvertCondition(prev);
    } else if (bb->GetId() != bb_f->GetId()) {
        // LOG("INSERT JMP BB BETWEEN: " << prev->GetId() << " " << bb->GetId());
        auto bb_jmp = InsertJumpBasicBlock(prev, bb_f);
        linear_bb_.push_back(bb_jmp);
    }
}

void LinearOrder::ProcessMultipleSuccessors(BasicBlock* bb, BasicBlock* prev)
{
    LOG("warning! untested! for isa's with 3-way jumps and more");

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
    auto sz = linear_bb_.size();
    assert(sz == graph_->GetPassManager()->GetValidPass<RPO>()->GetBlocks().size());

    for (size_t i = 0; i < sz; ++i) {
        using BranchFlag = isa::flag::Flag<isa::flag::BRANCH>;
        auto bb = linear_bb_[i];
        if (bb->GetNumSuccessors() > BranchFlag::Value::ONE_SUCCESSOR) {
            for (size_t s = 0; s < bb->GetNumSuccessors(); ++s) {
                if (s == Conditional::Branch::FALLTHROUGH) {
                    assert(i + 1 < sz);
                    // LOG("bb " << bb->GetId() << ", false " << bb->GetSuccessor(s)->GetId()
                    //           << ", linear_bb_[i + 1] " << linear_bb_[i + 1]->GetId());
                    assert(bb->GetSuccessor(s)->GetId() == linear_bb_[i + 1]->GetId());
                } else {
                    assert(i + 1 < sz);
                    // LOG("bb " << bb->GetId() << ", branch " << bb->GetSuccessor(s)->GetId()
                    //           << ", linear_bb_[i + 1] " << linear_bb_[i + 1]->GetId());
                    assert(bb->GetSuccessor(s)->GetId() != linear_bb_[i + 1]->GetId());
                }
            }
        } else if (bb->GetNumSuccessors() == BranchFlag::Value::ONE_SUCCESSOR) {
            auto last_inst = bb->GetLastInst();
            if (last_inst == nullptr || !last_inst->IsUnconditionalJump()) {
                auto succ = bb->GetSuccessor(Conditional::Branch::FALLTHROUGH);
                // LOG("bb " << bb->GetId() << ", succ " << succ->GetId() << ", linear_bb_[i + 1]"
                //           << linear_bb_[i + 1]->GetId());
                assert(succ->GetId() == linear_bb_[i + 1]->GetId());
            }
        }
    }
}

void LinearOrder::ResetState()
{
    linear_bb_.clear();
}
