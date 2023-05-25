#include "linear_scan.h"

#include "ir/bb.h"
#include "ir/graph.h"
#include "ir/inst.h"

#include <algorithm>

LinearScan::LinearScan(Graph* g) : Pass(g)
{
    ASSERT(g != nullptr);
}

bool LinearScan::Run()
{
    Init();
    LinearScanRegisterAllocation();
    InsertConnectingSpillFills();
    Check();

    return true;
}

void LinearScan::Init()
{
    current_stack_slot = 0;
    active_.clear();
    ranges_.clear();
    move_map_.clear();

    std::fill(reg_map_.begin(), reg_map_.end(), false);

    for (const auto& e :
         graph_->GetPassManager()->GetValidPass<LivenessAnalysis>()->GetInstLiveRanges()) {
        if (!e.first->HasFlag<isa::flag::Type::NO_USE>()) {
            ranges_.emplace_back(e.first, e.second);
        }
    }

    std::sort(ranges_.begin(), ranges_.end(), [](const LiveRange& l, const LiveRange& r) {
        return l.range.GetStart() < r.range.GetStart();
    });
}

void LinearScan::LinearScanRegisterAllocation()
{
    for (auto& range : ranges_) {
        ExpireOldIntervals(&range);
        if (IsRegMapEmpty()) {
            SpillAtInterval(&range);
        } else {
            AssignRegister(&range);
            AddToActive(&range);
        }
    }
}

static bool IsCriticalEdge(BasicBlock* from, BasicBlock* to)
{
    return (from->GetNumSuccessors() > 1) && (to->GetNumPredecessors() > 1);
}

static BasicBlock* FixCriticalEdge(Graph* g, InstBase* phi, unsigned input_idx)
{
    auto input = phi->GetInput(input_idx);

    auto bb_phi = phi->GetBasicBlock();
    auto bb_input = input.GetSourceBB();

    ASSERT(IsCriticalEdge(bb_input, bb_phi));

    auto bb_fix = g->NewBasicBlock();
    g->InsertBasicBlock(bb_fix, bb_input, bb_phi);

    phi->SetInput(input_idx, input.GetInst(), bb_fix);

    return bb_fix;
}

void LinearScan::InsertConnectingSpillFills()
{
    for (const auto& r : ranges_) {
        if (!r.inst->IsPhi()) {
            continue;
        }

        auto phi = r.inst;

        unsigned input_idx = 0;
        for (const auto& in : phi->GetInputs()) {
            auto input = in.GetInst();
            auto bb_input = in.GetSourceBB();
            if (input->GetLocation() == phi->GetLocation()) {
                continue;
            }

            auto move_bb = bb_input;
            if (IsCriticalEdge(bb_input, phi->GetBasicBlock())) {
                move_bb = FixCriticalEdge(graph_, phi, input_idx);
            }

            move_map_[move_bb].emplace_back(input->GetLocation(), phi->GetLocation());

            ++input_idx;
        }
    }
}

void LinearScan::ExpireOldIntervals(LiveRange* range)
{
    auto it_to = active_.begin();
    for (auto it = active_.begin(); it != active_.end(); it++) {
        ASSERT(*it != nullptr);
        auto r = *it;
        it_to = it;
        if (range->range.GetStart() < r->range.GetEnd()) {
            break;
        }
        ReleaseRegister(r);
    }

    active_.erase(active_.begin(), it_to);
}

void LinearScan::SpillAtInterval(LiveRange* r)
{
    ASSERT(r != nullptr);

    if (active_.back()->range.GetEnd() > r->range.GetEnd()) {
        ASSERT(active_.back()->inst->GetLocation().IsOnRegister());
        auto slot = AssignStackSlot(active_.back());
        ASSERT(active_.back()->inst->GetLocation().IsOnStack());
        r->inst->SetLocation(Location::Where::REGISTER, slot);
        active_.pop_back();
        AddToActive(r);
    } else {
        AssignStackSlot(r);
    }
}

unsigned LinearScan::AssignStackSlot(LiveRange* r)
{
    ASSERT(r != nullptr);

    auto prev_slot = r->inst->GetLocation().slot;
    r->inst->SetLocation(Location::Where::STACK, GetStackSlot());
    return prev_slot;
}

void LinearScan::AssignRegister(LiveRange* r)
{
    ASSERT(r != nullptr);
    ASSERT(!IsRegMapEmpty());

    for (unsigned i = 0; i < reg_map_.size(); ++i) {
        if (reg_map_[i] == false) {
            reg_map_[i] = true;
            r->inst->SetLocation(Location::Where::REGISTER, i);
            return;
        }
    }
}

void LinearScan::ReleaseRegister(LiveRange* r)
{
    ASSERT(r != nullptr);
    ASSERT(r->inst->GetLocation().IsOnRegister());
    reg_map_[r->inst->GetLocation().slot] = false;
}

void LinearScan::AddToActive(LiveRange* r)
{
    ASSERT(r != nullptr);
    auto it = std::find_if(active_.begin(), active_.end(),
                           [r](LiveRange* i) { return r->range.GetEnd() < i->range.GetEnd(); });
    active_.insert(it, r);
}

bool LinearScan::IsRegMapEmpty() const
{
    return active_.size() == reg_map_.size();
}

unsigned LinearScan::GetStackSlot()
{
    return current_stack_slot++;
}

void LinearScan::Check() const
{
#ifndef RELEASE_BUILD
    for (const auto& bb : graph_->GetPassManager()->GetValidPass<RPO>()->GetBlocks()) {
        for (auto phi = bb->GetFirstPhi(); phi != nullptr; phi = phi->GetNext()) {
            for (const auto& i : phi->GetInputs()) {
                auto input = i.GetInst();
                auto bb_input = i.GetSourceBB();

                ASSERT(bb_input->Precedes(bb));
                ASSERT(bb->Succeeds(bb_input));

                std::vector<Move> moves{};
                if (move_map_.count(bb_input) != 0) {
                    moves = move_map_.at(bb_input);
                }

                auto it = std::find_if(moves.begin(), moves.end(), [input](const auto& pair) {
                    return pair.first == input->GetLocation();
                });

                if (it != moves.end()) {
                    ASSERT(input->GetLocation() == it->first);
                    ASSERT(phi->GetLocation() == it->second);
                } else {
                    ASSERT(phi->GetLocation() == input->GetLocation());
                }
            }
        }
    }
#endif
}
