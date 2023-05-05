
#include "loop_analysis.h"
#include "ir/bb.h"
#include "ir/graph.h"
#include "ir/loop.h"
#include "utils/marker/marker_factory.h"

#include <algorithm>
#include <set>

bool LoopAnalysis::Run()
{
    ResetState();
    graph_->GetPassManager()->GetValidPass<DomTree>();

    MarkersBckEdges markers_bck = { marker::MarkerFactory::AcquireMarker(),
                                    marker::MarkerFactory::AcquireMarker() };

    CollectBackEdges(graph_->GetStartBasicBlock(), markers_bck);
    SplitBackEdges();
    RecalculateLoopsReducibility();
    AddPreHeaders();
    PopulateLoops();
    BuildLoopTree();
    SetValid(true);

    Check();

    return true;
}

void LoopAnalysis::CollectBackEdges(BasicBlock* bb, const MarkersBckEdges markers)
{
    bb->SetMark(&markers[MarksBckEdges::GREY]);
    bb->SetMark(&markers[MarksBckEdges::BLACK]);

    id_to_dfs_idx_[bb->GetId()] = id_to_dfs_idx_.size();

    for (const auto& succ : bb->GetSuccessors()) {
        if (succ->ProbeMark(&markers[MarksBckEdges::GREY])) {
            auto loop = succ->GetLoop();
            bool need_new_loop = (loop == nullptr);

            if (need_new_loop) {
                loops_.emplace_back(new Loop(loops_.size(), succ, bb));
                succ->SetLoop(loops_.back().get(), true);
            } else {
                loop->AddBackEdge(bb);
            }
        }

        if (succ->ProbeMark(&markers[MarksBckEdges::BLACK])) {
            continue;
        }

        CollectBackEdges(succ, markers);
    }

    bb->ClearMark(&markers[MarksBckEdges::GREY]);
}

void LoopAnalysis::PopulateLoops()
{
    for (const auto& bb : graph_->GetPassManager()->GetValidPass<PO>()->GetBlocks()) {
        if (bb->IsLoopHeader()) {
            PopulateLoop(bb->GetLoop());
        }
    }
}

void LoopAnalysis::PopulateLoop(Loop* loop)
{
    if (loop->IsReducible()) {
        MarkersPopulate markers = { marker::MarkerFactory::AcquireMarker() };

        loop->GetHeader()->SetMark(&markers[MarksPopulate::GREEN]);

        for (const auto& bck : loop->GetBackEdges()) {
            RunLoopSearch(loop, bck, markers);
        }
    } else {
        assert(loop->GetHeader() != nullptr);
        // assert(loop->GetOuterLoop() != nullptr);
        // auto header = loop->GetHeader();
        loop->GetHeader()->SetLoop(loop->GetOuterLoop());
        // for (const auto& bck : loop->GetBackEdges()) {
        //     if (bck->GetLoop() == nullptr) {
        //         bck->SetLoop(loop);
        //     }

        //     loop->AddBlock(bck);
        // }
    }
}

void LoopAnalysis::RunLoopSearch(Loop* cur_loop, BasicBlock* cur_bb, const MarkersPopulate markers)
{
    cur_bb->SetMark(&markers[MarksPopulate::GREEN]);

    auto cur_bb_loop = cur_bb->GetLoop();

    if (cur_bb_loop == nullptr) {
        cur_bb->SetLoop(cur_loop);
        cur_loop->AddBlock(cur_bb);
    } else if ((cur_loop->GetId() != cur_bb_loop->GetId()) &&
               cur_bb_loop->GetOuterLoop() == nullptr) {
        cur_bb_loop->SetOuterLoop(cur_loop);
        cur_loop->AddInnerLoop(cur_bb_loop);
    }

    for (const auto& bb : cur_bb->GetPredecessors()) {
        if (!bb->ProbeMark(&markers[MarksPopulate::GREEN])) {
            RunLoopSearch(cur_loop, bb, markers);
        }
    }
}

void LoopAnalysis::SplitBackEdges()
{
    for (const auto& loop : loops_) {
        if (!loop->IsReducible()) {
            continue;
        }

        if (loop->GetBackEdges().size() > 1) {
            SplitBackEdge(loop.get());
        }
    }
}

void LoopAnalysis::SplitBackEdge(Loop* loop)
{
    auto back_edges = loop->GetBackEdges();

    std::sort(back_edges.begin(), back_edges.end(), [this](BasicBlock* lhs, BasicBlock* rhs) {
        assert(lhs != nullptr);
        assert(rhs != nullptr);
        return id_to_dfs_idx_.at(lhs->GetId()) < id_to_dfs_idx_.at(rhs->GetId());
    });

    auto header = loop->GetHeader();

    std::vector<BasicBlock*> new_headers{};
    for (auto edge = back_edges.begin() + 1; edge != back_edges.end(); ++edge) {
        auto bb = graph_->NewBasicBlock();
        loops_.emplace_back(new Loop(loops_.size(), bb, *edge));
        bb->SetLoop(loops_.back().get(), true);
        new_headers.push_back(bb);
    }
    loop->ClearBackEdges();
    loop->AddBackEdge(back_edges.at(0));

    auto prev_head = header;
    for (size_t i = 0; i < new_headers.size(); ++i) {
        graph_->InsertBasicBlockBefore(new_headers.at(i), prev_head);
        graph_->ReplaceSuccessor(back_edges.at(i), new_headers.at(i), prev_head);
        PropagatePhis(prev_head, new_headers.at(i));
        prev_head = new_headers.at(i);
    }
}

