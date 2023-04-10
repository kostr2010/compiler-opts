#ifndef __LIVENESS_ANALYSIS_H_INCLUDED__
#define __LIVENESS_ANALYSIS_H_INCLUDED__

#include "ir_isa.h"
#include "pass.h"

#include <unordered_map>
#include <vector>

class BasicBlock;
class Inst;

class LivenessAnalysis : public Pass
{
  public:
    LivenessAnalysis(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

  private:
    void ResetState();
};

template <>
struct Pass::PassTraits<LivenessAnalysis>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
};

#endif