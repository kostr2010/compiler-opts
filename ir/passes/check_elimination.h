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

    static void VisitCHECK_ZERO(GraphVisitor* v, Inst* inst);
    static void VisitCHECK_NULL(GraphVisitor* v, Inst* inst);
    static void VisitCHECK_SIZE(GraphVisitor* v, Inst* inst);

    std::vector<Inst*> CHECK_ZERO_rpo_{};
    std::vector<Inst*> CHECK_NULL_rpo_{};
    std::vector<Inst*> CHECK_SIZE_rpo_{};

#include "graph_visitor.inc"
};

#endif