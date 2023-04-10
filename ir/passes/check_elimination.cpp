#include "check_elimination.h"
#include "bb.h"
#include "graph.h"

#include "dom_tree.h"
#include "loop_analysis.h"

GEN_VISIT_FUNCTIONS_WITH_BLOCK_ORDER(CheckElimination, RPO);

using EqualityCriterion = bool (*)(const Inst*, const Inst*);

template <Inst::Opcode OPCODE, typename EqualityCriterion>
static void DeleteDominatedChecks(Inst* inst, EqualityCriterion eq_criterion)
{
    // for (auto& user : inst->GetInput(0).GetInst()->GetUsers()) {
    //     auto user_inst = user.GetInst();
    //     // NOLINTNEXTLINE(readability-magic-numbers)
    //     if (user_inst->GetOpcode() == OPC && user_inst != inst &&
    //         user_inst->GetType() == inst->GetType() && check_inputs(user_inst) &&
    //         inst->InSameBlockOrDominate(user_inst)) {
    //         ASSERT(inst->IsDominate(user_inst));
    //         COMPILER_LOG(DEBUG, CHECKS_ELIM)
    //             // NOLINTNEXTLINE(readability-magic-numbers)
    //             << GetOpcodeString(OPC) << " with id = " << inst->GetId() << " dominate on "
    //             << GetOpcodeString(OPC) << " with id = " << user_inst->GetId();
    //         ReplaceUsersAndRemoveCheck(user_inst, inst);
    //     }
    // }
}

bool CheckElimination::RunPass()
{
    graph_->GetAnalyser()->GetValidPass<DomTree>();
    graph_->GetAnalyser()->GetValidPass<LoopAnalysis>();

    ResetState();
    VisitGraph();

    return true;
}

void CheckElimination::ResetState()
{
    CHECK_ZERO_rpo_.clear();
    CHECK_NULL_rpo_.clear();
    CHECK_SIZE_rpo_.clear();
}

#define GEN_VISIT_CHECK(name)                                                                     \
    void CheckElimination::Visit##name(GraphVisitor* v, Inst* inst)                               \
    {                                                                                             \
        static_cast<CheckElimination*>(v)->name##_rpo_.push_back(inst);                           \
    }

GEN_VISIT_CHECK(CHECK_ZERO);
GEN_VISIT_CHECK(CHECK_NULL);
GEN_VISIT_CHECK(CHECK_SIZE);

#undef GEN_VISIT_CHECK