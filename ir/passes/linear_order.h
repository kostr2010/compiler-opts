#ifndef __PASS_LINEAR_ORDER_INCLUDED__
#define __PASS_LINEAR_ORDER_INCLUDED__

#include "pass.h"

#include <vector>

class BasicBlock;
class Loop;

class LinearOrder : public Pass
{
  public:
    enum Marks
    {
        VISITED = 0,
        N_MARKS,
    };
    using Markers = marker::Markers<Marks::N_MARKS>;

    LinearOrder(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void RunPass_(BasicBlock* cur_bb);
    void LinearizeLoop(Loop* loop, const Markers m);
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
