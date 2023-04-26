#include "linear_order.h"
#include "bb.h"
#include "graph.h"
#include "loop.h"
#include "marker/marker_factory.h"

bool LinearOrder::RunPass()
{
    graph_->GetAnalyser()->GetValidPass<LoopAnalysis>();
    graph_->GetAnalyser()->GetValidPass<DomTree>();

    Markers markers = { marker::MarkerFactory::AcquireMarker() };

    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
        if (bb->ProbeMark(&markers[Marks::VISITED])) {
            continue;
        }

        if (bb->IsLoopHeader()) {
            LinearizeLoop(bb->GetLoop(), markers);
            continue;
        }

        linear_bb_.push_back(bb);
        bb->SetMark(&markers[Marks::VISITED]);
    }

    ResetState();
    RunPass_(graph_->GetStartBasicBlock());
    Check();
    SetValid(true);

    return true;
}

std::vector<BasicBlock*> LinearOrder::GetBlocks()
{
    return linear_bb_;
}

void LinearOrder::RunPass_(BasicBlock* cur_bb)
{
    cur_bb->Dump();
}

void LinearOrder::LinearizeLoop([[maybe_unused]] Loop* loop, [[maybe_unused]] const Markers m)
{
}

void LinearOrder::Check()
{
    auto sz = linear_bb_.size();
    for (size_t i = 0; i < sz; ++i) {
        using BranchFlag = isa::flag::Flag<isa::flag::BRANCH>;
        auto bb = linear_bb_[i];
        if (bb->GetNumSuccessors() > BranchFlag::Value::ONE_SUCCESSOR) {
            for (size_t s = 0; s < bb->GetNumSuccessors(); ++s) {
                if (s == Conditional::Branch::FALLTHROUGH) {
                    assert(i + 1 < sz);
                    assert(bb->GetSuccessor(s)->GetId() == linear_bb_[i + 1]->GetId());
                } else {
                    assert(i + 1 < sz);
                    assert(bb->GetSuccessor(s)->GetId() != linear_bb_[i + 1]->GetId());
                }
            }
        } else if (bb->GetNumSuccessors() == BranchFlag::Value::ONE_SUCCESSOR) {
            if (!bb->GetLastInst()->IsUnconditionalJump()) {
                auto succ = bb->GetSuccessor(Conditional::Branch::FALLTHROUGH);
                assert(succ->GetId() == linear_bb_[i + 1]->GetId());
            }
        }
    }
}

void LinearOrder::ResetState()
{
    linear_bb_.clear();
}
