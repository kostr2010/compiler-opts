#ifndef __PASS_LINEAR_ORDER_INCLUDED__
#define __PASS_LINEAR_ORDER_INCLUDED__

#include "pass.h"

#include <vector>

class BasicBlock;

class LinearOrder : public Pass
{
  public:
    using is_cfg_sensitive = std::true_type;

    LinearOrder(Graph* graph) : Pass(graph)
    {
    }

    bool Run() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void Linearize();
    void AppendJump(BasicBlock* bb);
    BasicBlock* InsertJumpBasicBlock(BasicBlock* prev, BasicBlock* next);
    void ProcessSingleSuccessor(BasicBlock* bb, BasicBlock* prev);
    void ProcessTwoSuccessors(BasicBlock* bb, BasicBlock* prev);
    void ProcessMultipleSuccessors(BasicBlock* bb, BasicBlock* prev);
    void ProcessLast(BasicBlock* bb);
    void Check();
    void ResetState();

    std::vector<BasicBlock*> linear_bb_{};
};

#endif
