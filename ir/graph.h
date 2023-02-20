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
    DEFAULT_DTOR(Graph);

    BasicBlock* GetStartBasicBlock() const;
    BasicBlock* GetBasicBlock(IdType bb_id) const;

    void ClearDominators();
    void ClearLoops();

    void AddEdge(IdType from, IdType to);
    void AddEdge(BasicBlock* from, BasicBlock* to);

    void RemoveEdge(IdType from, IdType to);
    void RemoveEdge(BasicBlock* from, BasicBlock* to);

    void InsertBasicBlock(BasicBlock* bb, BasicBlock* from, BasicBlock* to);
    void InsertBasicBlockBefore(BasicBlock* bb, BasicBlock* before);
    void InsertBasicBlockAfter(BasicBlock* bb, BasicBlock* after);
    void AppendBasicBlock(BasicBlock* first, BasicBlock* second);

    BasicBlock* SplitBasicBlock(Inst* inst_after);

    BasicBlock* NewBasicBlock();
    BasicBlock* ReleaseBasicBlock(IdType id);
    IdType NewBasicBlock(BasicBlock* bb);

    void Dump();

    Analyser* GetAnalyser()
    {
        return &analyser_;
    }

  private:
    void InitStartBlock();

    std::vector<std::unique_ptr<BasicBlock> > bb_vector_;

    IdType bb_id_counter_{};

    Analyser analyser_;

    // Metadata metadata;
};

#endif
