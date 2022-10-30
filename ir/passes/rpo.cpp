#include "rpo.h"
#include "bb.h"
#include "graph.h"

bool RPO::RunPass()
{
    rpo_bb_.clear();

    RunPass_(graph_->GetStartBasicBlock());

    auto analyser = graph_->GetAnalyser();
    for (auto& bb : rpo_bb_) {
        analyser->ClearMark<RPO, MarkType::VISITED>(bb->GetBits());
    }

    SetValid(true);

    return true;
}

std::vector<BasicBlock*> RPO::GetBlocks()
{
    return rpo_bb_;
}

void RPO::RunPass_(BasicBlock* cur_bb)
{
    auto analyser = graph_->GetAnalyser();
    auto bits = cur_bb->GetBits();

    if (analyser->GetMark<RPO, MarkType::VISITED>(*bits)) {
        return;
    }

    analyser->SetMark<RPO, MarkType::VISITED>(bits);
    rpo_bb_.push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        RunPass_(succ);
    }
}