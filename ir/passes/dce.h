#ifndef __DCE_H_INCLUDED__
#define __DCE_H_INCLUDED__

#include "pass.h"

class BasicBlock;
class InstBase;

class DCE : public Pass
{
  public:
    enum Marks
    {
        VISITED = 0,
        N_MARKS,
    };
    using Markers = marker::Markers<Marks::N_MARKS>;

    DCE(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

  private:
    void Mark(const Markers markers);
    void MarkRecursively(InstBase* inst, const Markers markers);
    void Sweep(const Markers markers);
    void RemoveInst(InstBase* inst);
    void ClearMarks();
};

template <>
struct Pass::PassTraits<DCE>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
};

#endif