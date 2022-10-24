#ifndef ___GRAPH_H_INCLUDED___
#define ___GRAPH_H_INCLUDED___

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

    template <typename T>
    T* GetPass()
    {
        return analyser_.GetPass<T>();
    }

    template <typename T>
    bool RunPass()
    {
        return analyser_.RunPass<T>();
    }

    std::vector<BasicBlock*> RPOPass() const
    {
        std::vector<BasicBlock*> rpo_bb{};
        std::unordered_set<IdType> rpo_visited{};
        RPOPass_(&rpo_bb, &rpo_visited, GetStartBasicBlock());
        return rpo_bb;
    }

  private:
    void InitStartBlock();

    void RPOPass_(std::vector<BasicBlock*>* rpo_bb, std::unordered_set<IdType>* rpo_visited,
                  BasicBlock* cur_bb) const;

    std::vector<BasicBlock*> bb_vector_;
    BasicBlock* bb_start_{ nullptr };

    std::unordered_map<IdType, ConstantOp*> const_map_;

    mutable uint64_t inst_id_counter_{};
    uint64_t bb_id_counter_{};

    Analyser analyser_;

    // Metadata metadata;
};

#endif