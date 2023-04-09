#ifndef __GRAPH_VISITOR_H_INCLUDED__
#define __GRAPH_VISITOR_H_INCLUDED__

#include <vector>

#include "ir_isa.h"
#include "macros.h"

class Graph;
class BasicBlock;
class Inst;

#define GEN_INST_TYPES_FORWARD_DECL(t) class t;
INSTRUCTION_TYPES(GEN_INST_TYPES_FORWARD_DECL)
#undef GEN_INST_TYPES_FORWARD_DECL

class GraphVisitor
{
  public:
    DEFAULT_CTOR(GraphVisitor);

    virtual std::vector<BasicBlock*> BlockOrder() const = 0;

    virtual void VisitGraph() = 0;
    virtual void VisitBasicBlock(BasicBlock* bb) = 0;
    virtual void VisitInstruction(Inst* inst) = 0;

#define GEN_INST_TYPE_VISITORS_DECL(t) virtual void VisitInstType(t* i);
    INSTRUCTION_TYPES(GEN_INST_TYPE_VISITORS_DECL)
#undef GEN_INST_TYPE_VISITORS_DECL

  protected:
    virtual void VisitDefault(Inst* inst);

#define GEN_OPCODE_VISITORS_DECL(OPCODE, ...) static void Visit##OPCODE(GraphVisitor* v, Inst* i);
    INSTRUCTION_LIST(GEN_OPCODE_VISITORS_DECL)
#undef GEN_OPCODE_VISITORS_DECL
};

#endif