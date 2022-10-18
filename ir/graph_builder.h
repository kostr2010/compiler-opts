#ifndef ___GRAPH_BUILDER_H_INCLUDED___
#define ___GRAPH_BUILDER_H_INCLUDED___

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "bb.h"
#include "macros.h"
#include "typedefs.h"

class Graph;
// class BasicBlock;
class Inst;

// pointers to avoid circular dependency
class GraphBuilder
{
  public:
    GraphBuilder(Graph* g)
    {
        SetGraph(g);
    }

    ~GraphBuilder()
    {
    }

    void SetGraph(Graph* g)
    {
        assert(g != nullptr);
        graph_ = g;

        bb_map_[g->BB_START_ID] = g->GetStartBasicBlock();
        bb_map_[g->BB_END_ID] = g->GetEndBasicBlock();

        cur_bb_ = g->GetStartBasicBlock();
        cur_bb_id_ = g->BB_START_ID;

        cfg_constructed = false;
        dfg_constructed = false;
        graph_checked = false;
        is_empty = true;
    }

    template <Opcode op, typename... Args>
    IdType NewInst(Args&&... args)
    {
        assert(graph_ != nullptr);
        assert(cur_bb_ != nullptr);

        static_assert(op != Opcode::CONST);

        auto inst = graph_->NewInst<op>(std::forward<Args>(args)...);
        assert(inst != nullptr);
        auto id = inst->GetId();

        cur_inst_ = inst;
        cur_inst_id_ = id;

        inst_map_[id] = inst;

        if (inst->IsPhi()) {
            cur_bb_->PushBackPhi(inst);
        } else {
            cur_bb_->PushBackInst(inst);
        }

        return id;
    }

    IdType NewParameter(ArgNumType arg_num)
    {
        assert(graph_ != nullptr);
        assert(graph_->GetStartBasicBlock() != nullptr);

        auto inst = graph_->NewParam(arg_num);
        assert(inst != nullptr);
        auto id = inst->GetId();

        cur_inst_ = inst;
        cur_inst_id_ = id;

        inst_map_[id] = inst;

        graph_->GetStartBasicBlock()->PushBackInst(inst);

        return id;
    }

    template <typename T>
    IdType NewConst(T value)
    {
        assert(graph_ != nullptr);
        assert(graph_->GetStartBasicBlock() != nullptr);

        auto inst = graph_->NewConst(value);
        assert(inst != nullptr);
        auto id = inst->GetId();

        cur_inst_ = inst;
        cur_inst_id_ = id;

        inst_map_[id] = inst;

        graph_->GetStartBasicBlock()->PushBackInst(inst);

        return id;
    }

    IdType NewBlock()
    {
        assert(graph_ != nullptr);

        auto bb = graph_->NewBasicBlock();
        auto id = bb->GetId();

        cur_bb_ = bb;
        cur_bb_id_ = id;

        bb_map_[cur_bb_id_] = bb;

        if (is_empty) {
            is_empty = false;
            graph_->GetStartBasicBlock()->AddSucc(bb);
        }

        return id;
    }

    void SetSuccessors(IdType bb_id, std::vector<IdType>&& succs)
    {
        bb_succ_map_[bb_id] = succs;
    }

    template <typename... Args>
    void SetInputs(IdType id, Args... inputs_id)
    {
        assert(inst_map_.find(id) != inst_map_.end());
        // assert(!inst_map_.at(id)->IsPhi());

        if constexpr (sizeof...(inputs_id)) {
            AddInput(id, inputs_id...);
        }
    }

    void SetInputs(IdType id, std::vector<std::pair<IdType, IdType> >&& inputs)
    {
        assert(inst_map_.find(id) != inst_map_.end());
        assert(inst_map_.at(id)->IsPhi());

        phi_inputs_map_[id].reserve(inputs.size());
        phi_inputs_map_.at(id) = std::move(inputs);
    }

    void SetType(IdType id, DataType t)
    {
        auto inst = inst_map_[id];
        assert(inst != nullptr);
        inst->SetDataType(t);
    }

