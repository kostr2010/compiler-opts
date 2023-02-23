#ifndef __DCE_H_INCLUDED__
#define __DCE_H_INCLUDED__

#include "pass.h"

class BasicBlock;
class Inst;

class DCE : public Pass
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

    DCE(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

  private:
    void Mark();
    void MarkRecursively(Inst* inst);
    void RemoveInst(Inst* inst);
    void Sweep();
    void ClearMarks();
};

#endif