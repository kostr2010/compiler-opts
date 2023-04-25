#ifndef ___BASICBLOCK_H_INCLUDED___
#define ___BASICBLOCK_H_INCLUDED___

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "inst_.h"
#include "loop.h"
#include "macros.h"
#include "marker/markable.h"
#include "typedefs.h"

#include "isa/isa.h"

class Graph;

class BasicBlock : public marker::Markable
{
  public:
    BasicBlock(const IdType& id) : id_(id)
    {
    }

    GETTER(Predecessors, preds_);
    GETTER(Successors, succs_);
    GETTER_SETTER(LastInst, InstBase*, last_inst_);
    GETTER_SETTER(LastPhi, InstBase*, last_phi_);
    GETTER_SETTER(Id, IdType, id_);
    GETTER_SETTER(ImmDominator, BasicBlock*, imm_dominator_);
    GETTER(Loop, loop_);

    bool HasNoPredecessors() const;
    size_t GetNumPredecessors() const;
    BasicBlock* GetPredecessor(size_t idx);

    bool HasNoSuccessors() const;
    size_t GetNumSuccessors() const;
    BasicBlock* GetSuccessor(size_t idx);

    void SetLoop(Loop* loop, bool is_header = false);
    bool IsLoopHeader() const;

    InstBase* GetFirstPhi() const;
    InstBase* GetFirstInst() const;

    void ClearImmDominator();
    bool Dominates(BasicBlock* bb) const;

    bool IsEmpty() const;
    bool IsStartBlock() const;
    bool IsEndBlock() const;

    void PushBackInst(std::unique_ptr<InstBase> inst);
    void PushFrontInst(std::unique_ptr<InstBase> inst);
    void InsertInst(std::unique_ptr<InstBase> inst, InstBase* left, InstBase* right);
    void InsertInstAfter(std::unique_ptr<InstBase> inst, InstBase* after);
    void InsertInstBefore(std::unique_ptr<InstBase> inst, InstBase* before);
    void PushBackPhi(std::unique_ptr<InstBase> inst);
    void PushBackPhi(InstBase* inst);

    void UnlinkInst(InstBase* inst);
    InstBase* TransferInst();
    InstBase* TransferPhi();

    void Dump() const;

  private:
    // only graph can manipulate CFG
    friend Graph;
    void SetSucc(BasicBlock* bb, size_t pos = 0);
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

    void SetFirstInst(std::unique_ptr<InstBase> inst);

    Loop* loop_ = nullptr;
    bool is_loop_header_ = false;

    using NoBranches =
        std::integral_constant<isa::flag::ValueT,
                               isa::flag::Flag<isa::flag::Type::BRANCH>::Value::NO_SUCCESSORS>;

    template <isa::inst::Opcode OP>
    using GetNumBranches = isa::FlagValueOr<OP, isa::flag::Type::BRANCH, NoBranches::value>;

    template <typename INST, typename ACC>
    struct BranchNumAccumulator
    {
        using BranchNum = GetNumBranches<INST::opcode>;
        static constexpr isa::flag::ValueT value =
            std::conditional_t < ACC::value<BranchNum::value, BranchNum, ACC>::value;
    };

    using MaxBranchNum = type_sequence::Accumulate<isa::ISA, BranchNumAccumulator, NoBranches>;
    std::vector<BasicBlock*> preds_{};                     // predecessors
    std::array<BasicBlock*, MaxBranchNum::value> succs_{}; // successors

    BasicBlock* imm_dominator_{ nullptr };

    IdType id_;

    std::unique_ptr<InstBase> first_inst_{ nullptr };
    InstBase* last_inst_{ nullptr };
    std::unique_ptr<InstBase> first_phi_{ nullptr };
    InstBase* last_phi_{ nullptr };
};

#endif
