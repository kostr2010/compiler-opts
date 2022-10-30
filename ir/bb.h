#ifndef ___BASICBLOCK_H_INCLUDED___
#define ___BASICBLOCK_H_INCLUDED___

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "inst.h"
#include "macros.h"

#include "typedefs.h"

class Graph;

class BasicBlock
{
  public:
    BasicBlock(/* Graph* graph, */ const IdType& id) : id_(id)
    {
        // assert(graph != nullptr);
        // graph_ = graph;
    }
    DEFAULT_DTOR(BasicBlock);

    GETTER(Predecesors, preds_);
    GETTER(Successors, succs_);
    GETTER(Dominated, dominated_);
    GETTER(Dominators, dominators_);
    GETTER(LastInst, last_inst_);
    GETTER(Id, id_);
    GETTER_SETTER(ImmDominator, BasicBlock*, imm_dominator_);

    uint64_t* GetBits()
    {
        return &bits;
    }

    inline Inst* GetFirstPhi()
    {
        return first_phi_.get();
    }

    inline Inst* GetFirstInst()
    {
        return first_inst_.get();
    }

    inline void ClearImmDominator()
    {
        imm_dominator_ = nullptr;
    }

    inline void ClearDominators()
    {
        dominators_.clear();
    }

    inline void ClearDominated()
    {
        dominated_.clear();
    }

    void AddDominator(BasicBlock* bb)
    {
        if (HasDominator(bb)) {
            return;
        }

        dominators_.push_back(bb);
    }

    void AddDominated(BasicBlock* bb)
    {
        if (HasDominated(bb)) {
            return;
        }

        dominated_.push_back(bb);
    }

    inline bool HasDominated(IdType bb_id)
    {
        return std::find_if(dominated_.begin(), dominated_.end(), [bb_id](BasicBlock* bb) {
                   return bb->GetId() == bb_id;
               }) != dominated_.end();
    }

    inline bool HasDominated(BasicBlock* bb)
    {
        assert(bb != nullptr);
        return std::find_if(dominated_.begin(), dominated_.end(), [bb](BasicBlock* d) {
                   return bb->GetId() == d->GetId();
               }) != dominated_.end();
    }

    inline bool HasDominator(IdType bb_id)
    {
        return std::find_if(dominators_.begin(), dominators_.end(), [bb_id](BasicBlock* bb) {
                   return bb->GetId() == bb_id;
               }) != dominators_.end();
    }

    inline bool HasDominator(BasicBlock* bb)
    {
        assert(bb != nullptr);
        return std::find_if(dominators_.begin(), dominators_.end(), [bb](BasicBlock* d) {
                   return bb->GetId() == d->GetId();
               }) != dominators_.end();
    }

    bool IsStartBlock() const;
    bool IsEndBlock() const;

    void AddSucc(BasicBlock* bb);
    bool IsInSucc(IdType bb_id) const;
    bool IsInSucc(BasicBlock* bb) const;
    void RemoveSucc(BasicBlock* bb);

    void AddPred(BasicBlock* bb);
    bool IsInPred(IdType bb_id) const;
    bool IsInPred(BasicBlock* bb) const;
    void RemovePred(BasicBlock* bb);

    void PushBackInst(std::unique_ptr<Inst> inst);
    void PushFrontInst(std::unique_ptr<Inst> inst);
    void PushBackPhi(std::unique_ptr<Inst> inst);

    void Dump() const;

  private:
    void SetFirstInst(std::unique_ptr<Inst> inst);

    // Graph* graph_; // access to metadata

    std::vector<BasicBlock*> preds_{}; // predecessors
    std::vector<BasicBlock*> succs_{}; // successors

    std::vector<BasicBlock*> dominated_{};
    std::vector<BasicBlock*> dominators_{};
    BasicBlock* imm_dominator_{ nullptr };

    const IdType id_;

    std::unique_ptr<Inst> first_inst_{ nullptr };
    Inst* last_inst_{ nullptr };
    std::unique_ptr<Inst> first_phi_{ nullptr };

    MarkHolderT bits{};
};

#endif