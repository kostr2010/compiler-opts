#include "peepholes.h"
#include "bb.h"
#include "graph.h"
#include "inst.h"

static void TransferUsers(Inst* from, Inst* to)
{
    for (const auto& user : from->GetUsers()) {
        to->AddUser(user);
        user.GetInst()->ReplaceInput(from, to);
        from->RemoveUser(user);
    }
}

static void ReplaceWithConst(Inst* inst, uint64_t raw, DataType type)
{
    assert(inst != nullptr);
    assert(inst->GetBasicBlock() != nullptr);

    auto bb = inst->GetBasicBlock();
    bb->PushFrontInst(Inst::NewInst<ConstantOp>(raw, type));
    auto new_inst = bb->GetFirstInst();

    TransferUsers(inst, new_inst);
}

template <Opcode OPCODE>
static void FoldBinOpToBinImmOp(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetInstType() == InstType::BinaryOp);

    auto inputs = i->GetInputs();

    assert(inputs.size() == 2);
    assert(inputs[0].GetInst()->IsConst() || inputs[1].GetInst()->IsConst());

    bool input_1_is_const = inputs[1].GetInst()->IsConst();
    auto input_const = inputs[input_1_is_const].GetInst();
    auto input_param = inputs[!input_1_is_const].GetInst();

    assert(input_const->IsConst() &&
           static_cast<ConstantOp*>(input_const)->GetDataType() == DataType::INT);

    int64_t val = static_cast<ConstantOp*>(input_const)->GetValInt();
    i->GetBasicBlock()->InsertInstAfter(Inst::NewInst<BinaryImmOp>(OPCODE, val), i);
    i->GetNext()->SetInput(0, input_param);
    TransferUsers(i, i->GetNext());
}

bool Peepholes::RunPass()
{
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
        for (auto i = bb->GetFirstInst(); i != nullptr; i = i->GetNext()) {
            switch (i->GetOpcode()) {
            case Opcode::ADD:
                MatchADD(i);
                break;
            case Opcode::ASHR:
                MatchASHR(i);
                break;
            case Opcode::XOR:
                MatchXOR(i);
                break;
            default:
                break;
            }
        }
    }

    return true;
}

void Peepholes::MatchADD(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::ADD);

    if (FoldADD(i)) {
        return;
    }

    if (MatchADD_zero(i)) {
        return;
    }

    if (MatchADD_after_sub(i)) {
        return;
    }

    if (MatchADD_same_value(i)) {
        return;
    }

    if (MatchADD_fold_to_ADDI(i)) {
        return;
    }
}

bool Peepholes::FoldADD(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::ADD);

    auto inputs = i->GetInputs();
    assert(inputs.size() == 2);

    bool is_foldable = (inputs[0].GetInst()->IsConst() && inputs[1].GetInst()->IsConst());
    bool is_integral = (inputs[0].GetInst()->GetDataType() == DataType::INT &&
                        inputs[1].GetInst()->GetDataType() == DataType::INT);

    if (is_foldable && is_integral) {
        auto val1 = static_cast<ConstantOp*>(inputs[0].GetInst())->GetValInt();
        auto val2 = static_cast<ConstantOp*>(inputs[1].GetInst())->GetValInt();
        auto res = val1 + val2;
        ReplaceWithConst(i, (uint64_t)res, DataType::INT);
        return true;
    }

    return false;
}

// 1. ADD v0, 0 / ADD 0, v0
// ->
// v0
bool Peepholes::MatchADD_zero(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::ADD);

    auto inputs = i->GetInputs();
    assert(inputs.size() == 2);

    auto input_0 = inputs[0].GetInst();
    auto input_1 = inputs[1].GetInst();

    if (input_1->IsConst() && (static_cast<ConstantOp*>(input_1)->GetValRaw() == 0)) {
        TransferUsers(i, input_0);
        return true;
    } else if (input_0->IsConst() && (static_cast<ConstantOp*>(input_0)->GetValRaw() == 0)) {
        TransferUsers(i, input_1);
        return true;
    }

    return false;
}

