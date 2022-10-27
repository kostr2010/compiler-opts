#include "dom_tree_slow.h"
#include "bb.h"
#include "graph.h"

#include <algorithm>

bool DomTreeSlow::RunPass()
{
    graph_->ClearDominators();

    auto analyser = graph_->GetAnalyser();

    analyser->RunPass<RPO>();

    auto reachable = analyser->GetPass<RPO>()->GetBlocks();

    auto comparator = [](BasicBlock* rhs, BasicBlock* lhs) { return rhs->GetId() < lhs->GetId(); };
    std::sort(reachable.begin(), reachable.end(), comparator);

    for (auto& bb : reachable) {
        bb->AddDominator(graph_->GetStartBasicBlock());
        graph_->GetStartBasicBlock()->AddDominated(bb);

        analyser->SetMark<RPO, RPO::MarkType::VISITED>(bb->GetBits());
        analyser->RunPass<RPO>();

        auto new_reachable = analyser->GetPass<RPO>()->GetBlocks();
        std::sort(new_reachable.begin(), new_reachable.end(), comparator);

        std::vector<BasicBlock*> diff{};

        std::set_difference(reachable.begin(), reachable.end(), new_reachable.begin(),
                            new_reachable.end(), std::back_inserter(diff), comparator);

        for (auto bb_dom : diff) {
            bb_dom->AddDominator(bb);
            bb->AddDominated(bb_dom);
        }

        analyser->ClearMark<RPO, RPO::MarkType::VISITED>(bb->GetBits());
    }

    return true;
}