#ifndef __PASS_PO_INCLUDED__
#define __PASS_PO_INCLUDED__

#include "pass.h"

#include <vector>

class BasicBlock;

class PO : public Pass
{
  public:
    enum Marks
    {
        VISITED = 0,
        N_MARKS,
    };

    PO(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void RunPass_(BasicBlock* cur_bb);
    void ResetState();
    void ClearMarks();

    std::vector<BasicBlock*> po_bb_{};
};

template <>
struct Pass::PassTraits<PO>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
    using num_marks = std::integral_constant<size_t, PO::Marks::N_MARKS>;
};

#endif
