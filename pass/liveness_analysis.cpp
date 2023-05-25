#include "liveness_analysis.h"
#include "ir/bb.h"
#include "ir/graph.h"

#include <list>

bool LivenessAnalysis::Run()
{
    ResetState();

    LinearizeBlocks();
    CheckLinearOrder();

    Init();
    CalculateLiveness();

    SetValid(true);

    return true;
}

bool LivenessAnalysis::AllForwardEdgesVisited(BasicBlock* bb, Markers markers)
{
    if (!bb->IsLoopHeader()) {
        for (auto pred : bb->GetPredecessors()) {
            if (!pred->ProbeMark(&markers[Marks::VISITED])) {
                return false;
            }
        }
    } else {
        // irreducible loops are to be eliminated during loop analysis
        ASSERT(bb->GetLoop()->IsReducible());

        for (auto pred : bb->GetPredecessors()) {
            if (!bb->Dominates(pred) && !pred->ProbeMark(&markers[Marks::VISITED])) {
                return false;
            }
        }
    }

    return true;
}

// while being correct, linear_order.cpp does not meet cte requirements for this liveness
// algorithm, so i need to come up with the new one here
// it should be possible to use different linear order in codegen, as linear reordering does not
// influence value liveness. so here i will use linear order that does not mmet the criteria of
// linear order presented in LinearOrder::Check(), but satisfies criteria of liveness analysis. and
// during codegen i will use already implemented and tested LinearOrder pass
void LivenessAnalysis::LinearizeBlocks()
{
    graph_->GetPassManager()->GetValidPass<DomTree>();
    graph_->GetPassManager()->GetValidPass<LoopAnalysis>();

    Markers markers{};

    std::list<BasicBlock*> queue{ graph_->GetStartBasicBlock() };

    while (!queue.empty()) {
        auto bb = queue.front();
        ASSERT(queue.front() != nullptr);
        queue.pop_front();

        linear_blocks_.push_back(bb);
        ASSERT(!bb->ProbeMark(&markers[Marks::VISITED]));
        bb->SetMark(&markers[Marks::VISITED]);

        auto succs = bb->GetSuccessors();
        for (auto it = succs.rbegin(); it != succs.rend(); ++it) {
            auto succ = *it;

            ASSERT(succ != nullptr);

            if (succ->ProbeMark(&markers[Marks::VISITED]) ||
                !AllForwardEdgesVisited(succ, markers)) {
                continue;
            }

            auto it_before_inner_loop =
                std::find_if(queue.begin(), queue.end(), [succ](BasicBlock* b) {
                    return !b->GetLoop()->Inside(succ->GetLoop());
                });

            queue.insert(it_before_inner_loop, succ);
        }
    }
}

void LivenessAnalysis::CheckLinearOrder()
{
    auto rpo = graph_->GetPassManager()->GetValidPass<RPO>()->GetBlocks();

    ASSERT(rpo.size() == linear_blocks_.size());

    std::vector<unsigned> bb_to_lin_number{};
    bb_to_lin_number.resize(linear_blocks_.size());

    for (unsigned i = 0; i < linear_blocks_.size(); ++i) {
        bb_to_lin_number[linear_blocks_[i]->GetId()] = i;
    }

    for (const auto& bb : linear_blocks_) {
        if (bb->GetImmDominator() != nullptr) {
            ASSERT(bb_to_lin_number[bb->GetImmDominator()->GetId()] <=
                   bb_to_lin_number[bb->GetId()]);
        }
    }
}

void LivenessAnalysis::Init()
{
    unsigned cur_live_number = 0;
    unsigned cur_linear_number = 0;

    for (const auto& bb : linear_blocks_) {
        unsigned bb_start = cur_live_number;

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

        unsigned bb_end = cur_live_number;
        bb_live_ranges_.emplace(bb, Range(bb_start, bb_end));
    }
}

void LivenessAnalysis::CalculateLiveness()
{
    for (auto bb_it = linear_blocks_.rbegin(); bb_it != linear_blocks_.rend(); bb_it++) {
        CalculateLiveRanges(*bb_it);
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
        ASSERT(bb->GetLoop() != nullptr);
        ASSERT(bb->GetLoop()->GetBackEdges().size() == 1);

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
    linear_blocks_.clear();
}

LivenessAnalysis::LiveSet LivenessAnalysis::Union(const LiveSet& a, const LiveSet& b)
{
    LiveSet result = a;
    result.insert(b.begin(), b.end());
    return result;
}
