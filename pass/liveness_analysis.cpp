#include "liveness_analysis.h"
#include "ir/bb.h"
#include "ir/graph.h"

bool LivenessAnalysis::Run()
{
    ResetState();
    Init();

    auto lin_order = graph_->GetPassManager()->GetValidPass<LinearOrder>()->GetBlocks();

    for (auto bb_it = lin_order.rbegin(); bb_it != lin_order.rend(); bb_it++) {
        CalculateLiveRanges(*bb_it);
    }

    SetValid(true);

    return true;
}

void LivenessAnalysis::Init()
{
    size_t cur_live_number = 0;
    size_t cur_linear_number = 0;

    for (const auto& bb : graph_->GetPassManager()->GetValidPass<LinearOrder>()->GetBlocks()) {
        size_t bb_start = cur_live_number;

        for (auto phi = bb->GetFirstPhi(); phi != nullptr; phi = phi->GetNext()) {
            inst_linear_numbers_[phi] = cur_linear_number;
            inst_live_numbers_[phi] = cur_live_number;

            cur_linear_number += LINEAR_NUMBER_STEP;
        }

        cur_live_number += LIVE_NUMBER_STEP;

        for (auto inst = bb->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
            inst_linear_numbers_[inst] = cur_linear_number;
            inst_live_numbers_[inst] = cur_live_number;

            cur_linear_number += LINEAR_NUMBER_STEP;
            cur_live_number += LIVE_NUMBER_STEP;
        }

        size_t bb_end = cur_live_number;
        bb_live_ranges_.emplace(bb, Range(bb_start, bb_end));
    }
}

void LivenessAnalysis::CalculateLiveRanges(BasicBlock* bb)
{
    CalculateInitialLiveSet(bb);

    auto range = bb_live_ranges_.at(bb);
    for (const auto& i : bb_live_sets_.at(bb)) {
        InstAddLiveRange(i, range);
    }

    for (auto i = bb->GetLastInst(); i != nullptr; i = i->GetPrev()) {
        auto i_live_num = inst_live_numbers_[i];

        if (inst_live_ranges_.count(i) == 0) {
            inst_live_ranges_.emplace(i, Range(i_live_num, i_live_num + LIVE_NUMBER_STEP));
        } else {
            inst_live_ranges_.at(i).SetStart(i_live_num);
        }

        bb_live_sets_.at(bb).erase(i);

        for (const auto& input : i->GetInputs()) {
            bb_live_sets_.at(bb).insert(input.GetInst());
            InstAddLiveRange(input.GetInst(), Range(range.GetStart(), i_live_num));
        }
    }

    for (auto phi = bb->GetFirstPhi(); phi != nullptr; phi = phi->GetNext()) {
        bb_live_sets_.at(bb).erase(phi);
    }

    if (bb->IsLoopHeader()) {
        assert(bb->GetLoop() != nullptr);
        assert(bb->GetLoop()->GetBackEdges().size() == 1);

        auto bck = bb->GetLoop()->GetBackEdges().front();

        for (const auto& i : bb_live_sets_.at(bb)) {
            auto start = range.GetStart();
            auto end = bb_live_ranges_.at(bck).GetEnd();
            InstAddLiveRange(i, Range(start, end));
        }
    }
}

void LivenessAnalysis::InstAddLiveRange(InstBase* inst, const Range& range)
{
    if (inst_live_ranges_.count(inst) == 0) {
        inst_live_ranges_.emplace(inst, range);
    } else {
        inst_live_ranges_.at(inst) = Range::Union(inst_live_ranges_.at(inst), range);
    }
}

void LivenessAnalysis::CalculateInitialLiveSet(BasicBlock* bb)
{
    LiveSet set = {};
    for (const auto& succ : bb->GetSuccessors()) {
        set = Union(set, bb_live_sets_[succ]);

        for (auto phi = succ->GetFirstPhi(); phi != nullptr; phi = phi->GetNext()) {
            for (const auto& input : phi->GetInputs()) {
                if (input.GetSourceBB()->GetId() == bb->GetId()) {
                    set.insert(input.GetInst());
                }
            }
        }
    }
    bb_live_sets_[bb] = set;
}

void LivenessAnalysis::ResetState()
{
    inst_linear_numbers_.clear();
    inst_live_numbers_.clear();
    inst_live_ranges_.clear();
    bb_live_ranges_.clear();
    bb_live_sets_.clear();
}

LivenessAnalysis::LiveSet LivenessAnalysis::Union(const LiveSet& a, const LiveSet& b)
{
    LiveSet result = a;
    result.insert(b.begin(), b.end());
    return result;
}
