#ifndef ___GRAPH_H_INCLUDED___
#define ___GRAPH_H_INCLUDED___

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "inst.h"
#include "ir_isa.h"
#include "typedefs.h"

// class RuntimeInfo;
// class MethodInfo;
// class OptInfo;
// class ArchInfo;

// class Metadata
// {
//     RuntimeInfo inf_runtime;
//     MethodInfo inf_method;
//     OptInfo inf_opt;
//     ArchInfo inf_arch;
// };

class BasicBlock;

class Graph
{
  public:
    static constexpr IdType BB_START_ID = 0;
    static constexpr IdType BB_END_ID = 1;
    Graph()
    {
        bb_vector_.resize(2);
        InitStartBlock();
        InitEndBlock();
    }

    ~Graph()
    {
    }

    static inline IdType GetStartBasicBlockId()
    {
        return BB_START_ID;
    }

    static inline IdType GetEndBasicBlockId()
    {
        return BB_START_ID;
    }

    BasicBlock* GetStartBasicBlock() const
    {
        return bb_start_;
    }

    BasicBlock* GetEndBasicBlock() const
    {
        return bb_end_;
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

  private:
    void InitStartBlock();
    void InitEndBlock();

    std::vector<BasicBlock*> bb_vector_;
    BasicBlock* bb_start_{ nullptr };
    BasicBlock* bb_end_{ nullptr };

    std::unordered_map<IdType, ConstantOp*> const_map_;

    mutable uint64_t inst_id_counter_{};
    uint64_t bb_id_counter_{};

    // Metadata metadata;
};

#endif