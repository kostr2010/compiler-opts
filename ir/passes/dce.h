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

template <>
struct Pass::PassTraits<DCE>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
    using num_marks = std::integral_constant<size_t, DCE::Marks::N_MARKS>;
};

#endif