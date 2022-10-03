#ifndef ___BASICBLOCK_H_INCLUDED___
#define ___BASICBLOCK_H_INCLUDED___

#include <algorithm>
#include <cassert>
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
        if (is_empty) {
            SetFirstInst(inst);
        } else {
            inst->SetNext(nullptr);
            inst->SetPrev(last_inst_);
            last_inst_->SetNext(inst);
        }
    }

    void PushFrontInst(Inst* inst)
    {
        assert(inst != nullptr);

        inst->SetBasicBlock(this);
        if (is_empty) {
            SetFirstInst(inst);
        } else {
            inst->SetNext(first_inst_);
            inst->SetPrev(nullptr);
            first_inst_->SetPrev(inst);
            first_inst_ = inst;
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

  private:
    void SetFirstInst(Inst* inst)
    {
        assert(inst != nullptr);

        inst->SetNext(nullptr);
        inst->SetPrev(nullptr);

        first_inst_ = inst;
        last_inst_ = inst;

        is_empty = false;
    }

    bool is_empty = true;

    Graph* graph_; // access to metadata

    std::vector<BasicBlock*> preds_{}; // predecessors
    std::vector<BasicBlock*> succs_{}; // successors
    std::vector<BasicBlock*> dom_{};   // dominates
    BasicBlock* dominator_{ nullptr };

    IdType id_;

    std::vector<Inst*> inst_vector_{};
    Inst* first_inst_{ nullptr };
    Inst* last_inst_{ nullptr };
};

#endif