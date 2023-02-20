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
    return bb_vector_.at(id).release();
}

IdType Graph::NewBasicBlock(BasicBlock* bb)
{
    bb_vector_.emplace_back(bb);
    bb->SetId(bb_id_counter_++);
    return bb_id_counter_;
}

void Graph::Dump()
{
    for (const auto& bb : bb_vector_) {
        bb->Dump();
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

void Graph::AddEdge(IdType from, IdType to)
{
    auto bb_to = bb_vector_.at(to).get();
    auto bb_from = bb_vector_.at(from).get();

    AddEdge(bb_from, bb_to);
}

void Graph::AddEdge(BasicBlock* from, BasicBlock* to)
{
    assert(to != nullptr);
    assert(from != nullptr);

    from->AddSucc(to);
    to->AddPred(from);

    analyser_.InvalidateCfgDependentActivePasses();
}

void Graph::RemoveEdge(IdType from, IdType to)
{
    auto bb_to = bb_vector_.at(to).get();
    auto bb_from = bb_vector_.at(from).get();

    RemoveEdge(bb_from, bb_to);
}

void Graph::RemoveEdge(BasicBlock* from, BasicBlock* to)
{
    assert(to != nullptr);
    assert(from != nullptr);

    from->RemoveSucc(to);
    to->RemovePred(from);

    analyser_.InvalidateCfgDependentActivePasses();
}

void Graph::InsertBasicBlock(BasicBlock* bb, BasicBlock* from, BasicBlock* to)
{
    from->ReplaceSucc(to, bb);
    to->ReplacePred(from, bb);

    analyser_.InvalidateCfgDependentActivePasses();
}

void Graph::InsertBasicBlockBefore(BasicBlock* bb, BasicBlock* before)
{
    assert(!before->GetPredecesors().empty());

    for (const auto& pred : before->GetPredecesors()) {
        pred->ReplaceSucc(before, bb);
        before->RemovePred(pred);
        bb->AddPred(pred);
    }
    AddEdge(bb, before);

    analyser_.InvalidateCfgDependentActivePasses();
}

void Graph::InsertBasicBlockAfter(BasicBlock* bb, BasicBlock* after)
{
    if (after->GetSuccessors().empty()) {
        AddEdge(after, bb);
    } else {
        for (const auto& succ : after->GetSuccessors()) {
            succ->ReplacePred(after, bb);
            after->RemoveSucc(succ);
            bb->AddSucc(succ);
        }
        AddEdge(after, bb);
    }

    analyser_.InvalidateCfgDependentActivePasses();
}

void Graph::AppendBasicBlock(BasicBlock* first, BasicBlock* second)
{
    assert(first != nullptr);
    assert(second != nullptr);

    auto last_phi = first->GetLastPhi();
    auto last_inst = first->GetLastInst();

    last_phi->SetNext(second->TransferPhi());
    last_inst->SetNext(second->TransferInst());

    last_inst = last_inst->GetNext();
    while (last_inst != nullptr) {
        last_inst->SetBasicBlock(first);
        last_inst = last_inst->GetNext();
    }

    last_phi = last_phi->GetNext();
    while (last_phi != nullptr) {
        last_phi->SetBasicBlock(first);
        last_phi = last_phi->GetNext();
    }
}

BasicBlock* Graph::SplitBasicBlock(Inst* inst_after)
{
    auto bb = inst_after->GetBasicBlock();
    auto prev_last = bb->GetLastInst();
    auto bb_new = NewBasicBlock();
    auto next = inst_after->ReleaseNext();

    if (next != nullptr) {
        std::unique_ptr<Inst> split{ next };
        bb_new->PushBackInst(std::move(split));
        assert(inst_after->GetNext() == nullptr);
        bb->SetLastInst(inst_after);
        bb_new->SetLastInst(prev_last);
    }

    for (auto inst = bb_new->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
        inst->SetBasicBlock(bb_new);
    }

    return bb_new;
}
