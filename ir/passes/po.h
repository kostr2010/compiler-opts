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

    static constexpr size_t GetNumMarks()
    {
        return Marks::N_MARKS;
    }

    PO(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(PO);

    bool RunPass() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void RunPass_(BasicBlock* cur_bb);
    void ResetStructs();
    void ClearMarks();

    std::vector<BasicBlock*> po_bb_{};
};

#endif
