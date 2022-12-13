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
#include "loop.h"
#include "macros.h"
#include "marker.h"
#include "typedefs.h"

class Graph;

class BasicBlock : public marking::Markable
{
  public:
    BasicBlock(const IdType& id) : id_(id)
    {
    }
    NO_DEFAULT_CTOR(BasicBlock);
    DEFAULT_DTOR(BasicBlock);

    GETTER(Predecesors, preds_);
    GETTER(Successors, succs_);
    GETTER(LastInst, last_inst_);
    GETTER(LastPhi, last_phi_);
    GETTER(Id, id_);
    GETTER_SETTER(ImmDominator, BasicBlock*, imm_dominator_);
    GETTER(Loop, loop_);

    void SetLoop(Loop* loop, bool is_header = false)
    {
        assert(loop != nullptr);

        is_loop_header_ = is_header;
        loop_ = loop;
    }

    bool IsLoopHeader()
    {
        return is_loop_header_;
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

    bool Dominates(BasicBlock* bb) const
    {
        assert(bb != nullptr);

        auto dom = bb->GetImmDominator();
        while (dom != nullptr) {
            if (dom == this) {
                return true;
            }
            dom = dom->GetImmDominator();
        }
        return false;
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

    void ReplaceSucc(BasicBlock* bb_old, BasicBlock* bb_new);
    void ReplacePred(BasicBlock* bb_old, BasicBlock* bb_new);

    void PushBackInst(std::unique_ptr<Inst> inst);
    void PushFrontInst(std::unique_ptr<Inst> inst);
    void InsertInst(std::unique_ptr<Inst> inst, Inst* left, Inst* right);
    void InsertInstAfter(std::unique_ptr<Inst> inst, Inst* after);
    void InsertInstBefore(std::unique_ptr<Inst> inst, Inst* before);
    void PushBackPhi(std::unique_ptr<Inst> inst);
    void PushBackPhi(Inst* inst);

    void RemoveInst(Inst* inst);
    void RemovePhi(Inst* inst);
    void RemoveInst(const IdType inst_id);

    void Dump() const;

  private:
    void SetFirstInst(std::unique_ptr<Inst> inst);

    Loop* loop_ = nullptr;
    bool is_loop_header_ = false;

    std::vector<BasicBlock*> preds_{}; // predecessors
    std::vector<BasicBlock*> succs_{}; // successors

    BasicBlock* imm_dominator_{ nullptr };

    const IdType id_;

    std::unique_ptr<Inst> first_inst_{ nullptr };
    Inst* last_inst_{ nullptr };
    std::unique_ptr<Inst> first_phi_{ nullptr };
    Inst* last_phi_{ nullptr };
};

#endif
