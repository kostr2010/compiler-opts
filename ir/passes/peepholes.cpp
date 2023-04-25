#include "peepholes.h"
#include "bb.h"
#include "graph.h"
#include "inst_.h"

static void TransferUsers(InstBase* from, InstBase* to)
{
    assert(from != nullptr);
    assert(to != nullptr);

    for (const auto& user : from->GetUsers()) {
        to->AddUser(user);
        user.GetInst()->ReplaceInput(from, to);
        from->RemoveUser(user);
    }
}

// 1. BINOP v0, CONST or BINOP CONST, v0
// ->
// 1. BINOPIMM v0, CONST_VAL
template <isa::inst::Opcode OPCODE_FROM, isa::inst::Opcode OPCODE_TO>
static bool FoldBinOpToBinImmOp(InstBase* i)
{
    static_assert(
        std::is_same<typename isa::inst::Inst<OPCODE_TO>::Type, isa::inst_type::BIN_IMM>::value);
    static_assert(
        std::is_same<typename isa::inst::Inst<OPCODE_FROM>::Type, isa::inst_type::BINARY>::value);

    using BinaryNumInputs = isa::InputValue<isa::inst_type::BINARY, isa::input::VREG>;

    assert(i != nullptr);
    assert(i->GetOpcode() == OPCODE_FROM);
    assert(i->GetNumInputs() == BinaryNumInputs::value);

    std::array inputs = { i->GetInput(0).GetInst(), i->GetInput(1).GetInst() };

    assert(inputs[0] != nullptr);
    assert(inputs[1] != nullptr);

    InstBase* input_const = nullptr;
    InstBase* input_param = nullptr;

    if constexpr (isa::HasFlag<OPCODE_FROM, isa::flag::Type::SYMMETRICAL>::value) {
        if (!inputs[0]->IsConst() && !inputs[1]->IsConst()) {
            return false;
        }

        bool input_1_is_const = inputs[1]->IsConst();

        input_const = inputs[input_1_is_const];
        input_param = inputs[!input_1_is_const];
    } else {
        if (!inputs[1]->IsConst()) {
            return false;
        }

        input_const = inputs[1];
        input_param = inputs[0];
    }

    assert(input_const->IsConst());
    auto const_op = static_cast<isa::inst_type::CONST*>(input_const);
    assert(const_op->GetDataType() == InstBase::DataType::INT);

    i->GetBasicBlock()->InsertInstAfter(InstBase::NewInst<OPCODE_TO>(), i);
    auto new_inst = i->GetNext();
    new_inst->SetInput(0, input_param);
    static_cast<isa::inst_type::BIN_IMM*>(new_inst)->SetImm(0, const_op->GetValInt());
    TransferUsers(i, new_inst);

    return true;
}

GEN_VISIT_FUNCTIONS_WITH_BLOCK_ORDER(Peepholes, RPO);

bool Peepholes::RunPass()
{
    VisitGraph();

    return true;
}

void Peepholes::ReplaceWithIntegralConst(InstBase* inst, int64_t val)
{
    graph_->GetStartBasicBlock()->PushBackInst(InstBase::NewInst<isa::inst::Opcode::CONST>(val));
    auto new_inst = graph_->GetStartBasicBlock()->GetLastInst();

    TransferUsers(inst, new_inst);
}

void Peepholes::VisitADD(GraphVisitor* v, InstBase* inst)
{
    assert(inst != nullptr);
    assert(inst->GetOpcode() == isa::inst::Opcode::ADD);
    assert(v != nullptr);

    auto _this = static_cast<Peepholes*>(v);

    if (_this->FoldADD(inst)) {
        return;
    }

    if (_this->MatchADD_zero(inst)) {
        return;
    }

    if (_this->MatchADD_after_sub(inst)) {
        return;
    }

    if (_this->MatchADD_same_value(inst)) {
        return;
    }

    if (FoldBinOpToBinImmOp<isa::inst::Opcode::ADD, isa::inst::Opcode::ADDI>(inst)) {
        return;
    }
}

