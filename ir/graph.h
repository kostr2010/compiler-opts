#ifndef ___GRAPH_H_INCLUDED___
#define ___GRAPH_H_INCLUDED___

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
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
        return bb_vector_.at(bb_id).get();
    }

    void ClearDominators();

    void AddEdge(IdType from, IdType to)
    {
        auto bb_to = bb_vector_.at(to).get();
        auto bb_from = bb_vector_.at(from).get();

        AddEdge(bb_from, bb_to);
    }

    void AddEdge(BasicBlock* from, BasicBlock* to);

    BasicBlock* NewBasicBlock();

    void Dump();

    Analyser* GetAnalyser()
    {
        return &analyser_;
    }

  private:
    void InitStartBlock();

    std::vector<std::unique_ptr<BasicBlock> > bb_vector_;
    BasicBlock* bb_start_{ nullptr };

    IdType bb_id_counter_{};

    Analyser analyser_;

    // Metadata metadata;
};

#endif