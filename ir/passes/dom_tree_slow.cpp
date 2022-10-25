#include "dom_tree_slow.h"
#include "bb.h"
#include "graph.h"

#include <algorithm>

bool DomTreeSlow::RunPass()
{
    graph_->ClearDominators();
    graph_->RunPass<RPO>();

    auto reachable = graph_->GetPass<RPO>()->GetBlocks();

    auto comparator = [](BasicBlock* rhs, BasicBlock* lhs) { return rhs->GetId() < lhs->GetId(); };
    std::sort(reachable.begin(), reachable.end(), comparator);

    for (auto& bb : reachable) {
        bb->AddDominator(graph_->GetStartBasicBlock());
        graph_->GetStartBasicBlock()->AddDominated(bb);

        graph_->UnbindBasicBlock(bb);

        graph_->RunPass<RPO>();
        auto new_reachable = graph_->GetPass<RPO>()->GetBlocks();
        std::sort(new_reachable.begin(), new_reachable.end(), comparator);

        std::vector<BasicBlock*> diff{};

        std::set_difference(reachable.begin(), reachable.end(), new_reachable.begin(),
                            new_reachable.end(), std::back_inserter(diff), comparator);

        for (auto bb_dom : diff) {
            bb_dom->AddDominator(bb);
            bb->AddDominated(bb_dom);
        }

        graph_->BindBasicBlock(bb);
    }

    return true;
}