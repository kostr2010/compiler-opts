#ifndef __REGALLOC_LINEAR_SCAN_H_INCLUDED__
#define __REGALLOC_LINEAR_SCAN_H_INCLUDED__

#include "arch/arch_info.h"
#include "pass/liveness_analysis.h"
#include "utils/range/range.h"

#include <list>
#include <vector>

class LinearScan
{
  public:
    struct LiveRange
    {
        enum class Location
        {
            UNSET,
            REGISTER,
            STACK,
        };

        LiveRange(InstBase* i, const Range& r) : range(r), inst(i){};
        LiveRange(InstBase* i, Range&& r) : range(std::move(r)), inst(i){};

        void AssignRegister(size_t s);
        void AssignStackSlot(size_t s);

        DEFAULT_COPY_SEMANTIC(LiveRange);
        DEFAULT_MOVE_SEMANTIC(LiveRange);
        DEFAULT_DTOR(LiveRange);

        Range range;
        InstBase* inst;
        Location loc{ Location::UNSET };
        size_t slot{ 0 };
    };

    LinearScan(Graph* g);
    bool Run();

  private:
    void LinearScanRegisterAllocation();
    void ExpireOldIntervals(const LiveRange& range);
    void SpillAtInterval(LiveRange* r);
    size_t AssignStackSlot(LiveRange* r);
    void AssignRegister(LiveRange* r);
    void ReleaseRegister(const LiveRange& r);
    void RemoveFromActive(LiveRange* r);
    void AddToActive(LiveRange* r);
    bool IsRegMapEmpty() const;
    size_t GetStackSlot();
    void ResetState();

    std::vector<LiveRange> ranges_{};
    std::list<LiveRange*> active_{};
    // FIXME: dynamic register information
    std::array<bool, arch::ArchInfo<arch::Arch::X86_64>::NUM_REGISTERS> reg_map_{};

    size_t current_stack_slot{ 0 };
};

#endif