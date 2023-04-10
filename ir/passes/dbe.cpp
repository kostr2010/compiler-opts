#include "dbe.h"
#include "bb.h"
#include "graph.h"

bool DBE::RunPass()
{
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
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
    if (!bb->IsEmpty()) {
        return false;
    }

    // empty bb can only have 1 successor
    assert(bb->GetNumSuccessors() <= 1);

    auto preds = bb->GetPredecessors();
    for (const auto& pred : preds) {
        graph_->RemoveEdge(pred, bb);
    }

    if (!bb->HasNoSuccessors()) {
        for (const auto& pred : preds) {
            graph_->AddEdge(pred, bb->GetSuccessor(0));
        }
        graph_->RemoveEdge(bb, bb->GetSuccessor(0));
    }

    assert(bb->HasNoSuccessors());
    assert(bb->HasNoPredecessors());

    graph_->DestroyBasicBlock(bb->GetId());

    return true;
}

bool DBE::RemoveUnlinked(BasicBlock* bb)
{
    bool is_unlinked = bb->HasNoSuccessors() && bb->HasNoPredecessors();
    if (!is_unlinked) {
        return false;
    }

    graph_->DestroyBasicBlock(bb->GetId());

    return true;
}
