#include "po.h"
#include "bb.h"
#include "graph.h"

bool PO::RunPass()
{
    ResetStructs();
    RunPass_(graph_->GetStartBasicBlock());
    ClearMarks();
    SetValid(true);

    return true;
}

std::vector<BasicBlock*> PO::GetBlocks()
{
    return po_bb_;
}

void PO::RunPass_(BasicBlock* cur_bb)
{
    auto analyser = graph_->GetAnalyser();
    auto bits = cur_bb->GetBits();

    if (analyser->GetMark<PO, MarkType::VISITED>(*bits)) {
        return;
    }

    analyser->SetMark<PO, MarkType::VISITED>(bits);

    for (const auto succ : cur_bb->GetSuccessors()) {
        RunPass_(succ);
    }

    po_bb_.push_back(cur_bb);
}

void PO::ResetStructs()
{
    po_bb_.clear();
}

void PO::ClearMarks()
{
    auto analyser = graph_->GetAnalyser();
    for (auto& bb : po_bb_) {
        analyser->ClearMark<PO, MarkType::VISITED>(bb->GetBits());
    }
}
