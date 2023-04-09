#include "graph_visitor.h"
#include "inst.h"

void GraphVisitor::VisitDefault(Inst* inst)
{
    return (void)(inst);
}

#define GEN_INST_TYPE_VISITORS_IMPL(t)                                                            \
    void GraphVisitor::VisitInstType(t* i)                                                        \
    {                                                                                             \
        return VisitDefault(i);                                                                   \
    }
INSTRUCTION_TYPES(GEN_INST_TYPE_VISITORS_IMPL)
#undef GEN_INST_TYPE_VISITORS_IMPL

#define GEN_OPCODE_VISITORS_IMPL(OPCODE, ...)                                                     \
    void GraphVisitor::Visit##OPCODE(GraphVisitor* v, Inst* i)                                    \
    {                                                                                             \
        v->VisitDefault(i);                                                                       \
    }
INSTRUCTION_LIST(GEN_OPCODE_VISITORS_IMPL)
#undef GEN_OPCODE_VISITORS_IMPL