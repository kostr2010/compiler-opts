#ifndef ___BASICBLOCK_H_INCLUDED___
#define ___BASICBLOCK_H_INCLUDED___

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "graph.h"
#include "inst.h"

#include "typedefs.h"

class Graph;

class BasicBlock
{
  public:
    BasicBlock(Graph* graph)
    {
        assert(graph != nullptr);
        graph_ = graph;
    }

    bool IsStartBlock() const
    {
        return id_ == graph_->GetStartBasicBlockId();
    }

    bool IsEndBlock() const
    {
        return id_ == graph_->GetEndBasicBlockId();
    }

    void SetId(IdType id)
    {
        id_ = id;
    }

    IdType GetId() const
    {
        return id_;
    }

    void AddSucc(BasicBlock* bb)
    {
        assert(bb != nullptr);

        if (IsInSucc(bb)) {
            return;
        }

        succs_.push_back(bb);
        bb->AddPred(this);
    }

    bool IsInSucc(BasicBlock* bb) const
    {
        assert(bb != nullptr);
        return std::find(succs_.begin(), succs_.end(), bb) != succs_.end();
    }

    void AddPred(BasicBlock* bb)
    {
        assert(bb != nullptr);

        if (IsInPred(bb)) {
            return;
        }

        preds_.push_back(bb);
        bb->AddSucc(this);
    }

    bool IsInPred(BasicBlock* bb) const
    {
        assert(bb != nullptr);
        return std::find(preds_.begin(), preds_.end(), bb) != preds_.end();
    }

    void PushBackInst(Inst* inst)
    {
        assert(inst != nullptr);

        inst->SetBasicBlock(this);
        if (first_inst_ == nullptr) {
            SetFirstInst(inst);
        } else {
            inst->SetNext(nullptr);
            inst->SetPrev(last_inst_);
            last_inst_->SetNext(inst);
            last_inst_ = inst;
        }
    }

    void PushFrontInst(Inst* inst)
    {
        assert(inst != nullptr);

        inst->SetBasicBlock(this);
        if (first_inst_ == nullptr) {
            SetFirstInst(inst);
        } else {
            inst->SetNext(first_inst_);
            inst->SetPrev(first_inst_->GetPrev());
            first_inst_->SetPrev(inst);
            if (first_phi_ != nullptr) {
                first_phi_->SetNext(inst);
            }
            first_inst_ = inst;
        }
    }

    void PushBackPhi(Inst* inst)
    {
        assert(inst != nullptr);
        assert(inst->IsPhi());

        inst->SetBasicBlock(this);
        if (first_phi_ == nullptr) {
            // all phi's go in front of actual instructions
            inst->SetNext(first_inst_);
            if (first_inst_ != nullptr) {
                first_inst_->SetPrev(inst);
            }
            first_phi_ = inst;
        } else {
            if (first_inst_ != nullptr) {
                auto prev = first_inst_->GetPrev();
                assert(prev != nullptr);
                assert(prev->IsPhi());
                assert(prev == first_phi_);

                inst->SetPrev(prev);
                inst->SetNext(first_inst_);
                prev->SetNext(inst);
                first_inst_->SetPrev(inst);
            } else {
                inst->SetPrev(first_phi_);
                first_phi_->SetNext(inst);
            }
        }
    }

    Inst* GetFirstInst() const
    {
        return first_inst_;
    }

    Inst* GetLastInst() const
    {
        return last_inst_;
    }

    void Dump()
    {
        std::cout << "# BB id: " << id_ << "\n";

        std::cout << "# PREDS: ";
        for (auto bb : preds_) {
            std::cout << bb->GetId() << " ";
        }
        std::cout << "\n";

        std::cout << "# SUCCS: ";
        for (auto bb : succs_) {
            std::cout << bb->GetId() << " ";
        }
        std::cout << "\n";

        std::cout << "# INSTRUCTIONS:\n";
        for (auto inst = ((first_phi_ == nullptr) ? first_inst_ : first_phi_); inst != nullptr;
             inst = inst->GetNext()) {
            inst->Dump();
        }
        std::cout << "\n";
    }

  private:
    void SetFirstInst(Inst* inst)
    {

        assert(inst != nullptr);

        inst->SetNext(nullptr);
        inst->SetPrev(nullptr);

        if (first_phi_ != nullptr) {
            first_phi_->SetNext(inst);
            inst->SetPrev(first_phi_);
        }

        first_inst_ = inst;
        last_inst_ = inst;
    }

    Graph* graph_; // access to metadata

    std::vector<BasicBlock*> preds_{}; // predecessors
    std::vector<BasicBlock*> succs_{}; // successors
    std::vector<BasicBlock*> dom_{};   // dominates
    BasicBlock* dominator_{ nullptr };

    IdType id_;

    Inst* first_inst_{ nullptr };
    Inst* last_inst_{ nullptr };
    Inst* first_phi_{ nullptr };
};

#endif