bool Peepholes::FoldADD(InstBase* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == isa::inst::Opcode::ADD);

    using NumInputs = isa::InputValue<typename isa::inst::Inst<isa::inst::Opcode::ADD>::Type,
                                      isa::input::Type::VREG>;
    assert(i->GetNumInputs() == NumInputs::value);

    std::array inputs = { i->GetInput(0).GetInst(), i->GetInput(1).GetInst() };

    bool is_foldable = (inputs[0]->IsConst() && inputs[1]->IsConst());
    bool is_integral = (inputs[0]->GetDataType() == InstBase::DataType::INT &&
                        inputs[1]->GetDataType() == InstBase::DataType::INT);

    if (is_foldable && is_integral) {
        auto val1 = static_cast<isa::inst_type::CONST*>(inputs[0])->GetValInt();
        auto val2 = static_cast<isa::inst_type::CONST*>(inputs[1])->GetValInt();
        auto res = val1 + val2;
        ReplaceWithIntegralConst(i, res);
        return true;
    }

    return false;
}

// 1. ADD v0, 0 / ADD 0, v0
// ->
// v0
bool Peepholes::MatchADD_zero(InstBase* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == isa::inst::Opcode::ADD);

    using NumInputs = isa::InputValue<typename isa::inst::Inst<isa::inst::Opcode::ADD>::Type,
                                      isa::input::Type::VREG>;
    assert(i->GetNumInputs() == NumInputs::value);

    std::array inputs = { i->GetInput(0).GetInst(), i->GetInput(1).GetInst() };

    if (inputs[1]->IsConst() && (static_cast<isa::inst_type::CONST*>(inputs[1])->IsZero())) {
        TransferUsers(i, inputs[0]);
        return true;
    } else if (inputs[0]->IsConst() &&
               (static_cast<isa::inst_type::CONST*>(inputs[0])->IsZero())) {
        TransferUsers(i, inputs[1]);
        return true;
    }

    return false;
}

static bool TryMatchADD_after_sub(InstBase* add, InstBase* sub)
{
    assert(add != nullptr);
    assert(sub != nullptr);
    assert(sub->GetOpcode() == isa::inst::Opcode::SUB);

    using NumInputsAdd = isa::InputValue<typename isa::inst::Inst<isa::inst::Opcode::ADD>::Type,
                                         isa::input::Type::VREG>;
    assert(add->GetNumInputs() == NumInputsAdd::value);

    std::array inputs_add = { add->GetInput(0).GetInst(), add->GetInput(1).GetInst() };

    using NumInputsSub = isa::InputValue<typename isa::inst::Inst<isa::inst::Opcode::SUB>::Type,
                                         isa::input::Type::VREG>;
    assert(sub->GetNumInputs() == NumInputsSub::value);

    std::array inputs_sub = { sub->GetInput(0).GetInst(), sub->GetInput(1).GetInst() };

    if (!(inputs_sub[1]->GetId() == inputs_add[0]->GetId() ||
          inputs_sub[1]->GetId() == inputs_add[1]->GetId())) {
        return false;
    }

    bool is_input_1_common_arg = inputs_sub[1]->GetId() == inputs_add[1]->GetId();
    auto sub_common_arg = inputs_sub[is_input_1_common_arg];

    TransferUsers(add, sub_common_arg);

    return true;
}

// 1. SUB v0, v4
// 2. ADD v1, v4 / ADD v4, v1
// ->
// 1. SUB v0, v4
// 2. v0
bool Peepholes::MatchADD_after_sub(InstBase* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == isa::inst::Opcode::ADD);

    using NumInputs = isa::InputValue<typename isa::inst::Inst<isa::inst::Opcode::ADD>::Type,
                                      isa::input::Type::VREG>;
    assert(i->GetNumInputs() == NumInputs::value);

    std::array inputs = { i->GetInput(0).GetInst(), i->GetInput(1).GetInst() };

    if (inputs[0]->GetOpcode() == isa::inst::Opcode::SUB && TryMatchADD_after_sub(i, inputs[0])) {
        return true;
    }

    if (inputs[1]->GetOpcode() == isa::inst::Opcode::SUB && TryMatchADD_after_sub(i, inputs[1])) {
        return true;
    }

    return false;
}

