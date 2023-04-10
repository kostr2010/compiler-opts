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
    using Markers = marking::Markers<Marks::N_MARKS>;

    DCE(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

  private:
    void Mark(const Markers markers);
    void MarkRecursively(Inst* inst, const Markers markers);
    void Sweep(const Markers markers);
    void RemoveInst(Inst* inst);
    void ClearMarks();
};

template <>
struct Pass::PassTraits<DCE>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
};

#endif