void LoopAnalysis::AddPreHeaders()
{
    for (auto loop = loops_.begin() + 1; loop != loops_.end(); ++loop) {
        if (!loop->get()->IsReducible()) {
            continue;
        }
        AddPreHeader(loop->get());
        PropagatePhis(loop->get()->GetHeader(), loop->get()->GetPreHeader());
    }
}

void LoopAnalysis::AddPreHeader(Loop* loop)
{
    assert(loop != nullptr);
    auto head = loop->GetHeader();
    assert(head != nullptr);

    std::vector<BasicBlock*> pred{};

    auto bck = loop->GetBackEdges();
    for (const auto& bb : head->GetPredecessors()) {
        if (std::find(bck.begin(), bck.end(), bb) == bck.end()) {
            pred.push_back(bb);
        }
    }

    assert(!pred.empty());

    auto preheader = graph_->NewBasicBlock();
    loop->SetPreHeader(preheader);

    for (const auto& bb : pred) {
        graph_->ReplaceSuccessor(bb, head, preheader);
    }
    graph_->AddEdge(preheader, head, Conditional::Branch::FALLTHROUGH);
}

void LoopAnalysis::PropagatePhis(BasicBlock* bb, BasicBlock* pred)
{
    assert(bb != nullptr);
    assert(pred != nullptr);
    assert(bb->GetLoop() != nullptr);
    assert(bb->GetLoop()->GetBackEdges().size() == 1);
    assert(bb->Succeeds(pred));
    assert(pred->Precedes(bb));

    auto loop = bb->GetLoop();
    auto bck = loop->GetBackEdges().front();

    for (auto i = bb->GetFirstPhi(); i != nullptr; i = i->GetNext()) {
        assert(i->IsPhi());
        auto phi = static_cast<isa::inst::Inst<isa::inst::Opcode::PHI>::Type*>(i);
        auto inputs = phi->GetInputs();
        auto it = std::find_if(inputs.begin(), inputs.end(), [bck](const Input& in) {
            return in.GetSourceBB()->GetId() == bck->GetId();
        });

        if (it != phi->GetInputs().end()) {
            auto bck_input = *it;
            phi->RemoveInput(bck_input);
            bck_input.GetInst()->RemoveUser(User(phi));

            inputs = phi->GetInputs();
            for (const auto& in : inputs) {
                in.GetInst()->RemoveUser(phi);
            }
            phi->ClearInputs();

            InstBase* source_inst = nullptr;
            if (inputs.size() > 1) {
                pred->PushBackPhi(InstBase::NewInst<isa::inst::Opcode::PHI>());
                source_inst = pred->GetLastPhi();
                for (const auto& input : inputs) {
                    source_inst->AddInput(input);
                }
            } else {
                source_inst = inputs.front().GetInst();
            }

            phi->AddInput(bck_input);
            phi->AddInput(source_inst, pred);
        }
    }
}

void LoopAnalysis::PopulateRootLoop()
{
    for (const auto& bb : graph_->GetPassManager()->GetValidPass<RPO>()->GetBlocks()) {
        if (bb->GetLoop() == nullptr) {
            GetRootLoop()->AddBlock(bb);
            bb->SetLoop(GetRootLoop());
        }
    }
}

void LoopAnalysis::BuildLoopTree()
{
    PopulateRootLoop();

    for (auto loop = loops_.begin() + 1; loop != loops_.end(); ++loop) {
        if (loop->get()->GetOuterLoop() == nullptr) {
            loop->get()->SetOuterLoop(GetRootLoop());
            GetRootLoop()->AddInnerLoop(loop->get());
        }
    }
}

void LoopAnalysis::ResetState()
{
    id_to_dfs_idx_.clear();
    loops_.clear();
    InitStartLoop();
}

void LoopAnalysis::InitStartLoop()
{
    assert(loops_.empty());

    loops_.emplace_back(new Loop(ROOT_LOOP_ID));
}

void LoopAnalysis::RecalculateLoopsReducibility()
{
    auto analyser = graph_->GetPassManager();

    if (analyser->GetPass<DomTree>()->GetValid()) {
        return;
    }

    analyser->Run<DomTree>();

    for (const auto& loop : loops_) {
        loop->CalculateReducibility();
    }
}

void LoopAnalysis::Check()
{
    std::set<IdType> bbs{};

    for (const auto& loop : loops_) {
        if (!loop->IsReducible()) {
            continue;
        }

        if (!loop->IsRoot()) {
            assert(!bbs.contains(loop->GetHeader()->GetId()));
            assert(loop->GetId() == loop->GetHeader()->GetLoop()->GetId());
            bbs.insert(loop->GetHeader()->GetId());
        }

        for (const auto& bb : loop->GetBlocks()) {
            assert(!bbs.contains(bb->GetId()));
            assert(bb->GetLoop()->GetId() == loop->GetId());
            bbs.insert(bb->GetId());
        }
    }

    auto rpo = graph_->GetPassManager()->GetValidPass<RPO>()->GetBlocks();
    assert(bbs.size() == rpo.size());

    for (const auto& bb : rpo) {
        assert(bb->GetLoop() != nullptr);
        assert(bb->GetLoop()->IsReducible());
    }
}
