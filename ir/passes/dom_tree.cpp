#include <algorithm>
#include <unordered_map>

#include "bb.h"
#include "dom_tree.h"
#include "graph.h"

bool DomTree::RunPass()
{
    graph_->ClearDominators();

    ResetState();
    FillTree();
    ComputeSdoms();
    ComputeDoms();

    SetValid(true);

    return true;
}

void DomTree::ComputeDoms()
{
    for (auto w = tree_.begin() + 1; w != tree_.end(); ++w) {
        if (w->dom->dfs_idx != w->semi->dfs_idx) {
            w->dom = w->dom->dom;
        }

        w->bb->SetImmDominator(w->dom->bb);
    }
}

void DomTree::ComputeSdoms()
{
    for (auto w = tree_.rbegin(); w != tree_.rend() - 1; ++w) {
        for (auto v : w->pred) {
            auto u = Eval(v);
            if (u->semi < w->semi) {
                w->semi = u->semi;
            }
        }

        w->semi->bucket.push_back(&(*w));
        Link(w->parent, &(*w));

        for (const auto& v : w->parent->bucket) {
            auto u = Eval(v);
            v->dom = (u->semi->dfs_idx < v->semi->dfs_idx) ? u : w->parent;
        }
    }
}

void DomTree::Link(Node* v, Node* w)
{
    w->ancestor = v;
}

DomTree::Node* DomTree::Eval(Node* v)
{
    if (v->ancestor == nullptr) {
        return v;
    }
    Compress(v);

    return v->label;
}

void DomTree::Compress(Node* v)
{
    assert(v->ancestor != nullptr);

    if (v->ancestor->ancestor == nullptr) {
        return;
    }

    Compress(v->ancestor);

    if (v->ancestor->label->semi->dfs_idx < v->label->semi->dfs_idx) {
        v->label = v->ancestor->label;
    }

    v->ancestor = v->ancestor->ancestor;
}

void DomTree::FillTree()
{
    auto dfs_bb = graph_->GetAnalyser()->GetValidPass<DFS>()->GetBlocks();
    tree_.reserve(dfs_bb.size());
    for (size_t i = 0; i < dfs_bb.size(); ++i) {
        auto bb = dfs_bb.at(i);
        id_to_dfs_idx_[bb->GetId()] = i;
        tree_.emplace_back(bb);
        tree_.back().dfs_idx = i;
    }

    auto root = &(tree_.at(0));
    FillTree_(root);
}

void DomTree::FillTree_(Node* v)
{
    v->label = v;
    v->semi = v;
    for (auto w_bb : v->bb->GetSuccessors()) {
        auto w = &(tree_.at(id_to_dfs_idx_.at(w_bb->GetId())));
        if (w->semi == nullptr) {
            w->parent = v;
            FillTree_(w);
        }
        w->pred.push_back(v);
    }
}

void DomTree::ResetState()
{
    tree_.clear();
    id_to_dfs_idx_.clear();
}
