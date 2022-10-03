#ifndef ___GRAPH_BUILDER_H_INCLUDED___
#define ___GRAPH_BUILDER_H_INCLUDED___

#include <cassert>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "bb.h"
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
        cur_bb_ = g->GetStartBasicBlock();
        cur_bb_id_ = g->BB_START_ID;
    }

    ~GraphBuilder()
    {
    }

    void SetGraph(Graph* g)
    {
        assert(g != nullptr);
        graph_ = g;
    }

    template <typename... Args>
    IdType NewInst(Args&&... args)
    {
        auto inst = graph_->NewInst(args...);
        auto id = inst->GetId();

        cur_inst_ = inst;
        cur_inst_id_ = id;

        inst_map_[id] = inst;

        cur_bb_->PushBackInst(inst);

        return id;
    }

    IdType NewParameter(ArgNumType arg_num)
    {
        auto inst = graph_->NewParam(arg_num);
        auto id = inst->GetId();

        cur_inst_ = inst;
        cur_inst_id_ = id;

        inst_map_[id] = inst;

        return id;
    }

    template <typename T>
    IdType NewConst(T value)
    {
        auto inst = graph_->NewInst(value);
        auto id = inst->GetId();

        cur_inst_ = inst;
        cur_inst_id_ = id;

        inst_map_[id] = inst;

        cur_bb_->PushBackInst(inst);

        return id;
    }

    IdType NewBlock()
    {
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

    void SetSuccessors(IdType bb_id, std::vector<IdType>& succs)
    {
        bb_succ_map_[bb_id] = succs;
    }

    // void NewInst(InstOpcode op);
    template <typename... Args>
    void SetInstInputs(IdType id, Args... inputs_id)
    {
        if constexpr (sizeof...(inputs_id)) {
            AddInput(id, inputs_id...);
        }
    }

    void SetInstType(IdType id, DataType t)
    {
        auto inst = inst_map_[id];
        assert(inst != nullptr);
        inst->SetDataType(t);
    }

    void SetInstImm(IdType id, ImmType imm)
    {
        auto inst = inst_map_[id];
        assert(inst != nullptr);
        inst->SetImm(imm);
    }

    bool ConstructCFG();
    bool ConstructDFG();

  private:
    void AddInput(IdType i_id, IdType id)
    {
        inst_inputs_map_[i_id].push_back(id);
    }

    template <typename T, typename... Args>
    void AddInput(IdType i_id, T id, Args... args)
    {
        inst_inputs_map_[i_id].push_back(id);
        AddInput(i_id, args...);
    }

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
};

#endif