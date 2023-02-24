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
    void ResetState();
    void TryInlineStatic();
    void UpdateDFGParameters();
    void UpdateDFGReturns();
    void MoveConstants();
    void MoveCalleeBlocks();
    void InsertInlinedGraph();

    Inst* cur_call_{ nullptr };
    BasicBlock* callee_start_bb_{ nullptr };
    std::vector<BasicBlock*> ret_bbs_{};
    std::unique_ptr<Inst> ret_phi_{ nullptr };
};

#endif
