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
#include "macros.h"

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

    DEFAULT_DTOR(BasicBlock);

    GETTER(Predecesors, preds_);
    GETTER(Successors, succs_);
    GETTER(Dominated, dom_);
    GETTER(Dominator, dominator_);
    GETTER(FirstPhi, first_phi_);
    GETTER(FirstInst, first_inst_);
    GETTER(LastInst, last_inst_);
    GETTER_SETTER(Id, IdType, id_);

    bool Dominates(BasicBlock* bb)
    {
        assert(bb != nullptr);
        // FIXME:
        return true;
    }

    bool IsStartBlock() const
    {
        return id_ == graph_->GetStartBasicBlockId();
    }

    bool IsEndBlock() const
    {
        return succs_.empty();
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

    void PushBackInst(Inst* inst);
    void PushFrontInst(Inst* inst);
    void PushBackPhi(Inst* inst);

    void Dump() const;

    uint64_t* GetBits()
    {
        return &bits;
    }

    void ResetBits()
    {
        bits &= 0;
    }

  private:
    void SetFirstInst(Inst* inst)
    {
        assert(inst != nullptr);
        assert(first_inst_ == nullptr);
        assert(last_inst_ == nullptr);

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

    uint64_t bits{ 0 };
};

#endif