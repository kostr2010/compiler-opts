#include "rpo.h"
#include "bb.h"
#include "graph.h"

bool RPO::RunPass()
{
    rpo_bb_.clear();

    RunPass_(graph_->GetStartBasicBlock());

    for (auto& bb : rpo_bb_) {
        bb->ResetBits();
    }

    return true;
}

std::vector<BasicBlock*> RPO::GetBlocks()
{
    return rpo_bb_;
}

void RPO::RunPass_(BasicBlock* cur_bb)
{
    auto bits = cur_bb->GetBits();

    if (BbVisited::Get(*bits)) {
        return;
    }

    BbVisited::Set(bits);
    rpo_bb_.push_back(cur_bb);

    for (const auto succ : cur_bb->GetSuccessors()) {
        RunPass_(succ);
    }
}