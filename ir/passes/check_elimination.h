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
    void RemoveRedundantChecks();
    void ResetStructs();

    static void VisitCHECK_ZERO(GraphVisitor* v, Inst* inst);
    static void VisitCHECK_NULL(GraphVisitor* v, Inst* inst);
    static void VisitCHECK_SIZE([[maybe_unused]] GraphVisitor* v, Inst* inst);

    std::vector<Inst*> redundant_checks_;

#include "graph_visitor.inc"
};

#endif