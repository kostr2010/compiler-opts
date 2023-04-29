#ifndef __PEEPHOLES_H_INCLUDED__
#define __PEEPHOLES_H_INCLUDED__

#include "ir/graph_visitor.h"
#include "pass.h"

class BasicBlock;
class InstBase;

class Peepholes : public Pass, public GraphVisitor
{
  public:
    Peepholes(Graph* graph) : Pass(graph)
    {
    }

    bool RunPass() override;

  private:
    void ReplaceWithIntegralConst(InstBase* inst, int64_t val);

    bool FoldADD(InstBase* i);

    // ADD v0, 0
    // ->
    // v0
    bool MatchADD_zero(InstBase* i);

    // 1. ADD v0, v4
    // 2. SUB v1, v4
    // ->
    // 1. ADD v0, v4
    // 2. v0
    bool MatchADD_after_sub(InstBase* i);

    // 1. ADD v0, v0
    // ->
    // 1. SHLI v0, 2
    bool MatchADD_same_value(InstBase* i);

    bool FoldASHR(InstBase* i);

    // 1. ASHR v0, 0
    // ->
    // 1. v0
    bool MatchASHR_zero_0(InstBase* i);

    // 1. ASHR 0, v0
    // ->
    // 1. CONST 0
    bool MatchASHR_zero_1(InstBase* i);

    bool FoldXOR(InstBase* i);

    // 1. XOR v0, v0
    // ->
    // 1. CONST 0
    bool MatchXOR_same_value(InstBase* i);

    // 1. XOR v0, 0
    // ->
    // 1. v0
    bool MatchXOR_zero(InstBase* i);

  private:
    static void VisitADD(GraphVisitor* v, InstBase* inst);
    static void VisitXOR(GraphVisitor* v, InstBase* inst);
    static void VisitASHR(GraphVisitor* v, InstBase* inst);

#include "ir/graph_visitor.inc"
};

#endif