#include "dbe.h"
#include "bb.h"
#include "graph.h"

bool DBE::RunPass()
{
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
        if (bb->IsEmpty()) {
            // empty bb can only have 1 successor
            assert(bb->GetSuccessors().size() <= 1);
            auto preds = bb->GetPredecesors();
            for (const auto& pred : preds) {
                graph_->RemoveEdge(pred, bb);
            }

            if (bb->GetSuccessors().size() == 1) {
                for (const auto& pred : preds) {
                    graph_->AddEdge(pred, bb->GetSuccessors().front());
                }
                graph_->RemoveEdge(bb, bb->GetSuccessors().front());
            }

            graph_->ReleaseBasicBlock(bb->GetId());
        }
    }

    return true;
}
