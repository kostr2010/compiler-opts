#include "liveness_analysis.h"
#include "ir/bb.h"
#include "ir/graph.h"

bool LivenessAnalysis::RunPass()
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

Range LivenessAnalysis::GetInstLiveRange(IdType inst_id) const
{
    return inst_live_ranges_.at(inst_id);
}

size_t LivenessAnalysis::GetInstLiveNumber(IdType inst_id) const
{
    return inst_live_numbers_.at(inst_id);
}

Range LivenessAnalysis::GetBasicBlockLiveRange(IdType inst_id) const
{
    return bb_live_ranges_.at(inst_id);
}

void LivenessAnalysis::Init()
{
    size_t cur_live_number = 0;
    size_t cur_linear_number = 0;

    for (const auto& bb : graph_->GetPassManager()->GetValidPass<LinearOrder>()->GetBlocks()) {
        size_t bb_start = cur_live_number;

        for (auto phi = bb->GetFirstPhi(); phi != nullptr; phi = phi->GetNext()) {
            inst_linear_numbers_[phi->GetId()] = cur_linear_number;
            inst_live_numbers_[phi->GetId()] = cur_live_number;

            cur_linear_number += LINEAR_NUMBER_STEP;

            LOG("PHI:  " << phi->GetId() << ", LIVE_NUM: " << inst_live_numbers_[phi->GetId()]
                         << ", LIN_NUM: " << inst_linear_numbers_[phi->GetId()]);
        }

        cur_live_number += LIVE_NUMBER_STEP;

        for (auto inst = bb->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
            inst_linear_numbers_[inst->GetId()] = cur_linear_number;
            inst_live_numbers_[inst->GetId()] = cur_live_number;

            cur_linear_number += LINEAR_NUMBER_STEP;
            cur_live_number += LIVE_NUMBER_STEP;
            LOG("INST: " << inst->GetId() << ", LIVE_NUM: " << inst_live_numbers_[inst->GetId()]
                         << ", LIN_NUM: " << inst_linear_numbers_[inst->GetId()]);
        }

        size_t bb_end = cur_live_number;
        LOG("BB: " << bb->GetId() << ", LIVE_RANGE: " << Range(bb_start, bb_end));
        bb_live_ranges_.emplace(bb->GetId(), Range(bb_start, bb_end));
    }
}

void LivenessAnalysis::CalculateLiveRanges(BasicBlock* bb)
{
    auto bb_id = bb->GetId();

    CalculateInitialLiveSet(bb);

    auto range = bb_live_ranges_.at(bb_id);
    for (const auto& i : bb_live_sets_.at(bb_id)) {
        InstAddLiveRange(i, range);
    }

    for (auto i = bb->GetLastInst(); i != nullptr; i = i->GetPrev()) {
        auto i_id = i->GetId();
        auto i_live_num = inst_live_numbers_[i_id];

        if (inst_live_ranges_.count(i_id) == 0) {
            inst_live_ranges_.emplace(i_id, Range(i_live_num, i_live_num + LIVE_NUMBER_STEP));
        } else {
            inst_live_ranges_.at(i_id).SetStart(i_live_num);
        }

        bb_live_sets_.at(bb_id).erase(i);

        for (const auto& input : i->GetInputs()) {
            bb_live_sets_.at(bb_id).insert(input.GetInst());
            InstAddLiveRange(input.GetInst(), Range(range.GetStart(), i_live_num));
        }
    }

    for (auto phi = bb->GetFirstPhi(); phi != nullptr; phi = phi->GetNext()) {
        bb_live_sets_.at(bb_id).erase(phi);
    }

    if (bb->IsLoopHeader()) {
        assert(bb->GetLoop() != nullptr);
        assert(bb->GetLoop()->GetBackEdges().size() == 1);

        auto bck_id = bb->GetLoop()->GetBackEdges().front()->GetId();

        for (const auto& i : bb_live_sets_.at(bb_id)) {
            auto start = range.GetStart();
            auto end = bb_live_ranges_.at(bck_id).GetEnd();
            InstAddLiveRange(i, Range(start, end));
        }
    }
}

void LivenessAnalysis::InstAddLiveRange(InstBase* inst, const Range& range)
{
    if (inst_live_ranges_.count(inst->GetId()) == 0) {
        inst_live_ranges_.emplace(inst->GetId(), range);
    } else {
        inst_live_ranges_.at(inst->GetId()) =
            Range::Union(inst_live_ranges_.at(inst->GetId()), range);
    }
}

void LivenessAnalysis::CalculateInitialLiveSet(BasicBlock* bb)
{
    LiveSet set = {};
    for (const auto& succ : bb->GetSuccessors()) {
        set = Union(set, bb_live_sets_[succ->GetId()]);

        for (auto phi = succ->GetFirstPhi(); phi != nullptr; phi = phi->GetNext()) {
            for (const auto& input : phi->GetInputs()) {
                if (input.GetSourceBB()->GetId() == bb->GetId()) {
                    set.insert(input.GetInst());
                }
            }
        }
    }
    bb_live_sets_[bb->GetId()] = set;
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