    void SetImm(IdType id, ImmType imm)
    {
        auto inst = inst_map_[id];
        assert(inst != nullptr);

        switch (inst->GetOpcode()) {
        case Opcode::ADDI:
        case Opcode::SUBI:
        case Opcode::MULI:
        case Opcode::DIVI:
        case Opcode::MODI:
        case Opcode::MINI:
        case Opcode::MAXI:
        case Opcode::SHLI:
        case Opcode::SHRI:
        case Opcode::ASHRI:
        case Opcode::ANDI:
        case Opcode::ORI:
        case Opcode::XORI:
            static_cast<BinaryImmOp*>(inst)->SetImm(imm);
            break;
        case Opcode::IF_IMM:
            static_cast<IfImmOp*>(inst)->SetImm(imm);
            break;
        default:
            assert(false);
        }
    }

    void SetCond(IdType id, CondType c)
    {
        auto inst = inst_map_[id];
        assert(inst != nullptr);
        assert(inst->IsCond());

        switch (inst->GetOpcode()) {
        case Opcode::IF:
            static_cast<IfOp*>(inst)->SetCond(c);
            break;
        case Opcode::IF_IMM:
            static_cast<IfImmOp*>(inst)->SetCond(c);
            break;
        case Opcode::CMP:
            static_cast<CompareOp*>(inst)->SetCond(c);
            break;
        default:
            assert(false);
        }
    }

    void ConstructCFG()
    {
        for (auto& [bb_id, succs] : bb_succ_map_) {
            assert(succs.size() <= 2);
            auto bb = bb_map_.at(bb_id);
            for (auto succ : succs) {
                bb->AddSucc(bb_map_.at(succ));
            }
        }

        cfg_constructed = true;
    }

    void ConstructDFG()
    {
        for (auto& [inst_id, inputs] : inst_inputs_map_) {
            assert(inst_map_.find(inst_id) != inst_map_.end());
            auto inst = inst_map_.at(inst_id);
            assert(!inst->IsPhi());

            size_t input_idx = 0;

            for (auto input_id : inputs) {
                assert(inst_map_.find(input_id) != inst_map_.end());

                inst->SetInput(input_idx, inst_map_.at(input_id));
                ++input_idx;
            }

            if (inst->IsTypeSensitive()) {
                inst->CheckInputType();
            }
        }

        for (auto& [inst_id, inputs] : phi_inputs_map_) {
            assert(inst_map_.find(inst_id) != inst_map_.end());
            auto inst = inst_map_.at(inst_id);
            assert(inst->IsPhi());

            for (auto& input : inputs) {
                auto input_inst_id = input.first;
                auto input_inst_bb = input.second;

                assert(inst_map_.find(input_inst_id) != inst_map_.end());
                auto input_inst = inst_map_.at(input_inst_id);
                auto input_bb = bb_map_.at(input_inst_bb);

                auto idx = inst->AddInput(input_inst);
                static_cast<PhiOp*>(inst)->SetInputBB(idx, input_bb);
            }
        }

        dfg_constructed = true;
    }

    std::vector<BasicBlock*> RPOPass()
    {
        if (!graph_checked) {
            LOG_ERROR("RPO can only be constructed after graph validation");
        }

        return {};
    }

    bool RunChecks()
    {
        assert(cfg_constructed);
        assert(dfg_constructed);

        // check inputs of variable length
        // PHI: inst->bb->Dominates(source_bb);
        //      inputs.size() == preds.size()

        // check types

        graph_checked = true;
        return true;
    }

  private:
    void AddInput(IdType i_id, IdType id)
    {
        inst_inputs_map_[i_id].push_back(id);
    }

    template <typename T, typename... Args>
    void AddInput(IdType i_id, T id, Args&&... args)
    {
        inst_inputs_map_[i_id].push_back(id);
        AddInput(i_id, std::forward<Args>(args)...);
    }

    bool cfg_constructed = false;
    bool dfg_constructed = false;
    bool graph_checked = false;

    bool is_empty = true;
    Graph* graph_{ nullptr };
    BasicBlock* cur_bb_{ nullptr };
    IdType cur_bb_id_;
    Inst* cur_inst_{ nullptr };
    IdType cur_inst_id_;

    std::unordered_map<IdType, BasicBlock*> bb_map_;
    std::unordered_map<IdType, std::vector<IdType> > bb_succ_map_;
    std::unordered_map<IdType, Inst*> inst_map_;
    std::unordered_map<IdType, std::vector<IdType> > inst_inputs_map_;
    std::unordered_map<IdType, std::vector<std::pair<IdType, IdType> > > phi_inputs_map_;
};

#endif