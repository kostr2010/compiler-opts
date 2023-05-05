#include "linear_scan.h"

#include "ir/bb.h"
#include "ir/graph.h"
#include "ir/inst.h"

#include <algorithm>

void LinearScan::LiveRange::AssignRegister(size_t s)
{
    slot = s;
    loc = Location::REGISTER;
}

void LinearScan::LiveRange::AssignStackSlot(size_t s)
{
    slot = s;
    loc = Location::STACK;
}

LinearScan::LinearScan(Graph* g)
{
    assert(g != nullptr);

    for (const auto& e :
         g->GetPassManager()->GetValidPass<LivenessAnalysis>()->GetInstLiveRanges()) {
        if (e.first->GetNumUsers() != 0) {
            ranges_.emplace_back(e.first, e.second);
        }
    }

    std::sort(ranges_.begin(), ranges_.end(), [](const LiveRange& l, const LiveRange& r) {
        return l.range.GetStart() < r.range.GetStart();
    });
}

bool LinearScan::Run()
{
    ResetState();
    LinearScanRegisterAllocation();

    return true;
}

void LinearScan::LinearScanRegisterAllocation()
{
    for (auto& range : ranges_) {
        ExpireOldIntervals(range);
        if (IsRegMapEmpty()) {
            SpillAtInterval(&range);
        } else {
            AssignRegister(&range);
            AddToActive(&range);
        }
    }
}

void LinearScan::ExpireOldIntervals(const LiveRange& range)
{
    for (const auto& r : active_) {
        if (r->range.GetEnd() > range.range.GetStart()) {
            return;
        }
        RemoveFromActive(r);
        ReleaseRegister(*r);
    }
}

void LinearScan::SpillAtInterval(LiveRange* r)
{
    assert(r != nullptr);

    if (active_.back()->range.GetEnd() < r->range.GetEnd()) {
        AssignStackSlot(r);
    } else {
        auto slot = AssignStackSlot(active_.back());
        assert(active_.back()->loc == LiveRange::Location::STACK);
        r->AssignRegister(slot);
        active_.pop_back();
    }
}

size_t LinearScan::AssignStackSlot(LiveRange* r)
{
    assert(r != nullptr);
    assert(r->loc == LiveRange::Location::REGISTER);

    auto prev_slot = r->slot;
    r->AssignStackSlot(GetStackSlot());
    return prev_slot;
}

void LinearScan::AssignRegister(LiveRange* r)
{
    assert(r != nullptr);
    assert(!IsRegMapEmpty());

    for (size_t i = 0; i < reg_map_.size(); ++i) {
        if (reg_map_[i] == false) {
            r->AssignRegister(i);
            return;
        }
    }
}

void LinearScan::ReleaseRegister(const LiveRange& r)
{
    assert(r.loc == LiveRange::Location::REGISTER);
    reg_map_[r.slot] = false;
}

void LinearScan::RemoveFromActive(LiveRange* r)
{
    assert(r != nullptr);
    active_.erase(std::remove(active_.begin(), active_.end(), r), active_.end());
}

void LinearScan::AddToActive(LiveRange* r)
{
    assert(r != nullptr);
    auto it = std::find_if(active_.begin(), active_.end(),
                           [r](LiveRange* i) { return r->range.GetEnd() <= i->range.GetEnd(); });
    active_.insert(it, r);
}

bool LinearScan::IsRegMapEmpty() const
{
    return active_.size() == reg_map_.size();
}

size_t LinearScan::GetStackSlot()
{
    return current_stack_slot++;
}

void LinearScan::ResetState()
{
    current_stack_slot = 0;
    active_.clear();
}