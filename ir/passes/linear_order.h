#ifndef __PASS_LINEAR_ORDER_INCLUDED__
#define __PASS_LINEAR_ORDER_INCLUDED__

#include "pass.h"

#include <vector>

class BasicBlock;
class Loop;

class LinearOrder : public Pass
{
  public:
    LinearOrder(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

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

template <>
struct Pass::PassTraits<LinearOrder>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
};

#endif
