#ifndef __PASS_LINEAR_ORDER_INCLUDED__
#define __PASS_LINEAR_ORDER_INCLUDED__

#include "pass.h"

#include <vector>

class BasicBlock;

class LinearOrder : public Pass
{
  public:
    enum Marks
    {
        VISITED = 0,
        N_MARKS,
    };

    LinearOrder(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void RunPass_(BasicBlock* cur_bb);
    void ResetState();
    void ClearMarks();

    std::vector<BasicBlock*> linear_bb_{};
};

template <>
struct Pass::PassTraits<LinearOrder>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
    using num_marks = std::integral_constant<size_t, LinearOrder::Marks::N_MARKS>;
};

#endif
