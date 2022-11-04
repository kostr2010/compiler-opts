
#include "loop_analysis.h"
#include "bb.h"
#include "graph.h"
#include "loop.h"

bool LoopAnalysis::RunPass()
{
    ResetStructs();
    graph_->GetAnalyser()->GetValidPass<DomTree>();
    CollectBackEdges(graph_->GetStartBasicBlock());
    SplitBackEdges();
    RecalculateLoopsReducibility();
    AddPreHeaders();
    PopulateLoops();
    BuildLoopTree();
    ClearMarks();
    SetValid(true);

    return true;
}

void LoopAnalysis::CollectBackEdges(BasicBlock* bb)
{
    auto analyser = graph_->GetAnalyser();
    auto bb_bits = bb->GetBits();

    analyser->SetMark<LoopAnalysis, MarkType::GREY>(bb_bits);
    analyser->SetMark<LoopAnalysis, MarkType::BLACK>(bb_bits);
    id_to_dfs_idx_[bb->GetId()] = id_to_dfs_idx_.size();

    for (const auto& succ : bb->GetSuccessors()) {
        auto succ_bits = *(succ->GetBits());

        if (analyser->GetMark<LoopAnalysis, MarkType::GREY>(succ_bits)) {
            auto loop = succ->GetLoop();
            bool need_new_loop = (loop == nullptr);

            if (need_new_loop) {
                loops_.emplace_back(new Loop(loops_.size(), succ, bb));
                succ->SetLoop(loops_.back().get(), true);
            } else {
                loop->AddBackEdge(bb);
            }
        }

        if (analyser->GetMark<LoopAnalysis, MarkType::BLACK>(succ_bits)) {
            continue;
        }

        CollectBackEdges(succ);
    }

    analyser->ClearMark<LoopAnalysis, MarkType::GREY>(bb_bits);
}

void LoopAnalysis::PopulateLoops()
{
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<PO>()->GetBlocks()) {
        if (bb->IsLoopHeader()) {
            PopulateLoop(bb->GetLoop());
        }
    }
}

void LoopAnalysis::ClearLoopMarks(Loop* loop)
{
    auto analyser = graph_->GetAnalyser();
    for (const auto& bb : loop->GetBlocks()) {
        analyser->ClearMark<LoopAnalysis, MarkType::GREEN>(bb->GetBits());
    }
    analyser->ClearMark<LoopAnalysis, MarkType::GREEN>(loop->GetHeader()->GetBits());
    for (const auto& l : loop->GetInnerLoops()) {
        ClearLoopMarks(l);
    }
}

void LoopAnalysis::PopulateLoop(Loop* loop)
{
    if (loop->IsReducible()) {
        auto analyser = graph_->GetAnalyser();
        analyser->SetMark<LoopAnalysis, MarkType::GREEN>(loop->GetHeader()->GetBits());

        for (const auto& bck : loop->GetBackEdges()) {
            RunLoopSearch(loop, bck);
        }

        ClearLoopMarks(loop);
    } else {
        for (const auto& bck : loop->GetBackEdges()) {
            if (bck->GetLoop() != nullptr) {
                continue;
            }

            bck->SetLoop(loop);
            loop->AddBlock(bck);
        }
    }
}

void LoopAnalysis::RunLoopSearch(Loop* cur_loop, BasicBlock* cur_bb)
{
    auto analyser = graph_->GetAnalyser();
    analyser->SetMark<LoopAnalysis, MarkType::GREEN>(cur_bb->GetBits());

    auto cur_bb_loop = cur_bb->GetLoop();

    if (cur_bb_loop == nullptr) {
        cur_bb->SetLoop(cur_loop);
        cur_loop->AddBlock(cur_bb);
    } else if ((cur_loop->GetId() != cur_bb_loop->GetId()) &&
               cur_bb_loop->GetOuterLoop() == nullptr) {
        cur_bb_loop->SetOuterLoop(cur_loop);
        cur_loop->AddInnerLoop(cur_bb_loop);
    }

    for (const auto& bb : cur_bb->GetPredecesors()) {
        if (!analyser->GetMark<LoopAnalysis, MarkType::GREEN>(*(bb->GetBits()))) {
            RunLoopSearch(cur_loop, bb);
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
    static const auto comparator = [this](BasicBlock* lhs, BasicBlock* rhs) {
        return id_to_dfs_idx_.at(lhs->GetId()) < id_to_dfs_idx_.at(rhs->GetId());
    };

    auto back_edges = loop->GetBackEdges();
    std::sort(back_edges.begin(), back_edges.end(), comparator);
    auto header = loop->GetHeader();

    std::vector<BasicBlock*> pre_headers{};
    for (auto edge = back_edges.begin() + 1; edge != back_edges.end(); ++edge) {
        auto bb = graph_->NewBasicBlock();
        loops_.emplace_back(new Loop(loops_.size(), bb, *edge));
        bb->SetLoop(loops_.back().get(), true);
        (*edge)->ReplaceSucc(header, bb);
        header->RemovePred(*edge);
        pre_headers.push_back(bb);
    }
    header->RemovePred(back_edges.at(0));
    loop->ClearBackEdges();
    loop->AddBackEdge(back_edges.at(0));

    auto prev_head = header;
    for (size_t i = 0; i < pre_headers.size(); ++i) {
        graph_->InsertBasicBlockBefore(pre_headers.at(i), prev_head);
        graph_->AddEdge(back_edges.at(i), prev_head);
        prev_head = pre_headers.at(i);
    }
    graph_->AddEdge(back_edges.back(), prev_head);
}

void LoopAnalysis::AddPreHeaders()
{
    for (auto loop = loops_.begin() + 1; loop != loops_.end(); ++loop) {
        if (!loop->get()->IsReducible()) {
            continue;
        }
        AddPreHeader(loop->get());
    }
}

void LoopAnalysis::AddPreHeader(Loop* loop)
{
    auto head = loop->GetHeader();
    assert(head != nullptr);

    for (const auto& bck : loop->GetBackEdges()) {
        head->RemovePred(bck);
    }

    auto bb = graph_->NewBasicBlock();
    graph_->InsertBasicBlockBefore(bb, head);
    loop->SetPreHeader(bb);

    for (const auto& bck : loop->GetBackEdges()) {
        head->AddPred(bck);
    }
}

void LoopAnalysis::PopulateRootLoop()
{
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
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

void LoopAnalysis::ResetStructs()
{
    id_to_dfs_idx_.clear();
    loops_.clear();
    InitStartLoop();
}

void LoopAnalysis::ClearMarks()
{
    auto analyser = graph_->GetAnalyser();

    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
        auto bits = bb->GetBits();
        analyser->ClearMark<LoopAnalysis, MarkType::GREY>(bits);
        analyser->ClearMark<LoopAnalysis, MarkType::BLACK>(bits);
        analyser->ClearMark<LoopAnalysis, MarkType::GREEN>(bits);
    }
}

void LoopAnalysis::InitStartLoop()
{
    assert(loops_.empty());

    loops_.emplace_back(new Loop(ROOT_LOOP_ID));
}

void LoopAnalysis::RecalculateLoopsReducibility()
{
    auto analyser = graph_->GetAnalyser();

    if (analyser->GetPass<DomTree>()->GetValid()) {
        return;
    }

    analyser->RunPass<DomTree>();

    for (const auto& loop : loops_) {
        loop->RecalculateReducibility();
    }
}