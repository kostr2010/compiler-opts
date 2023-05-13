#ifndef __LIVENESS_ANALYSIS_H_INCLUDED__
#define __LIVENESS_ANALYSIS_H_INCLUDED__

#include "ir/typedefs.h"
#include "pass.h"
#include "utils/marker/marker.h"
#include "utils/range/range.h"

#include <set>
#include <unordered_map>
#include <vector>

class BasicBlock;
class InstBase;

class LivenessAnalysis : public Pass
{
  public:
    using is_cfg_sensitive = std::true_type;

    enum Marks
    {
        VISITED = 0,
        N_MARKS,
    };
    using Markers = marker::Markers<Marks::N_MARKS>;

    static constexpr size_t LIVE_NUMBER_STEP = 2;
    static constexpr size_t LINEAR_NUMBER_STEP = 1;

    LivenessAnalysis(Graph* graph) : Pass(graph)
    {
    }

    bool Run() override;

    auto GetInstLiveNumbers() const
    {
        return inst_live_numbers_;
    }

    auto GetInstLiveRanges() const
    {
        return inst_live_ranges_;
    }

    auto GetBasicBlockLiveRanges() const
    {
        return bb_live_ranges_;
    }

  private:
    using LiveSet = std::set<InstBase*>;

    static LiveSet Union(const LiveSet& a, const LiveSet& b);

    void Init();
    bool AllForwardEdgesVisited(BasicBlock* bb, Markers markers);
    void LinearizeBlocks();
    void CheckLinearOrder();
    void CalculateLiveness();
    void CalculateLiveRanges(BasicBlock* bb);
    void InstAddLiveRange(InstBase* inst, const Range& range);
    void CalculateInitialLiveSet(BasicBlock* bb);
    void ResetState();

    std::vector<BasicBlock*> linear_blocks_{};
    std::unordered_map<InstBase*, size_t> inst_linear_numbers_{};
    std::unordered_map<InstBase*, size_t> inst_live_numbers_{};
    std::unordered_map<InstBase*, Range> inst_live_ranges_{};
    std::unordered_map<BasicBlock*, Range> bb_live_ranges_{};
    std::unordered_map<BasicBlock*, LiveSet> bb_live_sets_{};
};

#endif