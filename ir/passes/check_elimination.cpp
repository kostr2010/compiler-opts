#include "check_elimination.h"
#include "bb.h"
#include "graph.h"

#include "dom_tree.h"
#include "loop_analysis.h"

GEN_VISIT_FUNCTIONS_WITH_BLOCK_ORDER(CheckElimination, RPO);

static void DeleteCheck(Inst* check)
{
    assert(check != nullptr);
    assert(check->IsCheck());
    assert(check->GetNumUsers() == 0);
    assert(check->GetBasicBlock() != nullptr);

    for (auto i : check->GetInputs()) {
        assert(i.GetInst() != nullptr);
        i.GetInst()->RemoveUser(check);
    }
    check->GetBasicBlock()->UnlinkInst(check);
}

// returns true if two checks with same check subject are equivalent
using EquivalenceCriterion = bool (*)(const Inst*, const Inst*);

template <Inst::Opcode OPCODE, typename EquivalenceCriterion>
static void DeleteDominatedChecks(Inst* inst, EquivalenceCriterion eq_criterion)
{
    assert(inst != nullptr);
    assert(inst->IsCheck());
    assert(inst->GetNumUsers() == 0);
    assert(inst->GetNumInputs() != 0);
    assert(inst->GetInput(0).GetInst() != nullptr);

    auto check_input = inst->GetInput(0).GetInst();

    for (const auto user : check_input->GetUsers()) {
        assert(user.GetInst() != nullptr);
        auto user_inst = user.GetInst();

        if (user_inst->GetOpcode() != OPCODE) {
            continue;
        }

        if (user_inst->GetId() == inst->GetId()) {
            continue;
        }

        assert(inst->GetBasicBlock() != nullptr);
        assert(user_inst->GetBasicBlock() != nullptr);
        // since we are processing instructions in descending order in basic block, we can omit
        // checking dominance for two instructions in the same block
        bool in_same_block = inst->GetBasicBlock()->GetId() == user_inst->GetBasicBlock()->GetId();
        bool dominates = in_same_block || inst->Dominates(user_inst);

        if (dominates && eq_criterion(user_inst, inst)) {
            DeleteCheck(user_inst);
        }
    }
}

template <Inst::Opcode OPCODE>
static bool CheckRedundancy(Inst* check);

bool CheckElimination::RunPass()
{
    graph_->GetAnalyser()->GetValidPass<DomTree>();
    graph_->GetAnalyser()->GetValidPass<LoopAnalysis>();

    VisitGraph();
    RemoveRedundantChecks();
    ResetStructs();

    return true;
}

void CheckElimination::RemoveRedundantChecks()
{
    for (const auto i : redundant_checks_) {
        DeleteCheck(i);
    }
}

void CheckElimination::ResetStructs()
{
    redundant_checks_.clear();
}

static bool AreSameOrSameVal(const Inst* i1, const Inst* i2)
{
    if (i1->GetId() == i2->GetId()) {
        return true;
    }

    if (i1->IsConst() && i2->IsConst()) {
        return Inst::to_inst_type<Inst::Opcode::CONST>::Compare(i1, i2);
    }

    return false;
}

template <Inst::Opcode OPCODE>
static bool DefaultEquivalenceCheck(const Inst* i1, const Inst* i2)
{
    using T = Inst::to_inst_type<OPCODE>;
    static_assert(!Inst::has_dynamic_operands<OPCODE>());

    assert(i1->GetOpcode() == OPCODE);
    assert(i2->GetOpcode() == OPCODE);
    assert(i1->IsCheck());
    assert(i2->IsCheck());
    static_assert(Inst::get_num_inputs<T>() != 0);
    assert(i1->GetNumInputs() == Inst::get_num_inputs<T>());
    assert(i2->GetNumInputs() == Inst::get_num_inputs<T>());
    assert(i1->GetNumInputs() == i2->GetNumInputs());
    assert(i1->GetId() != i2->GetId());
    assert(i1->GetInput(0).GetInst()->GetId() == i2->GetInput(0).GetInst()->GetId());

    // if check only accepts object to check
    if constexpr (Inst::get_num_inputs<T>() == 1) {
        return true;
    }

    for (size_t i = 1; i < i1->GetNumInputs(); ++i) {
        auto in1 = i1->GetInput(i).GetInst();
        auto in2 = i2->GetInput(i).GetInst();
        if (!AreSameOrSameVal(in1, in2)) {
            return false;
        }
    }

    return true;
}

template <>
bool CheckRedundancy<Inst::Opcode::CHECK_ZERO>(Inst* check)
{
    using T = Inst::to_inst_type<Inst::Opcode::CHECK_ZERO>;
    static_assert(!Inst::has_dynamic_operands<Inst::Opcode::CHECK_ZERO>());
    static_assert(Inst::get_num_inputs<T>() != 0);
    assert(Inst::Opcode::CHECK_ZERO == check->GetOpcode());
    assert(Inst::get_num_inputs<T>() == check->GetNumInputs());

    auto input = check->GetInput(0).GetInst();
    assert(input != nullptr);

    using ConstT = Inst::to_inst_type<Inst::Opcode::CONST>;
    if (input->IsConst() && !static_cast<ConstT*>(input)->IsZero()) {
        return true;
    }

    return false;
}

void CheckElimination::VisitCHECK_ZERO(GraphVisitor* v, Inst* inst)
{
    DeleteDominatedChecks<Inst::Opcode::CHECK_ZERO>(
        inst, DefaultEquivalenceCheck<Inst::Opcode::CHECK_ZERO>);

    if (CheckRedundancy<Inst::Opcode::CHECK_ZERO>(inst)) {
        static_cast<CheckElimination*>(v)->redundant_checks_.push_back(inst);
    }
}

template <>
bool CheckRedundancy<Inst::Opcode::CHECK_NULL>(Inst* check)
{
    using T = Inst::to_inst_type<Inst::Opcode::CHECK_NULL>;
    static_assert(!Inst::has_dynamic_operands<Inst::Opcode::CHECK_NULL>());
    static_assert(Inst::get_num_inputs<T>() != 0);
    assert(Inst::Opcode::CHECK_NULL == check->GetOpcode());
    assert(Inst::get_num_inputs<T>() == check->GetNumInputs());

    auto input = check->GetInput(0).GetInst();
    assert(input != nullptr);

    using ConstT = Inst::to_inst_type<Inst::Opcode::CONST>;
    if (input->IsConst() && !static_cast<ConstT*>(input)->IsNull()) {
        return true;
    }

    return false;
}

void CheckElimination::VisitCHECK_NULL(GraphVisitor* v, Inst* inst)
{
    DeleteDominatedChecks<Inst::Opcode::CHECK_NULL>(
        inst, DefaultEquivalenceCheck<Inst::Opcode::CHECK_NULL>);

    if (CheckRedundancy<Inst::Opcode::CHECK_NULL>(inst)) {
        static_cast<CheckElimination*>(v)->redundant_checks_.push_back(inst);
    }
}

void CheckElimination::VisitCHECK_SIZE([[maybe_unused]] GraphVisitor* v, Inst* inst)
{
    using T = Inst::to_inst_type<Inst::Opcode::CHECK_SIZE>;
    static_assert(!Inst::has_dynamic_operands<Inst::Opcode::CHECK_SIZE>());
    static_assert(Inst::get_num_inputs<T>::value != 0);

    DeleteDominatedChecks<Inst::Opcode::CHECK_SIZE>(
        inst, DefaultEquivalenceCheck<Inst::Opcode::CHECK_SIZE>);

    // TODO: analyze if statements - requires range analysis
}
