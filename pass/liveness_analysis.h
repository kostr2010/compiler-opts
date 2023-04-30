#ifndef __LIVENESS_ANALYSIS_H_INCLUDED__
#define __LIVENESS_ANALYSIS_H_INCLUDED__

#include "ir/typedefs.h"
#include "pass.h"
#include "utils/range/range.h"

#include <set>
#include <unordered_map>
#include <vector>

class BasicBlock;
class InstBase;

class LivenessAnalysis : public Pass
{
  public:
    // struct LiveRange
    // {
    //     LiveRange(const Range& r) : range(r){};
    //     LiveRange(Range&& r) : range(std::move(r)){};

    //     Range range;
    //     bool is_on_stack = false;
    //     size_t slot = 0;
    // };

    static constexpr size_t LIVE_NUMBER_STEP = 2;
    static constexpr size_t LINEAR_NUMBER_STEP = 1;

    LivenessAnalysis(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

    Range GetInstLiveRange(IdType inst_id) const;
    size_t GetInstLiveNumber(IdType inst_id) const;
    Range GetBasicBlockLiveRange(IdType inst_id) const;

  private:
    using LiveSet = std::set<InstBase*>;

    static LiveSet Union(const LiveSet& a, const LiveSet& b);

    void Init();
    void CalculateLiveRanges(BasicBlock* bb);
    void InstAddLiveRange(InstBase* inst, const Range& range);
    void CalculateInitialLiveSet(BasicBlock* bb);
    void ResetState();

    std::unordered_map<IdType, size_t> inst_linear_numbers_{};
    std::unordered_map<IdType, size_t> inst_live_numbers_{};
    std::unordered_map<IdType, Range> inst_live_ranges_{};
    std::unordered_map<IdType, Range> bb_live_ranges_{};
    std::unordered_map<IdType, LiveSet> bb_live_sets_{};
};

template <>
struct Pass::PassTraits<LivenessAnalysis>
{
    using is_cfg_sensitive = std::integral_constant<bool, true>;
};

#endif