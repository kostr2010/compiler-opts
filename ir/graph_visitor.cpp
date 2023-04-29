#include "graph_visitor.h"
#include "inst.h"

void GraphVisitor::VisitDefault(InstBase* inst)
{
    return (void)(inst);
}

#define GEN_INST_TYPE_VISITORS_IMPL(TYPE, ...)                                                    \
    void GraphVisitor::VisitInstType(isa::inst_type::TYPE* i)                                     \
    {                                                                                             \
        return VisitDefault(i);                                                                   \
    }
ISA_INSTRUCTION_TYPE_LIST(GEN_INST_TYPE_VISITORS_IMPL)
#undef GEN_INST_TYPE_VISITORS_IMPL

#define GEN_OPCODE_VISITORS_IMPL(OPCODE, ...)                                                     \
    void GraphVisitor::Visit##OPCODE(GraphVisitor* v, InstBase* i)                                \
    {                                                                                             \
        v->VisitDefault(i);                                                                       \
    }
ISA_INSTRUCTION_LIST(GEN_OPCODE_VISITORS_IMPL)
#undef GEN_OPCODE_VISITORS_IMPL