static bool TryMatchADD_after_sub(Inst* add, Inst* sub)
{
    assert(add != nullptr);
    assert(sub != nullptr);
    assert(sub->GetOpcode() == Opcode::SUB);

    auto add_input_0 = add->GetInputs()[0].GetInst();
    auto add_input_1 = add->GetInputs()[1].GetInst();

    auto sub_inputs = sub->GetInputs();
    auto sub_input_1 = sub_inputs[1].GetInst();

    if (!(sub_input_1->GetId() == add_input_0->GetId() ||
          sub_input_1->GetId() == add_input_1->GetId())) {
        return false;
    }

    bool is_input_1_common_arg = sub_input_1->GetId() == add_input_1->GetId();
    auto sub_common_arg = sub_inputs[is_input_1_common_arg].GetInst();

    TransferUsers(add, sub_common_arg);

    return true;
}

// 1. SUB v0, v4
// 2. ADD v1, v4 / ADD v4, v1
// ->
// 1. SUB v0, v4
// 2. v0
bool Peepholes::MatchADD_after_sub(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::ADD);

    auto inputs = i->GetInputs();
    assert(inputs.size() == 2);

    auto input_0 = inputs[0].GetInst();
    auto input_1 = inputs[1].GetInst();

    if (input_0->GetOpcode() == Opcode::SUB && TryMatchADD_after_sub(i, input_0)) {
        return true;
    }

    if (input_1->GetOpcode() == Opcode::SUB && TryMatchADD_after_sub(i, input_1)) {
        return true;
    }

    return false;
}

// 1. ADD v0, v0
// ->
// 1. SHLI v0, 2
bool Peepholes::MatchADD_same_value(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::ADD);

    auto inputs = i->GetInputs();
    assert(inputs.size() == 2);

    auto input_0 = inputs[0].GetInst();
    auto input_1 = inputs[1].GetInst();

    if (input_0->GetId() == input_1->GetId()) {
        auto bb = i->GetBasicBlock();
        bb->InsertInstAfter(Inst::NewInst<BinaryImmOp>(Opcode::SHLI, 2), i);
        i->GetNext()->SetInput(0, input_0);
        TransferUsers(i, i->GetNext());
        return true;
    }

    return false;
}

// 1. ADD v0, CONST / ADD CONST, v0
// ->
// 1. ADDI v0, CONST_VAL
bool Peepholes::MatchADD_fold_to_ADDI(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::ADD);

    auto inputs = i->GetInputs();
    assert(inputs.size() == 2);

    if (inputs[0].GetInst()->IsConst() || inputs[1].GetInst()->IsConst()) {
        FoldBinOpToBinImmOp<Opcode::ADDI>(i);
        return true;
    }

    return false;
}

void Peepholes::MatchASHR(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::ASHR);

    if (FoldASHR(i)) {
        return;
    }

    if (MatchASHR_zero_0(i)) {
        return;
    }

    if (MatchASHR_zero_1(i)) {
        return;
    }

    if (MatchASHR_fold_to_ASHRI(i)) {
        return;
    }
}

bool Peepholes::FoldASHR(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::ASHR);

    auto inputs = i->GetInputs();
    assert(inputs.size() == 2);

    bool is_foldable = (inputs[0].GetInst()->IsConst() && inputs[1].GetInst()->IsConst());
    bool is_integral = (inputs[0].GetInst()->GetDataType() == DataType::INT &&
                        inputs[1].GetInst()->GetDataType() == DataType::INT);

    if (is_foldable && is_integral) {
        auto val1 = static_cast<ConstantOp*>(inputs[0].GetInst())->GetValInt();
        auto val2 = static_cast<ConstantOp*>(inputs[1].GetInst())->GetValInt();
        auto res = (val1 >> val2);
        ReplaceWithConst(i, res, DataType::INT);
        return true;
    }

    return false;
}

// 1. ASHR v0, 0
// ->
// 1. v0
bool Peepholes::MatchASHR_zero_0(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::ASHR);

    auto inputs = i->GetInputs();
    assert(inputs.size() == 2);

    auto input_0 = inputs[0].GetInst();
    auto input_1 = inputs[1].GetInst();

    if (input_1->IsConst() && (static_cast<ConstantOp*>(input_1)->GetValRaw() == 0)) {
        TransferUsers(i, input_0);
        return true;
    }

    return false;
}

