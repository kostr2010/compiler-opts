#ifndef ___GRAPH_BUILDER_H_INCLUDED___
#define ___GRAPH_BUILDER_H_INCLUDED___

#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "bb.h"
#include "graph.h"
#include "ir_isa.h"
#include "macros.h"
#include "typedefs.h"

class Inst;

class GraphBuilder
{
  public:
    GraphBuilder(Graph* g);
    DEFAULT_DTOR(GraphBuilder);

    template <Opcode OPCODE, typename... Args>
    IdType NewInst(Args&&... args)
    {
        assert(graph_ != nullptr);
        assert(cur_bb_ != nullptr);

        static_assert(OPCODE != Opcode::CONST);
        static_assert(OPCODE != Opcode::PARAM);

        std::unique_ptr<Inst> inst = Inst::NewInst<OPCODE>(std::forward<Args>(args)...);

        assert(inst != nullptr);
        auto id = inst->GetId();

        cur_inst_ = inst.get();
        inst_map_[id] = inst.get();

        if (inst->IsPhi()) {
            cur_bb_->PushBackPhi(std::move(inst));
        } else {
            cur_bb_->PushBackInst(std::move(inst));
        }

        return id;
    }

    IdType NewParameter(ArgNumType arg_num);

    template <typename T>
    IdType NewConst(T value)
    {
        assert(graph_ != nullptr);
        assert(graph_->GetStartBasicBlock() != nullptr);

        auto inst = Inst::NewInst<Opcode::CONST>(value);

        assert(inst != nullptr);
        auto id = inst->GetId();

        cur_inst_ = inst.get();
        inst_map_[id] = inst.get();

        graph_->GetStartBasicBlock()->PushBackInst(std::move(inst));

        return id;
    }

    IdType NewBlock();
    void SetSuccessors(IdType bb_id, std::vector<IdType>&& succs);

    void SetInputs(IdType id, std::vector<std::pair<IdType, IdType> >&& inputs);

    template <typename... Args>
    void SetInputs(IdType id, Args... inputs_id)
    {
        assert(inst_map_.find(id) != inst_map_.end());
        assert(!inst_map_.at(id)->IsPhi());

        if constexpr (sizeof...(inputs_id)) {
            AddInput(id, inputs_id...);
        }
    }

    void SetType(IdType id, DataType t);
    void SetImm(IdType id, ImmType imm);
    void SetCond(IdType id, CondType c);

    void ConstructCFG();
    void ConstructDFG();

    bool RunChecks();

  private:
    void SetGraph(Graph* g);

    void AddInput(IdType i_id, IdType id);

    template <typename T, typename... Args>
    void AddInput(IdType i_id, T id, Args&&... args)
    {
        inst_inputs_map_[i_id].push_back(id);
        AddInput(i_id, std::forward<Args>(args)...);
    }

    bool cfg_constructed = false;
    bool dfg_constructed = false;

    Graph* graph_{ nullptr };
    BasicBlock* cur_bb_{ nullptr };
    IdType cur_bb_id_;
    Inst* cur_inst_{ nullptr };

    std::unordered_map<IdType, BasicBlock*> bb_map_;
    std::unordered_map<IdType, std::vector<IdType> > bb_succ_map_;
    std::unordered_map<IdType, Inst*> inst_map_;
    std::unordered_map<IdType, std::vector<IdType> > inst_inputs_map_;
    std::unordered_map<IdType, std::vector<std::pair<IdType, IdType> > > phi_inputs_map_;
};

#endif
