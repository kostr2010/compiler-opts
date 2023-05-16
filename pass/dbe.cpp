#include "dbe.h"
#include "ir/bb.h"
#include "ir/graph.h"

bool DBE::Run()
{
    for (const auto& bb : graph_->GetPassManager()->GetValidPass<RPO>()->GetBlocks()) {
        if (bb->IsStartBlock()) {
            continue;
        }

        if (RemoveEmpty(bb)) {
            continue;
        }

        if (RemoveUnlinked(bb)) {
            continue;
        }
    }

    return true;
}

bool DBE::RemoveEmpty(BasicBlock* bb)
{
    ASSERT(!bb->IsStartBlock());

    if (!bb->IsEmpty()) {
        return false;
    }

    // empty bb can only have 1 successor
    ASSERT(bb->GetNumSuccessors() ==
           isa::flag::Flag<isa::flag::Type::BRANCH>::Value::ONE_SUCCESSOR);
    ASSERT(bb->GetSuccessor(0) != nullptr);
    auto succ = bb->GetSuccessor(0);

    auto preds = bb->GetPredecessors();
    for (const auto& pred : preds) {
        graph_->ReplaceSuccessor(pred, bb, succ);
        graph_->ReplaceSuccessor(bb, succ, nullptr);
    }

    ASSERT(bb->HasNoSuccessors());
    ASSERT(bb->HasNoPredecessors());

    graph_->DestroyBasicBlock(bb);

    return true;
}

bool DBE::RemoveUnlinked(BasicBlock* bb)
{
    ASSERT(!bb->IsStartBlock());

    bool is_unlinked = bb->HasNoSuccessors() && bb->HasNoPredecessors();
    if (!is_unlinked) {
        return false;
    }

    graph_->DestroyBasicBlock(bb);

    return true;
}
