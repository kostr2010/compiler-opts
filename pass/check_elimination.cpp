#include "check_elimination.h"
#include "dom_tree.h"
#include "ir/bb.h"
#include "ir/graph.h"
#include "loop_analysis.h"

GEN_DEFAULT_VISIT_FUNCTIONS(CheckElimination, RPO);

static void DeleteCheck(InstBase* check)
{
    ASSERT(check != nullptr);
    ASSERT(check->IsCheck());
    ASSERT(check->GetNumUsers() == 0);
    ASSERT(check->GetBasicBlock() != nullptr);

    for (auto i : check->GetInputs()) {
        ASSERT(i.GetInst() != nullptr);
        i.GetInst()->RemoveUser(check);
    }
    check->GetBasicBlock()->UnlinkInst(check);
}

// returns true if two checks with same check subject are equivalent
using EquivalenceCriterion = bool (*)(const InstBase*, const InstBase*);

template <isa::inst::Opcode OPCODE, typename EquivalenceCriterion>
static void DeleteDominatedChecks(InstBase* inst, EquivalenceCriterion eq_criterion)
{
    ASSERT(inst != nullptr);
    ASSERT(inst->IsCheck());
    ASSERT(inst->GetNumUsers() == 0);
    ASSERT(inst->GetNumInputs() != 0);
    ASSERT(inst->GetInput(0).GetInst() != nullptr);

    auto check_input = inst->GetInput(0).GetInst();

    for (const auto user : check_input->GetUsers()) {
        ASSERT(user.GetInst() != nullptr);
        auto user_inst = user.GetInst();

        if (user_inst->GetOpcode() != OPCODE) {
            continue;
        }

        if (user_inst->GetId() == inst->GetId()) {
            continue;
        }

        ASSERT(inst->GetBasicBlock() != nullptr);
        ASSERT(user_inst->GetBasicBlock() != nullptr);
        // since we are processing instructions in descending order in basic block, we can omit
        // checking dominance for two instructions in the same block
        bool in_same_block = inst->GetBasicBlock()->GetId() == user_inst->GetBasicBlock()->GetId();
        bool dominates = in_same_block || inst->Dominates(user_inst);

        if (dominates && eq_criterion(user_inst, inst)) {
            DeleteCheck(user_inst);
        }
    }
}

template <isa::inst::Opcode OPCODE>
static bool CheckRedundancy(InstBase* check);

