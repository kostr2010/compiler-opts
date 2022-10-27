#ifndef ___GRAPH_H_INCLUDED___
#define ___GRAPH_H_INCLUDED___

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "analyser.h"
#include "inst.h"
#include "ir_isa.h"
#include "typedefs.h"

class BasicBlock;

class Graph
{
  public:
    static constexpr IdType BB_START_ID = 0;
    Graph() : analyser_{ this }
    {
        InitStartBlock();
    }

    ~Graph()
    {
    }

    static inline IdType GetStartBasicBlockId()
    {
        return BB_START_ID;
    }

    BasicBlock* GetStartBasicBlock() const
    {
        return bb_start_;
    }

    BasicBlock* GetBasicBlock(IdType bb_id) const
    {
        assert(bb_id < bb_vector_.size());
        return bb_vector_.at(bb_id);
    }

    void ClearDominators();

    void AddEdge(IdType from, IdType to)
    {
        assert(from < bb_vector_.size());
        assert(to < bb_vector_.size());

        auto bb_to = bb_vector_.at(to);
        auto bb_from = bb_vector_.at(from);

        AddEdge(bb_from, bb_to);
    }

    void AddEdge(BasicBlock* from, BasicBlock* to);

    BasicBlock* NewBasicBlock();

    template <Opcode op, typename... Args>
    Inst* NewInst(Args&&... args) const
    {
        assert(op != Opcode::CONST);
        assert(op != Opcode::PARAM);

#define CREATE(OPCODE, TYPE)                                                                      \
    if constexpr (op == Opcode::OPCODE) {                                                         \
        auto inst = Inst::NewInst<TYPE>(Opcode::OPCODE, std::forward<Args>(args)...);             \
        inst->SetId(inst_id_counter_++);                                                          \
        return inst;                                                                              \
    }
        INSTRUCTION_LIST(CREATE)
#undef CREATE
    }

#define CREATE_INST(OPCODE, TYPE)                                                                 \
    template <typename... Args>                                                                   \
    TYPE* CreateInst##OPCODE(Args&&... args) const                                                \
    {                                                                                             \
        auto inst = Inst::NewInst<TYPE>(Opcode::OPCODE, std::forward<Args>(args)...);             \
        inst->SetId(inst_id_counter_++);                                                          \
        return inst;                                                                              \
    }
    INSTRUCTION_LIST(CREATE_INST)
#undef CREATE_INST

    template <typename T>
    Inst* NewConst(T val)
    {
        ConstantOp* inst = Inst::NewInst<ConstantOp>(Opcode::CONST, val);
        inst->SetId(inst_id_counter_);
        const_map_[inst_id_counter_++] = inst;
        return inst;
    }

    Inst* NewParam(ArgNumType arg_num);

    void Dump();

    Analyser* GetAnalyser()
    {
        return &analyser_;
    }

  private:
    void InitStartBlock();

    std::vector<BasicBlock*> bb_vector_;
    BasicBlock* bb_start_{ nullptr };

    std::unordered_map<IdType, ConstantOp*> const_map_;

    mutable uint64_t inst_id_counter_{};
    uint64_t bb_id_counter_{};

    Analyser analyser_;

    // Metadata metadata;
};

#endif