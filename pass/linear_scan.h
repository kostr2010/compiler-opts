#ifndef __REGALLOC_LINEAR_SCAN_H_INCLUDED__
#define __REGALLOC_LINEAR_SCAN_H_INCLUDED__

#include "arch/arch_info.h"
#include "ir/inst.h"
#include "liveness_analysis.h"
#include "pass.h"
#include "utils/range/range.h"

#include <list>
#include <unordered_map>
#include <vector>

class LinearScan : public Pass
{
  public:
    struct LiveRange
    {
        LiveRange(InstBase* i, const Range& r) : range(r), inst(i){};
        LiveRange(InstBase* i, Range&& r) : range(std::move(r)), inst(i){};

        DEFAULT_COPY_SEMANTIC(LiveRange);
        DEFAULT_MOVE_SEMANTIC(LiveRange);
        DEFAULT_DTOR(LiveRange);

        Range range;
        InstBase* inst;
    };

    LinearScan(Graph* g);
    bool Run() override;

    template <arch::Arch ARCH>
    void SetArch()
    {
        reg_map_.resize(arch::ArchInfo<ARCH>::NUM_REGISTERS);
    }

    auto GetLiveRanges() const
    {
        return ranges_;
    }

    auto GetMoveMap() const
    {
        return move_map_;
    }

  private:
    void LinearScanRegisterAllocation();
    void InsertConnectingSpillFills();
    void ExpireOldIntervals(LiveRange* range);
    void SpillAtInterval(LiveRange* r);
    size_t AssignStackSlot(LiveRange* r);
    void AssignRegister(LiveRange* r);
    void ReleaseRegister(LiveRange* r);
    void AddToActive(LiveRange* r);
    bool IsRegMapEmpty() const;
    size_t GetStackSlot();
    void Init();
    void Check() const;

    // insert move instruction from pair.first to pair.second at the end of the bb during codegen
    using Move = std::pair<Location, Location>;
    std::unordered_map<BasicBlock*, std::vector<Move> > move_map_{};

    std::vector<LiveRange> ranges_{};
    std::list<LiveRange*> active_{};
    std::vector<bool> reg_map_{};

    size_t current_stack_slot{ 0 };
};

#endif