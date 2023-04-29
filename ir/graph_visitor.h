#ifndef __GRAPH_VISITOR_H_INCLUDED__
#define __GRAPH_VISITOR_H_INCLUDED__

#include <vector>

#include "isa/isa.h"
#include "utils/macros.h"

class Graph;
class BasicBlock;
class InstBase;

class GraphVisitor
{
  public:
    DEFAULT_CTOR(GraphVisitor);

    virtual std::vector<BasicBlock*> BlockOrder() const = 0;

    virtual void VisitGraph() = 0;
    virtual void VisitBasicBlock(BasicBlock* bb) = 0;
    virtual void VisitInstruction(InstBase* inst) = 0;

#define GEN_INST_TYPE_VISITORS_DECL(TYPE, ...) virtual void VisitInstType(isa::inst_type::TYPE* i);
    ISA_INSTRUCTION_TYPE_LIST(GEN_INST_TYPE_VISITORS_DECL)
#undef GEN_INST_TYPE_VISITORS_DECL

  protected:
    virtual void VisitDefault(InstBase* inst);

#define GEN_OPCODE_VISITORS_DECL(OPCODE, ...)                                                     \
    static void Visit##OPCODE(GraphVisitor* v, InstBase* i);
    ISA_INSTRUCTION_LIST(GEN_OPCODE_VISITORS_DECL)
#undef GEN_OPCODE_VISITORS_DECL
};

#endif