bool CheckElimination::Run()
{
    graph_->GetPassManager()->GetValidPass<DomTree>();
    graph_->GetPassManager()->GetValidPass<LoopAnalysis>();

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

static bool AreSameOrSameVal(const InstBase* i1, const InstBase* i2)
{
    if (i1->GetId() == i2->GetId()) {
        return true;
    }

    if (i1->IsConst() && i2->IsConst()) {
        return isa::inst::Inst<isa::inst::Opcode::CONST>::Type::Compare(i1, i2);
    }

    return false;
}

template <isa::inst::Opcode OPCODE>
static bool DefaultEquivalenceCheck(const InstBase* i1, const InstBase* i2)
{
    using T = typename isa::inst::Inst<OPCODE>::Type;
    using NumInputs = isa::InputValue<T, isa::input::Type::VREG>;

    STATIC_ASSERT(!isa::InputValue<T, isa::input::Type::DYN>::value);
    STATIC_ASSERT(isa::HasFlag<OPCODE, isa::flag::Type::CHECK>::value);
    ASSERT(i1->GetOpcode() == OPCODE);
    ASSERT(i2->GetOpcode() == OPCODE);
    ASSERT(i1->IsCheck());
    ASSERT(i2->IsCheck());
    STATIC_ASSERT(NumInputs::value != 0);
    ASSERT(i1->GetNumInputs() == NumInputs::value);
    ASSERT(i2->GetNumInputs() == NumInputs::value);
    ASSERT(i1->GetNumInputs() == i2->GetNumInputs());
    ASSERT(i1->GetId() != i2->GetId());
    ASSERT(i1->GetInput(0).GetInst()->GetId() == i2->GetInput(0).GetInst()->GetId());

    // if check only accepts object to check
    if constexpr (NumInputs::value == 1) {
        return true;
    }

    for (unsigned i = 1; i < i1->GetNumInputs(); ++i) {
        auto in1 = i1->GetInput(i).GetInst();
        auto in2 = i2->GetInput(i).GetInst();
        if (!AreSameOrSameVal(in1, in2)) {
            return false;
        }
    }

    return true;
}

template <>
bool CheckRedundancy<isa::inst::Opcode::CHECK_ZERO>(InstBase* check)
{
    using T = typename isa::inst::Inst<isa::inst::Opcode::CHECK_ZERO>::Type;
    STATIC_ASSERT(!isa::InputValue<T, isa::input::Type::DYN>::value);
    STATIC_ASSERT(isa::HasFlag<isa::inst::Opcode::CHECK_ZERO, isa::flag::Type::CHECK>::value);
    STATIC_ASSERT(isa::InputValue<T, isa::input::Type::VREG>::value != 0);
    ASSERT(check->GetOpcode() == isa::inst::Opcode::CHECK_ZERO);

    auto input = check->GetInput(0).GetInst();
    ASSERT(input != nullptr);

    using ConstT = typename isa::inst::Inst<isa::inst::Opcode::CONST>::Type;
    if (input->IsConst() && !static_cast<ConstT*>(input)->IsZero()) {
        return true;
    }

    return false;
}

void CheckElimination::VisitCHECK_ZERO(GraphVisitor* v, InstBase* inst)
{
    DeleteDominatedChecks<isa::inst::Opcode::CHECK_ZERO>(
        inst, DefaultEquivalenceCheck<isa::inst::Opcode::CHECK_ZERO>);

    if (CheckRedundancy<isa::inst::Opcode::CHECK_ZERO>(inst)) {
        static_cast<CheckElimination*>(v)->redundant_checks_.push_back(inst);
    }
}

template <>
bool CheckRedundancy<isa::inst::Opcode::CHECK_NULL>(InstBase* check)
{
    using T = typename isa::inst::Inst<isa::inst::Opcode::CHECK_NULL>::Type;
    STATIC_ASSERT(!isa::InputValue<T, isa::input::Type::DYN>::value);
    STATIC_ASSERT(isa::InputValue<T, isa::input::Type::VREG>::value != 0);
    STATIC_ASSERT(isa::HasFlag<isa::inst::Opcode::CHECK_NULL, isa::flag::Type::CHECK>::value);
    ASSERT(check->GetOpcode() == isa::inst::Opcode::CHECK_NULL);

    auto input = check->GetInput(0).GetInst();
    ASSERT(input != nullptr);

    using ConstT = typename isa::inst::Inst<isa::inst::Opcode::CONST>::Type;
    if (input->IsConst() && !static_cast<ConstT*>(input)->IsNull()) {
        return true;
    }

    return false;
}

void CheckElimination::VisitCHECK_NULL(GraphVisitor* v, InstBase* inst)
{
    DeleteDominatedChecks<isa::inst::Opcode::CHECK_NULL>(
        inst, DefaultEquivalenceCheck<isa::inst::Opcode::CHECK_NULL>);

    if (CheckRedundancy<isa::inst::Opcode::CHECK_NULL>(inst)) {
        static_cast<CheckElimination*>(v)->redundant_checks_.push_back(inst);
    }
}

void CheckElimination::VisitCHECK_SIZE([[maybe_unused]] GraphVisitor* v, InstBase* inst)
{
    ASSERT(inst->GetOpcode() == isa::inst::Opcode::CHECK_SIZE);

    DeleteDominatedChecks<isa::inst::Opcode::CHECK_SIZE>(
        inst, DefaultEquivalenceCheck<isa::inst::Opcode::CHECK_SIZE>);

    // TODO: analyze if statements - requires range analysis
}
