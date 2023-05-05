#ifndef __CHECK_ELIMINATION_H_INCLUDED__
#define __CHECK_ELIMINATION_H_INCLUDED__

#include "ir/graph_visitor.h"
#include "pass.h"

#include <vector>

class BasicBlock;
class InstBase;

class CheckElimination : public Pass, public GraphVisitor
{
  public:
    CheckElimination(Graph* graph) : Pass(graph)
    {
    }

    bool Run() override;

  private:
    void RemoveRedundantChecks();
    void ResetStructs();

    static void VisitCHECK_ZERO(GraphVisitor* v, InstBase* inst);
    static void VisitCHECK_NULL(GraphVisitor* v, InstBase* inst);
    static void VisitCHECK_SIZE([[maybe_unused]] GraphVisitor* v, InstBase* inst);

    std::vector<InstBase*> redundant_checks_;

#include "ir/graph_visitor.inc"
};

#endif