#ifndef __PASS_INLINING_INCLUDED__
#define __PASS_INLINING_INCLUDED__

#include <memory>
#include <vector>

#include "graph_visitor.h"
#include "pass.h"

class BasicBlock;
class InstBase;

class Inlining : public Pass, public GraphVisitor
{
  public:
    Inlining(Graph* graph);

    bool RunPass() override;

  private:
    void ResetState();
    void TryInlineStatic();
    void UpdateDFGParameters();
    void UpdateDFGReturns();
    void MoveConstants();
    void MoveCalleeBlocks();
    void InsertInlinedGraph();

    InstBase* cur_call_{ nullptr };
    BasicBlock* callee_start_bb_{ nullptr };
    std::vector<BasicBlock*> ret_bbs_{};
    std::unique_ptr<InstBase> ret_phi_{ nullptr };

  private:
    static void VisitCALL_STATIC(GraphVisitor* v, InstBase* inst);

#include "graph_visitor.inc"
};

#endif
