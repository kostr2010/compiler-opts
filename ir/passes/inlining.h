#ifndef __PASS_INLINING_INCLUDED__
#define __PASS_INLINING_INCLUDED__

#include <memory>
#include <vector>

#include "pass.h"

class BasicBlock;
class Inst;

class Inlining : public Pass
{
  public:
    Inlining(Graph* graph);

    bool RunPass() override;

  private:
    void TryInlineStatic(Inst* inst);
    void UpdateDFGParameters(Inst* inst);
    void UpdateDFGReturns(Inst* inst);
    void MoveConstants(Graph* callee);
    void MoveCalleeBlocks(Graph* callee);
    void InsertInlinedGraph(Inst* call);

    BasicBlock* callee_start_bb_;
    std::vector<BasicBlock*> ret_bbs_;
    std::unique_ptr<Inst> ret_phi_;
};

#endif
