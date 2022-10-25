#ifndef ___BASICBLOCK_H_INCLUDED___
#define ___BASICBLOCK_H_INCLUDED___

#include <algorithm>
#include <cassert>
#include <iostream>
#include <list>
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
    GETTER(Dominated, dominated_);
    GETTER(Dominators, dominators_);
    GETTER(FirstPhi, first_phi_);
    GETTER(FirstInst, first_inst_);
    GETTER(LastInst, last_inst_);
    GETTER_SETTER(Id, IdType, id_);
    GETTER_SETTER(ImmDominator, BasicBlock*, imm_dominator_);

    void ClearImmDominator()
    {
        imm_dominator_ = nullptr;
    }

    void ClearDominators()
    {
        dominators_.clear();
    }

    void ClearDominated()
    {
        dominated_.clear();
    }

    void AddDominator(BasicBlock* bb)
    {
        if (IsInDominators(bb)) {
            return;
        }

        dominators_.push_back(bb);
    }

    void AddDominated(BasicBlock* bb)
    {
        if (IsInDominated(bb)) {
            return;
        }

        dominated_.push_back(bb);
    }

    bool IsInDominated(IdType bb_id)
    {
        return std::find_if(dominated_.begin(), dominated_.end(), [bb_id](BasicBlock* bb) {
                   return bb->GetId() == bb_id;
               }) != dominated_.end();
    }

    bool IsInDominated(BasicBlock* bb)
    {
        assert(bb != nullptr);
        return std::find(dominated_.begin(), dominated_.end(), bb) != dominated_.end();
    }

    bool IsInDominators(IdType bb_id)
    {
        return std::find_if(dominators_.begin(), dominators_.end(), [bb_id](BasicBlock* bb) {
                   return bb->GetId() == bb_id;
               }) != dominators_.end();
    }

    bool IsInDominators(BasicBlock* bb)
    {
        assert(bb != nullptr);
        return std::find(dominators_.begin(), dominators_.end(), bb) != dominators_.end();
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
    }

    bool IsInSucc(IdType bb_id) const
    {
        return std::find_if(succs_.begin(), succs_.end(), [bb_id](BasicBlock* bb) {
                   return bb->GetId() == bb_id;
               }) != succs_.end();
    }

    bool IsInSucc(BasicBlock* bb) const
    {
        assert(bb != nullptr);
        return std::find(succs_.begin(), succs_.end(), bb) != succs_.end();
    }

    void RemoveSucc(BasicBlock* bb)
    {
        if (!IsInSucc(bb)) {
            return;
        }

        succs_.erase(std::find(succs_.begin(), succs_.end(), bb));
    }

    void AddPred(BasicBlock* bb)
    {
        assert(bb != nullptr);

        if (IsInPred(bb)) {
            return;
        }

        preds_.push_back(bb);
    }

    bool IsInPred(IdType bb_id) const
    {
        return std::find_if(preds_.begin(), preds_.end(), [bb_id](BasicBlock* bb) {
                   return bb->GetId() == bb_id;
               }) != preds_.end();
    }

    bool IsInPred(BasicBlock* bb) const
    {
        assert(bb != nullptr);
        return std::find(preds_.begin(), preds_.end(), bb) != preds_.end();
    }

    void RemovePred(BasicBlock* bb)
    {
        if (!IsInPred(bb)) {
            return;
        }

        preds_.erase(std::find(preds_.begin(), preds_.end(), bb));
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

    std::vector<BasicBlock*> dominated_{};
    std::vector<BasicBlock*> dominators_{};
    BasicBlock* imm_dominator_{ nullptr };

    IdType id_;

    Inst* first_inst_{ nullptr };
    Inst* last_inst_{ nullptr };
    Inst* first_phi_{ nullptr };

    uint64_t bits{ 0 };
};

#endif