#ifndef ___BASICBLOCK_H_INCLUDED___
#define ___BASICBLOCK_H_INCLUDED___

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "inst.h"
#include "loop.h"
#include "typedefs.h"
#include "utils/macros.h"
#include "utils/marker/markable.h"

#include "isa/isa.h"

class Graph;

class BasicBlock : public marker::Markable
{
  public:
    BasicBlock(const IdType& id) : id_(id)
    {
    }

    GETTER(Predecessors, preds_);

    auto GetSuccessors() const
    {
        std::vector<BasicBlock*> succ{};

        for (auto bb : succs_) {
            if (bb != nullptr) {
                succ.push_back(bb);
            }
        }

        return succ;
    }

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

    bool Precedes(IdType bb_id) const;
    bool Precedes(BasicBlock* bb) const;

    bool Succeeds(IdType bb_id) const;
    bool Succeeds(BasicBlock* bb) const;

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
    void SetSuccsessor(size_t pos, BasicBlock* bb);

    void AddPredecessor(BasicBlock* bb);
    void RemovePredecessor(BasicBlock* bb);

    void ReplaceSuccessor(BasicBlock* bb_old, BasicBlock* bb_new);
    void ReplacePredecessor(BasicBlock* bb_old, BasicBlock* bb_new);

    void SetFirstInst(std::unique_ptr<InstBase> inst);

    Loop* loop_ = nullptr;
    bool is_loop_header_ = false;

    using NumBranchesDefault =
        std::integral_constant<isa::flag::ValueT,
                               isa::flag::Flag<isa::flag::Type::BRANCH>::Value::ONE_SUCCESSOR>;

    template <isa::inst::Opcode OP>
    using GetNumBranches =
        isa::FlagValueOr<OP, isa::flag::Type::BRANCH, NumBranchesDefault::value>;

    template <typename INST, typename ACC>
    struct BranchNumAccumulator
    {
        using BranchNum = GetNumBranches<INST::opcode>;
        static constexpr isa::flag::ValueT value =
            std::conditional_t < ACC::value<BranchNum::value, BranchNum, ACC>::value;
    };

    using MaxBranchNum =
        type_sequence::Accumulate<isa::ISA, BranchNumAccumulator, NumBranchesDefault>;
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