// 1. ADD v0, v0
// ->
// 1. SHLI v0, 2
bool Peepholes::MatchADD_same_value(InstBase* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == isa::inst::Opcode::ADD);

    using NumInputs = isa::InputValue<typename isa::inst::Inst<isa::inst::Opcode::ADD>::Type,
                                      isa::input::Type::VREG>;
    assert(i->GetNumInputs() == NumInputs::value);

    std::array inputs = { i->GetInput(0).GetInst(), i->GetInput(1).GetInst() };

    if (inputs[0]->GetId() == inputs[1]->GetId()) {
        auto bb = i->GetBasicBlock();
        bb->InsertInstAfter(InstBase::NewInst<isa::inst::Opcode::SHLI>(), i);
        auto new_inst = i->GetNext();
        new_inst->SetInput(0, inputs[0]);
        static_cast<isa::inst_type::BIN_IMM*>(new_inst)->SetImm(0, 2);
        TransferUsers(i, i->GetNext());
        return true;
    }

    return false;
}

void Peepholes::VisitASHR(GraphVisitor* v, InstBase* inst)
{
    assert(inst != nullptr);
    assert(inst->GetOpcode() == isa::inst::Opcode::ASHR);
    assert(v != nullptr);

    auto _this = static_cast<Peepholes*>(v);

    if (_this->FoldASHR(inst)) {
        return;
    }

    if (_this->MatchASHR_zero_0(inst)) {
        return;
    }

    if (_this->MatchASHR_zero_1(inst)) {
        return;
    }

    if (FoldBinOpToBinImmOp<isa::inst::Opcode::ASHR, isa::inst::Opcode::ASHRI>(inst)) {
        return;
    }
}

bool Peepholes::FoldASHR(InstBase* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == isa::inst::Opcode::ASHR);

    using NumInputs = isa::InputValue<typename isa::inst::Inst<isa::inst::Opcode::ASHR>::Type,
                                      isa::input::Type::VREG>;
    assert(i->GetNumInputs() == NumInputs::value);

    std::array inputs = { i->GetInput(0).GetInst(), i->GetInput(1).GetInst() };

    bool is_foldable = (inputs[0]->IsConst() && inputs[1]->IsConst());
    bool is_integral = (inputs[0]->GetDataType() == InstBase::DataType::INT &&
                        inputs[1]->GetDataType() == InstBase::DataType::INT);

    if (is_foldable && is_integral) {
        auto val1 = static_cast<isa::inst_type::CONST*>(inputs[0])->GetValInt();
        auto val2 = static_cast<isa::inst_type::CONST*>(inputs[1])->GetValInt();
        auto res = (val1 >> val2);
        ReplaceWithIntegralConst(i, res);
        return true;
    }

    return false;
}

// 1. ASHR v0, 0
// ->
// 1. v0
bool Peepholes::MatchASHR_zero_0(InstBase* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == isa::inst::Opcode::ASHR);

    using NumInputs = isa::InputValue<typename isa::inst::Inst<isa::inst::Opcode::ASHR>::Type,
                                      isa::input::Type::VREG>;
    assert(i->GetNumInputs() == NumInputs::value);

    std::array inputs = { i->GetInput(0).GetInst(), i->GetInput(1).GetInst() };

    if (inputs[1]->IsConst() && (static_cast<isa::inst_type::CONST*>(inputs[1])->IsZero())) {
        TransferUsers(i, inputs[0]);
        return true;
    }

    return false;
}

