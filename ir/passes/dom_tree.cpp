#include <algorithm>
#include <unordered_map>

#include "bb.h"
#include "dom_tree.h"
#include "graph.h"

static inline char IdToChar(IdType id)
{
    return (id == 0) ? '0' : (char)('A' + id - 1);
}

bool DomTree::RunPass()
{
    graph_->ClearDominators();

    FillTree();

    for (const auto& node : tree_) {
        std::cout << IdToChar(node.bb->GetId()) << "\n";
        std::cout << "\tpred:  "
                  << ((node.pred != nullptr) ? IdToChar(node.pred->bb->GetId()) : '-') << "\n";
        std::cout << "\tsuccs: ";
        for (const auto& succ : node.succs) {
            std::cout << IdToChar(succ->bb->GetId()) << " ";
        }
        std::cout << "\n";
    }

    for (auto node = tree_.begin() + 1; node != tree_.end(); ++node) {
        ComputeSdom(&(*node));
    }

    for (const auto& node : tree_) {
        std::cout << IdToChar(node.bb->GetId()) << "\n";
        std::cout << "\tsdom: "
                  << ((node.sdom != nullptr) ? IdToChar(node.sdom->bb->GetId()) : '-') << "\n";
    }

    return true;
}

void DomTree::ComputeAncestors(BasicBlock* bb, std::vector<Node*>* pot_doms, size_t w_dfs_idx)
{
    auto idx_u = id_to_dfs_idx_.at(bb->GetId());
    if (idx_u > w_dfs_idx) {
        pot_doms->push_back(ComputeSdom(&(tree_.at(idx_u))));
        for (auto pred : bb->GetPredecesors()) {
            ComputeAncestors(pred, pot_doms, w_dfs_idx);
        }
    }
}

DomTree::Node* DomTree::ComputeSdom(DomTree::Node* w)
{
    assert(!w->bb->IsStartBlock());

    auto sdom = w->sdom;
    if (sdom != nullptr) {
        return sdom;
    }

    std::cout << "computing sdoms for " << IdToChar(w->bb->GetId()) << "\n";

    std::vector<Node*> potential_sdoms{};

    if (w->pred->dfs_idx < w->dfs_idx) {
        potential_sdoms.push_back(w->pred);
    }

    for (auto v : w->bb->GetPredecesors()) {
        for (auto u : v->GetPredecesors()) {
            ComputeAncestors(u, &potential_sdoms, w->dfs_idx);
        }
    }

    std::cout << "potential sdoms for " << IdToChar(w->bb->GetId()) << ": ";
    for (auto node : potential_sdoms) {
        std::cout << IdToChar(node->bb->GetId()) << " ";
    }
    std::cout << "\n";

    auto comparator = [this](Node* lhs, Node* rhs) { return lhs->dfs_idx < rhs->dfs_idx; };

    auto it = std::min_element(potential_sdoms.begin(), potential_sdoms.end(), comparator);

    w->sdom = *it;

    return w->sdom;
}

void DomTree::FillTree()
{
    dfs_bb_ = graph_->GetAnalyser()->GetValidPass<DFS>()->GetBlocks();
    for (size_t i = 0; i < dfs_bb_.size(); ++i) {
        auto bb = dfs_bb_.at(i);
        tree_.emplace_back(bb);
        id_to_dfs_idx_[bb->GetId()] = i;
    }

    FillTree_(&(tree_.at(0)));
}

void DomTree::FillTree_(Node* node)
{
    auto analyser = graph_->GetAnalyser();
    analyser->SetMark<DomTree, MarkType::IN_TREE>(node->bb->GetBits());

    for (auto succ : node->bb->GetSuccessors()) {
        if (!analyser->GetMark<DomTree, MarkType::IN_TREE>(*(succ->GetBits()))) {
            auto succ_dfs_idx = id_to_dfs_idx_.at(succ->GetId());
            auto succ_node = &(tree_.at(succ_dfs_idx));
            succ_node->dfs_idx = succ_dfs_idx;
            succ_node->pred = node;
            node->succs.push_back(succ_node);

            FillTree_(succ_node);
        }
    }
}