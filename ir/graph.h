#ifndef ___GRAPH_H_INCLUDED___
#define ___GRAPH_H_INCLUDED___

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "bb.h"
#include "pass/pass_manager.h"
#include "typedefs.h"

class InstBase;
class BasicBlock;

class Graph
{
  public:
    static constexpr IdType BB_START_ID = 0;
    Graph();
    ~Graph();

    BasicBlock* GetStartBasicBlock() const;
    BasicBlock* GetBasicBlock(IdType bb_id) const;

    void ClearDominators();
    void ClearLoops();

    void AddEdge(BasicBlock* from, BasicBlock* to, unsigned slot);

    void InsertBasicBlock(BasicBlock* bb, BasicBlock* from, BasicBlock* to);
    void InsertBasicBlockBefore(BasicBlock* bb, BasicBlock* before);
    void ReplaceSuccessor(BasicBlock* bb, BasicBlock* prev_succ, BasicBlock* new_succ);
    void InvertCondition(BasicBlock* bb);
    void SwapTwoSuccessors(BasicBlock* bb);

    BasicBlock* SplitBasicBlock(InstBase* inst_after);

    BasicBlock* NewBasicBlock();
    // used to accept BB's released by another graph. obtains ownership of a pointer
    IdType NewBasicBlock(BasicBlock* bb);
    // used to transfer ownership of a block to another graph
    BasicBlock* ReleaseBasicBlock(IdType id);
    // null basic block, invalidate previous pointer
    void DestroyBasicBlock(BasicBlock* bb);

    void Dump(std::string name = "");

    PassManager* GetPassManager()
    {
        return &pass_mgr_;
    }

  private:
    void InitStartBlock();

    std::vector<std::unique_ptr<BasicBlock> > bb_vector_{};

    IdType bb_id_counter_{};

    PassManager pass_mgr_;

    // Metadata metadata;
};

#endif