// 1. ASHR 0, v0
// ->
// 1. CONST 0
bool Peepholes::MatchASHR_zero_1(InstBase* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == isa::inst::Opcode::ASHR);

    using NumInputs = isa::InputValue<typename isa::inst::Inst<isa::inst::Opcode::ASHR>::Type,
                                      isa::input::Type::VREG>;
    assert(i->GetNumInputs() == NumInputs::value);

    auto first_input = i->GetInput(0).GetInst();
    if (first_input->IsConst() && (static_cast<isa::inst_type::CONST*>(first_input)->IsZero())) {
        ReplaceWithIntegralConst(i, 0);
        return true;
    }

    return false;
}

void Peepholes::VisitXOR(GraphVisitor* v, InstBase* inst)
{
    assert(inst != nullptr);
    assert(inst->GetOpcode() == isa::inst::Opcode::XOR);
    assert(v != nullptr);

    auto _this = static_cast<Peepholes*>(v);
    if (_this->FoldXOR(inst)) {
        return;
    }

    if (_this->MatchXOR_same_value(inst)) {
        return;
    }

    if (_this->MatchXOR_zero(inst)) {
        return;
    }

    if (FoldBinOpToBinImmOp<isa::inst::Opcode::XOR, isa::inst::Opcode::XORI>(inst)) {
        return;
    }
}

bool Peepholes::FoldXOR(InstBase* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == isa::inst::Opcode::XOR);

    using NumInputsXor = isa::InputValue<typename isa::inst::Inst<isa::inst::Opcode::XOR>::Type,
                                         isa::input::Type::VREG>;
    assert(i->GetNumInputs() == NumInputsXor::value);

    std::array inputs = { i->GetInput(0).GetInst(), i->GetInput(1).GetInst() };

    bool is_foldable = (inputs[0]->IsConst() && inputs[1]->IsConst());
    bool is_integral = (inputs[0]->GetDataType() == InstBase::DataType::INT &&
                        inputs[1]->GetDataType() == InstBase::DataType::INT);

    if (is_foldable && is_integral) {
        auto val1 = static_cast<isa::inst_type::CONST*>(inputs[0])->GetValInt();
        auto val2 = static_cast<isa::inst_type::CONST*>(inputs[1])->GetValInt();
        int64_t res = val1 ^ val2;
        ReplaceWithIntegralConst(i, res);
        return true;
    }

    return false;
}

// 1. XOR v0, v0
// ->
// 1. CONST 0
bool Peepholes::MatchXOR_same_value(InstBase* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == isa::inst::Opcode::XOR);

    using NumInputsXor = isa::InputValue<typename isa::inst::Inst<isa::inst::Opcode::XOR>::Type,
                                         isa::input::Type::VREG>;
    assert(i->GetNumInputs() == NumInputsXor::value);

    std::array inputs = { i->GetInput(0).GetInst(), i->GetInput(1).GetInst() };

    if (inputs[0]->GetId() == inputs[1]->GetId()) {
        ReplaceWithIntegralConst(i, 0);
        return true;
    }

    return false;
}

// 1. XOR v0, 0 / XOR 0, v0
// ->
// 1. v0
bool Peepholes::MatchXOR_zero(InstBase* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == isa::inst::Opcode::XOR);

    using NumInputsXor = isa::InputValue<typename isa::inst::Inst<isa::inst::Opcode::XOR>::Type,
                                         isa::input::Type::VREG>;
    assert(i->GetNumInputs() == NumInputsXor::value);

    std::array inputs = { i->GetInput(0).GetInst(), i->GetInput(1).GetInst() };

    bool input_1_is_const = inputs[1]->IsConst();
    if (inputs[0]->IsConst() || input_1_is_const) {
        auto input_const = static_cast<isa::inst_type::CONST*>(inputs[input_1_is_const]);
        auto input_param = static_cast<isa::inst_type::CONST*>(inputs[!input_1_is_const]);

        if (input_const->IsZero()) {
            TransferUsers(i, input_param);
            return true;
        }
    }

    return false;
}
