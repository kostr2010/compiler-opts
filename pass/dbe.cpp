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
    assert(!bb->IsStartBlock());

    if (!bb->IsEmpty()) {
        return false;
    }

    // empty bb can only have 1 successor
    assert(bb->GetNumSuccessors() ==
           isa::flag::Flag<isa::flag::Type::BRANCH>::Value::ONE_SUCCESSOR);
    assert(bb->GetSuccessor(0) != nullptr);
    auto succ = bb->GetSuccessor(0);

    auto preds = bb->GetPredecessors();
    for (const auto& pred : preds) {
        graph_->ReplaceSuccessor(pred, bb, succ);
        graph_->ReplaceSuccessor(bb, succ, nullptr);
    }

    assert(bb->HasNoSuccessors());
    assert(bb->HasNoPredecessors());

    graph_->DestroyBasicBlock(bb);

    return true;
}

bool DBE::RemoveUnlinked(BasicBlock* bb)
{
    assert(!bb->IsStartBlock());

    bool is_unlinked = bb->HasNoSuccessors() && bb->HasNoPredecessors();
    if (!is_unlinked) {
        return false;
    }

    graph_->DestroyBasicBlock(bb);

    return true;
}
