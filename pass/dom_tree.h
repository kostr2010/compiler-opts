#ifndef __PASS_DOM_TREE_INCLUDED__
#define __PASS_DOM_TREE_INCLUDED__

#include <memory>
#include <unordered_map>

#include "ir/typedefs.h"
#include "pass.h"

class BasicBlock;
class DomTree : public Pass
{
  public:
    DomTree(Graph* graph) : Pass(graph)
    {
    }

    bool Run() override;

  private:
    struct Node
    {
        Node(BasicBlock* block) : bb(block)
        {
        }

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
    void ResetState();

    std::unordered_map<IdType, size_t> id_to_dfs_idx_{};
    std::vector<Node> tree_{};
};

template <>
struct Pass::PassTraits<DomTree>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
};

#endif
