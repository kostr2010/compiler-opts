#ifndef __PASS_DOM_TREE_INCLUDED__
#define __PASS_DOM_TREE_INCLUDED__

#include <memory>
#include <unordered_map>

#include "pass.h"

// FIXME:
class DomTree : public Pass
{
  public:
    using Marks = MarksT<1>;
    enum MarkType
    {
        IN_TREE = 0,
    };

    DomTree(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(DomTree);

    bool RunPass() override;

  private:
    struct Node
    {
        Node(BasicBlock* block) : bb(block)
        {
        }

        Node* sdom = nullptr;
        Node* pred = nullptr;
        std::vector<Node*> succs{};

        size_t dfs_idx{};

        BasicBlock* bb = nullptr;
    };

    void FillTree();
    void FillTree_(Node* node);
    void ComputeAncestors(BasicBlock* bb, std::vector<Node*>* pot_doms, size_t w_dfs_idx);
    Node* ComputeSdom(Node* node);

    std::unordered_map<IdType, size_t> id_to_dfs_idx_{};
    std::vector<BasicBlock*> dfs_bb_;
    std::vector<Node> tree_{};
};

#endif