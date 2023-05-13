#ifndef __DCE_H_INCLUDED__
#define __DCE_H_INCLUDED__

#include "pass.h"
#include "utils/marker/marker.h"

class BasicBlock;
class InstBase;

class DCE : public Pass
{
  public:
    using is_cfg_sensitive = std::true_type;

    enum Marks
    {
        VISITED = 0,
        N_MARKS,
    };
    using Markers = marker::Markers<Marks::N_MARKS>;

    DCE(Graph* graph) : Pass(graph)
    {
    }

    bool Run() override;

  private:
    void Mark(const Markers markers);
    void MarkRecursively(InstBase* inst, const Markers markers);
    void Sweep(const Markers markers);
};

#endif