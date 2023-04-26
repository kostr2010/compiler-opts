#include "graph.h"
#include "bb.h"

BasicBlock* Graph::GetStartBasicBlock() const
{
    return bb_vector_[BB_START_ID].get();
}

BasicBlock* Graph::GetBasicBlock(IdType bb_id) const
{
    assert(bb_id < bb_vector_.size());
    return bb_vector_.at(bb_id).get();
}

void Graph::InitStartBlock()
{
    assert(bb_vector_.empty());
    assert(BB_START_ID == bb_id_counter_);

    bb_vector_.emplace_back(new BasicBlock(bb_id_counter_++));
}

BasicBlock* Graph::NewBasicBlock()
{
    bb_vector_.emplace_back(new BasicBlock(bb_id_counter_++));
    return bb_vector_.back().get();
}

BasicBlock* Graph::ReleaseBasicBlock(IdType id)
{
    auto bb = bb_vector_.at(id).release();
    bb_vector_.at(id).reset();
    return bb;
}

IdType Graph::NewBasicBlock(BasicBlock* bb)
{
    bb_vector_.emplace_back(bb);
    bb->SetId(bb_id_counter_++);
    return bb_id_counter_;
}

void Graph::Dump(std::string name)
{
    std::cout << "#########################\n";
    std::cout << "# GRAPH: " << std::move(name) << "\n";
    for (const auto& bb : bb_vector_) {
        if (bb != nullptr) {
            bb->Dump();
        }
    }
}

void Graph::ClearDominators()
{
    for (const auto& bb : bb_vector_) {
        bb->ClearImmDominator();
    }
}

void Graph::ClearLoops()
{
    for (const auto& bb : bb_vector_) {
        bb->SetLoop(nullptr);
    }
}

void Graph::AddEdge(BasicBlock* from, BasicBlock* to, size_t slot)
{
    assert(to != nullptr);
    assert(from != nullptr);

    // from->AddSuccsessor(to);
    from->SetSuccsessor(slot, to);
    to->AddPredecessor(from);

    analyser_.InvalidateCFGSensitiveActivePasses();
}

void Graph::InsertBasicBlock(BasicBlock* bb, BasicBlock* from, BasicBlock* to)
{
    from->ReplaceSuccessor(to, bb);
    bb->AddPredecessor(from);
    to->ReplacePredecessor(from, bb);
    bb->AddSuccsessor(to);

    analyser_.InvalidateCFGSensitiveActivePasses();
}

void Graph::InsertBasicBlockBefore(BasicBlock* bb, BasicBlock* before)
{
    assert(!before->HasNoPredecessors());

    for (const auto& pred : before->GetPredecessors()) {
        pred->ReplaceSuccessor(before, bb);
        before->RemovePredecessor(pred);
        bb->AddPredecessor(pred);
    }
    AddEdge(bb, before, Conditional::Branch::FALLTHROUGH);

    analyser_.InvalidateCFGSensitiveActivePasses();
}

void Graph::ReplaceSuccessor(BasicBlock* bb, BasicBlock* prev_succ, BasicBlock* new_succ)
{
    assert(bb != nullptr);
    assert(prev_succ != nullptr);

    bb->ReplaceSuccessor(prev_succ, new_succ);
    prev_succ->RemovePredecessor(bb);

    if (new_succ != nullptr) {
        new_succ->AddPredecessor(bb);
    }

    analyser_.InvalidateCFGSensitiveActivePasses();
}

BasicBlock* Graph::SplitBasicBlock(InstBase* inst_after)
{
    assert(inst_after != nullptr);

    auto bb = inst_after->GetBasicBlock();
    auto prev_last = bb->GetLastInst();

    auto second_half = std::unique_ptr<InstBase>{ inst_after->ReleaseNext() };
    assert(inst_after->GetNext() == nullptr);
    auto first_half = std::unique_ptr<InstBase>{ bb->TransferInst() };

    auto bb_new = NewBasicBlock();
    bb_new->PushBackInst(std::move(first_half));
    bb_new->SetLastInst(inst_after);

    InsertBasicBlockBefore(bb_new, bb);

    if (second_half != nullptr) {
        second_half->SetPrev(nullptr);
        bb->PushBackInst(std::move(second_half));
        bb->SetLastInst(prev_last);
    }

    for (auto inst = bb_new->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
        inst->SetBasicBlock(bb_new);
    }

    analyser_.InvalidateCFGSensitiveActivePasses();

    return bb_new;
}

void Graph::DestroyBasicBlock(BasicBlock* bb)
{
    assert(bb != nullptr);
    assert(bb->HasNoPredecessors());
    assert(bb->HasNoSuccessors());

    bb_vector_.at(bb->GetId()).reset(nullptr);

    analyser_.InvalidateCFGSensitiveActivePasses();
}
