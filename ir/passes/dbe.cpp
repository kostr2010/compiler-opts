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
    assert(bb->GetSuccessors().size() <= 1);

    auto preds = bb->GetPredecesors();
    for (const auto& pred : preds) {
        graph_->RemoveEdge(pred, bb);
    }

    if (!bb->GetSuccessors().empty()) {
        for (const auto& pred : preds) {
            graph_->AddEdge(pred, bb->GetSuccessors().front());
        }
        graph_->RemoveEdge(bb, bb->GetSuccessors().front());
    }

    assert(bb->GetSuccessors().empty());
    assert(bb->GetPredecesors().empty());

    graph_->DestroyBasicBlock(bb->GetId());

    return true;
}

bool DBE::RemoveUnlinked(BasicBlock* bb)
{
    bool is_unlinked = bb->GetSuccessors().empty() && bb->GetPredecesors().empty();
    if (!is_unlinked) {
        return false;
    }

    graph_->DestroyBasicBlock(bb->GetId());

    return true;
}
