#ifndef __PASS_DOM_TREE_INCLUDED__
#define __PASS_DOM_TREE_INCLUDED__

#include <memory>
#include <unordered_map>

#include "pass.h"

// FIXME:
class DomTree : public Pass
{
  public:
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
        NO_DEFAULT_CTOR(Node);
        DEFAULT_DTOR(Node);

        Node* ancestor = nullptr;
        Node* parent = nullptr;
        Node* semi = nullptr;
        Node* label = nullptr;
        Node* dom = nullptr;

        std::vector<Node*> bucket{};
        std::vector<Node*> pred{};

        size_t dfs_idx{};

        BasicBlock* bb;
    };

    void FillTree();
    void FillTree_(Node* node);
    void ComputeDoms();
    void ComputeSdoms();
    void Link(Node* v, Node* w);
    Node* Eval(Node* v);
    void Compress(Node* v);

    std::unordered_map<IdType, size_t> id_to_dfs_idx_{};
    std::vector<Node> tree_{};
};

#endif