#ifndef __PEEPHOLES_H_INCLUDED__
#define __PEEPHOLES_H_INCLUDED__

#include "pass.h"

class BasicBlock;
class Inst;

class Peepholes : public Pass
{
  public:
    Peepholes(Graph* graph) : Pass(graph)
    {
    }
    DEFAULT_DTOR(Peepholes);

    bool RunPass() override;

  private:
    void MatchADD(Inst* i);
    bool FoldADD(Inst* i);

    // ADD v0, 0
    // ->
    // v0
    bool MatchADD_zero(Inst* i);

    // 1. ADD v0, v4
    // 2. SUB v1, v4
    // ->
    // 1. ADD v0, v4
    // 2. v0
    bool MatchADD_after_sub(Inst* i);

    // 1. ADD v0, v0
    // ->
    // 1. SHLI v0, 2
    bool MatchADD_same_value(Inst* i);

    void MatchASHR(Inst* i);
    bool FoldASHR(Inst* i);

    // 1. ASHR v0, 0
    // ->
    // 1. v0
    bool MatchASHR_zero_0(Inst* i);

    // 1. ASHR 0, v0
    // ->
    // 1. CONST 0
    bool MatchASHR_zero_1(Inst* i);

    void MatchXOR(Inst* i);
    bool FoldXOR(Inst* i);

    // 1. XOR v0, v0
    // ->
    // 1. CONST 0
    bool MatchXOR_same_value(Inst* i);

    // 1. XOR v0, 0
    // ->
    // 1. v0
    bool MatchXOR_zero(Inst* i);
};

#endif