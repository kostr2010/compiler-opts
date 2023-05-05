#ifndef __PASS_RPO_INCLUDED__
#define __PASS_RPO_INCLUDED__

#include "pass.h"
#include "utils/marker/marker.h"

#include <vector>

class BasicBlock;

class RPO : public Pass
{
  public:
    enum Marks
    {
        VISITED = 0,
        N_MARKS,
    };
    using Markers = marker::Markers<Marks::N_MARKS>;

    RPO(Graph* graph) : Pass(graph)
    {
    }

    bool Run() override;

    std::vector<BasicBlock*> GetBlocks();

  private:
    void Run_(BasicBlock* cur_bb, const Markers markers);
    void ResetState();

    std::vector<BasicBlock*> rpo_bb_{};
};

template <>
struct Pass::PassTraits<RPO>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
};

#endif
