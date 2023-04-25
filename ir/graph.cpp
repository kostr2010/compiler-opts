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

    analyser_.InvalidateCFGSensitiveActivePasses();
}

// void Graph::RemoveEdge(IdType from, IdType to)
// {
//     auto bb_to = bb_vector_.at(to).get();
//     auto bb_from = bb_vector_.at(from).get();

//     RemoveEdge(bb_from, bb_to);
// }

// void Graph::RemoveEdge(BasicBlock* from, BasicBlock* to)
// {
//     assert(to != nullptr);
//     assert(from != nullptr);

//     from->RemoveSucc(to);
//     to->RemovePred(from);

//     analyser_.InvalidateCFGSensitiveActivePasses();
// }

void Graph::InsertBasicBlock(BasicBlock* bb, BasicBlock* from, BasicBlock* to)
{
    from->ReplaceSucc(to, bb);
    bb->AddPred(from);
    to->ReplacePred(from, bb);
    bb->AddSucc(to);

    analyser_.InvalidateCFGSensitiveActivePasses();
}

void Graph::InsertBasicBlockBefore(BasicBlock* bb, BasicBlock* before)
{
    assert(!before->HasNoPredecessors());

    for (const auto& pred : before->GetPredecessors()) {
        pred->ReplaceSucc(before, bb);
        before->RemovePred(pred);
        bb->AddPred(pred);
    }
    AddEdge(bb, before);

    analyser_.InvalidateCFGSensitiveActivePasses();
}

// void Graph::InsertBasicBlockAfter(BasicBlock* bb, BasicBlock* after)
// {
//     if (after->IsEndBlock()) {
//         AddEdge(after, bb);
//     } else {
//         for (const auto& succ : after->GetSuccessors()) {
//             succ->ReplacePred(after, bb);
//             after->ReplaceSucc();
//             after->RemoveSucc(succ);
//             bb->AddSucc(succ);
//         }
//         AddEdge(after, bb);
//     }

//     analyser_.InvalidateCFGSensitiveActivePasses();
// }

void Graph::ReplaceSuccessor(BasicBlock* bb, BasicBlock* prev_succ, BasicBlock* new_succ)
{
    assert(bb != nullptr);
    assert(prev_succ != nullptr);

    bb->ReplaceSucc(prev_succ, new_succ);
    prev_succ->RemovePred(bb);

    if (new_succ != nullptr) {
        new_succ->AddPred(bb);
    }

    analyser_.InvalidateCFGSensitiveActivePasses();
}

// void Graph::AppendBasicBlock(BasicBlock* first, BasicBlock* second)
// {
//     assert(first != nullptr);
//     assert(second != nullptr);

//     auto second_last_phi = second->GetLastPhi();
//     auto first_last_phi = first->GetLastPhi();

//     // assert(first_last_phi != nullptr);
//     auto second_first_phi = second->TransferPhi();
//     if (second_first_phi != nullptr) {
//         std::unique_ptr<InstBase> i{ second_first_phi };
//         first->PushBackPhi(std::move(i));
//         second_first_phi->SetPrev(first_last_phi);
//         first->SetLastPhi(second_last_phi);

//         auto inst = second_first_phi;
//         while (inst != nullptr) {
//             inst->SetBasicBlock(first);
//             inst = inst->GetNext();
//         }
//     }

//     auto second_last_inst = second->GetLastInst();
//     auto first_last_inst = first->GetLastInst();

//     auto second_first_inst = second->TransferInst();
//     if (second_first_inst != nullptr) {
//         std::unique_ptr<InstBase> i{ second_first_inst };
//         first->PushBackInst(std::move(i));
//         second_first_inst->SetPrev(first_last_inst);
//         first->SetLastInst(second_last_inst);

//         auto inst = second_first_inst;
//         while (inst != nullptr) {
//             inst->SetBasicBlock(first);
//             inst = inst->GetNext();
//         }
//     }

//     analyser_.InvalidateCFGSensitiveActivePasses();
// }

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
