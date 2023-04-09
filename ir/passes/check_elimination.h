#ifndef __CHECK_ELIMINATION_H_INCLUDED__
#define __CHECK_ELIMINATION_H_INCLUDED__

#include "graph_visitor.h"
#include "ir_isa.h"
#include "pass.h"

#include <vector>

class BasicBlock;
class Inst;

class CheckElimination : public Pass, public GraphVisitor
{
  public:
    CheckElimination(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

  private:
    void ResetState();

    std::vector<Inst*> zero_checks_{};
    std::vector<Inst*> null_checks_{};
    std::vector<Inst*> size_checks_{};

#include "graph_visitor.inc"
};

#endif