// 1. ASHR 0, v0
// ->
// 1. CONST 0
bool Peepholes::MatchASHR_zero_1(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::ASHR);

    auto inputs = i->GetInputs();
    assert(inputs.size() == 2);

    auto input_0 = inputs[0].GetInst();
    auto input_1 = inputs[1].GetInst();

    if (input_0->IsConst() && (static_cast<ConstantOp*>(input_1)->GetValRaw() == 0)) {
        ReplaceWithConst(i, 0, DataType::INT);
        return true;
    }

    return false;
}

// 1. ASHR v0, CONST
// ->
// 1. ASHRI v0, CONST_VAL
bool Peepholes::MatchASHR_fold_to_ASHRI(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::ASHR);

    auto inputs = i->GetInputs();
    assert(inputs.size() == 2);

    if (inputs[1].GetInst()->IsConst()) {
        FoldBinOpToBinImmOp<Opcode::ASHRI>(i);
        return true;
    }
    return false;
}

void Peepholes::MatchXOR(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::XOR);

    if (FoldXOR(i)) {
        return;
    }

    if (MatchXOR_same_value(i)) {
        return;
    }

    if (MatchXOR_zero(i)) {
        return;
    }

    if (MatchXOR_fold_to_XORI(i)) {
        return;
    }
}

bool Peepholes::FoldXOR(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::XOR);

    auto inputs = i->GetInputs();
    assert(inputs.size() == 2);

    bool is_foldable = (inputs[0].GetInst()->IsConst() && inputs[1].GetInst()->IsConst());
    bool is_integral = (inputs[0].GetInst()->GetDataType() == DataType::INT &&
                        inputs[1].GetInst()->GetDataType() == DataType::INT);

    if (is_foldable && is_integral) {
        auto val1 = static_cast<ConstantOp*>(inputs[0].GetInst())->GetValRaw();
        auto val2 = static_cast<ConstantOp*>(inputs[1].GetInst())->GetValRaw();
        uint64_t res = val1 ^ val2;
        ReplaceWithConst(i, res, DataType::INT);
        return true;
    }

    return false;
}

// 1. XOR v0, v0
// ->
// 1. CONST 0
bool Peepholes::MatchXOR_same_value(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::XOR);

    auto inputs = i->GetInputs();
    assert(inputs.size() == 2);

    auto input_0 = inputs[0].GetInst();
    auto input_1 = inputs[1].GetInst();

    if (input_0->GetId() == input_1->GetId()) {
        ReplaceWithConst(i, 0, DataType::INT);
        return true;
    }

    return false;
}

// 1. XOR v0, 0 / XOR 0, v0
// ->
// 1. v0
bool Peepholes::MatchXOR_zero(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::XOR);

    auto inputs = i->GetInputs();
    assert(inputs.size() == 2);

    bool input_1_is_const = inputs[1].GetInst()->IsConst();
    if (inputs[0].GetInst()->IsConst() || input_1_is_const) {
        auto input_const = static_cast<ConstantOp*>(inputs[input_1_is_const].GetInst());
        auto input_param = static_cast<ConstantOp*>(inputs[!input_1_is_const].GetInst());

        if (input_const->GetValRaw() == 0) {
            TransferUsers(i, input_param);
            return true;
        }
    }

    return false;
}

// 1. XOR v0, CONST / XOR CONST, v0
// ->
// 1. XORI v0, CONST_VAL
bool Peepholes::MatchXOR_fold_to_XORI(Inst* i)
{
    assert(i != nullptr);
    assert(i->GetOpcode() == Opcode::XOR);

    auto inputs = i->GetInputs();
    assert(inputs.size() == 2);

    if (inputs[0].GetInst()->IsConst() || inputs[1].GetInst()->IsConst()) {
        FoldBinOpToBinImmOp<Opcode::XORI>(i);
        return true;
    }

    return false;
}
