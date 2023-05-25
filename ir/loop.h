#ifndef __LOOP_H_INCLUDED__
#define __LOOP_H_INCLUDED__

#include "typedefs.h"
#include "utils/macros.h"

#include <vector>

class BasicBlock;

class Loop
{
  public:
    explicit Loop(IdType id, BasicBlock* head, BasicBlock* back_edge);
    explicit Loop(IdType id) : id_(id)
    {
    }

    NO_COPY_SEMANTIC(Loop);
    NO_MOVE_SEMANTIC(Loop);

    GETTER(Id, id_);
    GETTER(Header, header_);
    GETTER(Blocks, blocks_);
    GETTER(BackEdges, back_edges_);
    GETTER(InnerLoops, inner_loops_);
    GETTER_SETTER(OuterLoop, Loop*, outer_loop_);
    GETTER_SETTER(PreHeader, BasicBlock*, pre_header_);

    bool IsRoot() const;
    void CalculateReducibility();
    bool IsReducible() const;
    void AddInnerLoop(Loop* loop);
    void AddBlock(BasicBlock* bb);
    void AddBackEdge(BasicBlock* bb);
    void ClearBackEdges();

    bool Inside(const Loop* other) const;

    void Dump();

  private:
    IdType id_;

    BasicBlock* header_ = nullptr;
    BasicBlock* pre_header_ = nullptr;

    std::vector<BasicBlock*> back_edges_ = {};
    std::vector<BasicBlock*> blocks_ = {};

    Loop* outer_loop_ = nullptr;
    std::vector<Loop*> inner_loops_ = {};

    bool is_reducible_ = true;
};

#endif
