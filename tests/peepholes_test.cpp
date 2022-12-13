#include "bb.h"
#include "graph.h"
#include "graph_builder.h"

#include "gtest/gtest.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static void CheckUsers(Inst* inst, std::set<std::pair<IdType, int> > expected)
{
    std::vector<std::pair<IdType, int> > res = {};
    for (const auto& u : inst->GetUsers()) {
        res.push_back({ u.GetInst()->GetId(), u.GetIdx() });
    }

    auto res_set = std::set<std::pair<IdType, int> >(res.begin(), res.end());
    ASSERT_EQ(res_set, expected);
}

static void CheckInputs(Inst* inst, std::set<std::pair<IdType, IdType> > expected)
{
    std::vector<std::pair<IdType, IdType> > res = {};
    for (const auto& i : inst->GetInputs()) {
        res.push_back({ i.GetInst()->GetId(), i.GetSourceBB()->GetId() });
    }

    auto res_set = std::set<std::pair<IdType, IdType> >(res.begin(), res.end());
    ASSERT_EQ(res_set, expected);
}

TEST(TestPeepholes, FoldADD_1)
{
    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(0U);
    auto C1 = b.NewConst(2U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::ADD>();
    auto I1 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, C1, C0);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_EQ(bb_start->GetFirstInst(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto c = bb_a->GetFirstInst();
    ASSERT_EQ(c->GetOpcode(), Opcode::CONST);
    ASSERT_EQ(c->GetDataType(), DataType::INT);
    ASSERT_EQ(static_cast<ConstantOp*>(c)->GetValInt(), 2U);
    auto i1 = c->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(c, {});
    CheckUsers(c, { { I1, 0 } });

    CheckInputs(i1, { { c->GetId(), A } });
    CheckUsers(i1, {});
}

TEST(TestPeepholes, FoldADD_2)
{
    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(4);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::ADD>();
    auto I1 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, C1, C0);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_EQ(bb_start->GetFirstInst(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto c = bb_a->GetFirstInst();
    ASSERT_EQ(c->GetOpcode(), Opcode::CONST);
    ASSERT_EQ(c->GetDataType(), DataType::INT);
    ASSERT_EQ(static_cast<ConstantOp*>(c)->GetValInt(), 2);
    auto i1 = c->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(c, {});
    CheckUsers(c, { { I1, 0 } });

    CheckInputs(i1, { { c->GetId(), A } });
    CheckUsers(i1, {});
}

TEST(TestPeepholes, MatchADD_zero_1)
{
    // ADD v0, 0
    // ->
    // v0

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::ADD>(); // -> ADDI
    auto I1 = b.NewInst<Opcode::ADD>(); // -> v0
    auto I2 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, P0, C1);
    b.SetInputs(I1, P0, C0);
    b.SetInputs(I2, I1);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_EQ(p0->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetId(), I2);

    CheckInputs(p0, {});
    CheckUsers(p0, { { I2, 0 } });
}

TEST(TestPeepholes, MatchADD_zero_2)
{
    // ADD 0, v0
    // ->
    // v0

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::ADD>(); // -> ADDI
    auto I1 = b.NewInst<Opcode::ADD>(); // -> v0
    auto I2 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, C1, P0);
    b.SetInputs(I1, C0, P0);
    b.SetInputs(I2, I1);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_EQ(p0->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetId(), I2);

    CheckInputs(p0, {});
    CheckUsers(p0, { { I2, 0 } });
}

TEST(TestPeepholes, MatchADD_after_sub_1)
{
    // 1. SUB v0, v4
    // 2. ADD v1, v4 / ADD v4, v1
    // ->
    // 1. SUB v0, v4
    // 2. v0

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto P1 = b.NewParameter(1);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::SUB>();
    auto II = b.NewInst<Opcode::ADD>();
    auto I1 = b.NewInst<Opcode::ADD>();
    auto I2 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, P1, P0);
    b.SetInputs(I1, P0, I0);
    b.SetInputs(II, I0, C0);
    b.SetInputs(I2, I1);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_NE(p0->GetNext(), nullptr);
    auto p1 = p0->GetNext();
    ASSERT_NE(p1, nullptr);
    ASSERT_EQ(p1->GetId(), P1);
    ASSERT_EQ(p1->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetId(), I2);

    CheckInputs(p0, {});
    CheckUsers(p0, {});

    CheckInputs(p1, {});
    CheckUsers(p1, { { I2, 0 } });

    CheckInputs(i2, { { P1, START } });
    CheckUsers(i2, {});
}

TEST(TestPeepholes, MatchADD_after_sub_2)
{
    // 1. SUB v0, v4
    // 2. ADD v1, v4 / ADD v4, v1
    // ->
    // 1. SUB v0, v4
    // 2. v0

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto P1 = b.NewParameter(1);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::SUB>();
    auto II = b.NewInst<Opcode::ADD>();
    auto I1 = b.NewInst<Opcode::ADD>();
    auto I2 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, P1, P0);
    b.SetInputs(I1, I0, P0);
    b.SetInputs(II, I0, C0);
    b.SetInputs(I2, I1);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_NE(p0->GetNext(), nullptr);
    auto p1 = p0->GetNext();
    ASSERT_NE(p1, nullptr);
    ASSERT_EQ(p1->GetId(), P1);
    ASSERT_EQ(p1->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetId(), I2);

    CheckInputs(p0, {});
    CheckUsers(p0, { { I2, 0 } });

    CheckInputs(p1, {});
    CheckUsers(p1, {});

    CheckInputs(i2, { { P0, START } });
    CheckUsers(i2, {});
}

TEST(TestPeepholes, MatchADD_after_sub_3)
{
    // 1. SUB v0, v4
    // 2. ADD v1, v4 / ADD v4, v1
    // ->
    // 1. SUB v0, v4
    // 2. v0

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto P1 = b.NewParameter(1);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I_ = b.NewInst<Opcode::SUB>();
    auto I0 = b.NewInst<Opcode::SUB>();
    auto II = b.NewInst<Opcode::ADD>();
    auto I1 = b.NewInst<Opcode::ADD>();
    auto I2 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I_, C1, P0);
    b.SetInputs(I0, P1, I_);
    b.SetInputs(I1, I0, I_);
    b.SetInputs(II, I0, C0);
    b.SetInputs(I2, I1);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_NE(p0->GetNext(), nullptr);
    auto p1 = p0->GetNext();
    ASSERT_NE(p1, nullptr);
    ASSERT_EQ(p1->GetId(), P1);
    ASSERT_NE(p1->GetNext(), nullptr);
    auto c1 = p1->GetNext();
    ASSERT_NE(c1, nullptr);
    ASSERT_EQ(c1->GetId(), C1);
    ASSERT_EQ(c1->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i_ = bb_a->GetFirstInst();
    ASSERT_NE(i_, nullptr);
    ASSERT_EQ(i_->GetId(), I_);
    ASSERT_NE(i_->GetNext(), nullptr);
    auto i2 = i_->GetNext();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetId(), I2);
    ASSERT_EQ(i2->GetNext(), nullptr);

    CheckInputs(p0, {});
    CheckUsers(p0, { { I_, 1 } });

    CheckInputs(p1, {});
    CheckUsers(p1, {});

    CheckInputs(c1, {});
    CheckUsers(c1, { { I_, 0 } });

    CheckInputs(i_, { { P0, START }, { C1, START } });
    CheckUsers(i_, { { I2, 0 } });

    CheckInputs(i2, { { I_, A } });
    CheckUsers(i2, {});
}

TEST(TestPeepholes, MatchADD_same_value_1)
{
    // 1. ADD v0, v0
    // ->
    // 1. SHLI v0, 2

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::ADD>();
    auto I1 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, P0, P0);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_EQ(p0->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetOpcode(), Opcode::SHLI);
    ASSERT_EQ(static_cast<BinaryImmOp*>(i2)->GetImm(), 2);
    auto i1 = i2->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(p0, {});
    CheckUsers(p0, { { i2->GetId(), 0 } });

    CheckInputs(i2, { { p0->GetId(), START } });
    CheckUsers(i2, { { i1->GetId(), 0 } });

    CheckInputs(i1, { { i2->GetId(), A } });
    CheckUsers(i1, {});
}

TEST(TestPeepholes, MatchADD_fold_to_ADDI_1)
{
    // 1. ADD v0, CONST / ADD CONST, v0
    // ->
    // 1. ADDI v0, CONST_VAL

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::ADD>();
    auto I1 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, P0, C1);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_EQ(p0->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetOpcode(), Opcode::ADDI);
    ASSERT_EQ(static_cast<BinaryImmOp*>(i2)->GetImm(), -2);
    auto i1 = i2->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(p0, {});
    CheckUsers(p0, { { i2->GetId(), 0 } });

    CheckInputs(i2, { { p0->GetId(), START } });
    CheckUsers(i2, { { i1->GetId(), 0 } });

    CheckInputs(i1, { { i2->GetId(), A } });
    CheckUsers(i1, {});
}

TEST(TestPeepholes, MatchADD_fold_to_ADDI_2)
{
    // 1. ADD v0, CONST / ADD CONST, v0
    // ->
    // 1. ADDI v0, CONST_VAL

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::ADD>();
    auto I1 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, C1, P0);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_EQ(p0->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetOpcode(), Opcode::ADDI);
    ASSERT_EQ(static_cast<BinaryImmOp*>(i2)->GetImm(), -2);
    auto i1 = i2->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(p0, {});
    CheckUsers(p0, { { i2->GetId(), 0 } });

    CheckInputs(i2, { { p0->GetId(), START } });
    CheckUsers(i2, { { i1->GetId(), 0 } });

    CheckInputs(i1, { { i2->GetId(), A } });
    CheckUsers(i1, {});
}

TEST(TestPeepholes, FoldASHR)
{
    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(2);
    auto C1 = b.NewConst(4);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::ASHR>();
    auto I1 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, C1, C0);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_EQ(bb_start->GetFirstInst(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_TRUE(i2->IsConst());
    ASSERT_EQ(static_cast<ConstantOp*>(i2)->GetDataType(), DataType::INT);
    ASSERT_EQ(static_cast<ConstantOp*>(i2)->GetValInt(), 1);
    auto i1 = i2->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(i2, {});
    CheckUsers(i2, { { i1->GetId(), 0 } });

    CheckInputs(i1, { { i2->GetId(), A } });
    CheckUsers(i1, {});
}

TEST(TestPeepholes, MatchASHR_zero_0)
{
    // 1. ASHR v0, 0
    // ->
    // 1. v0

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::ASHR>();
    auto I1 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, P0, C0);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_EQ(p0->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i1 = bb_a->GetFirstInst();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(p0, {});
    CheckUsers(p0, { { I1, 0 } });

    CheckInputs(i1, { { P0, START } });
    CheckUsers(i1, {});
}

TEST(TestPeepholes, MatchASHR_zero_1)
{
    // 1. ASHR 0, v0
    // ->
    // 1. CONST 0

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::ASHR>();
    auto I1 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, C0, P0);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_TRUE(i2->IsConst());
    ASSERT_EQ(static_cast<ConstantOp*>(i2)->GetDataType(), DataType::INT);
    ASSERT_EQ(static_cast<ConstantOp*>(i2)->GetValInt(), 0);
    auto i1 = i2->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(i2, {});
    CheckUsers(i2, { { I1, 0 } });

    CheckInputs(i1, { { i2->GetId(), A } });
    CheckUsers(i1, {});
}

TEST(TestPeepholes, MatchASHR_fold_to_ASHRI)
{
    // 1. ASHR v0, CONST
    // ->
    // 1. ASHRI v0, CONST_VAL

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(17);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::ASHR>();
    auto I1 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, P0, C1);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_EQ(p0->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetOpcode(), Opcode::ASHRI);
    ASSERT_EQ(static_cast<BinaryImmOp*>(i2)->GetImm(), 17);
    auto i1 = i2->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(p0, {});
    CheckUsers(p0, { { i2->GetId(), 0 } });

    CheckInputs(i2, { { p0->GetId(), START } });
    CheckUsers(i2, { { i1->GetId(), 0 } });

    CheckInputs(i1, { { i2->GetId(), A } });
    CheckUsers(i1, {});
}

TEST(TestPeepholes, FoldXOR_1)
{
    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(13);
    auto C1 = b.NewConst(17);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::XOR>();
    auto I1 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, C1, C0);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_EQ(bb_start->GetFirstInst(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto c = bb_a->GetFirstInst();
    ASSERT_EQ(c->GetOpcode(), Opcode::CONST);
    ASSERT_EQ(c->GetDataType(), DataType::INT);
    ASSERT_EQ(static_cast<ConstantOp*>(c)->GetValInt(), 28);
    auto i1 = c->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(c, {});
    CheckUsers(c, { { I1, 0 } });

    CheckInputs(i1, { { c->GetId(), A } });
    CheckUsers(i1, {});
}

TEST(TestPeepholes, MatchXOR_same_value)
{
    // 1. XOR v0, v0
    // ->
    // 1. CONST 0

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::XOR>();
    auto I1 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, P0, P0);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_TRUE(i2->IsConst());
    ASSERT_EQ(static_cast<ConstantOp*>(i2)->GetDataType(), DataType::INT);
    ASSERT_EQ(static_cast<ConstantOp*>(i2)->GetValInt(), 0);
    auto i1 = i2->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(i2, {});
    CheckUsers(i2, { { I1, 0 } });

    CheckInputs(i1, { { i2->GetId(), A } });
    CheckUsers(i1, {});
}

TEST(TestPeepholes, MatchXOR_zero)
{
    // 1. XOR v0, 0
    // ->
    // 1. v0

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::XOR>();
    auto I1 = b.NewInst<Opcode::XOR>();
    auto I2 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, P0, C1);
    b.SetInputs(I1, P0, C0);
    b.SetInputs(I2, I1);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_EQ(p0->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetId(), I2);

    CheckInputs(p0, {});
    CheckUsers(p0, { { I2, 0 } });
}

TEST(TestPeepholes, MatchXOR_zero_)
{
    // 1. XOR 0, v0
    // ->
    // 1. v0

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::XOR>();
    auto I1 = b.NewInst<Opcode::XOR>();
    auto I2 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, P0, C1);
    b.SetInputs(I1, C0, P0);
    b.SetInputs(I2, I1);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_EQ(p0->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetId(), I2);

    CheckInputs(p0, {});
    CheckUsers(p0, { { I2, 0 } });
}

TEST(TestPeepholes, MatchXOR_fold_to_XORI)
{
    // 1. XOR v0, CONST
    // ->
    // 1. XORI v0, CONST_VAL

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::XOR>();
    auto I1 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, P0, C1);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_EQ(p0->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetOpcode(), Opcode::XORI);
    ASSERT_EQ(static_cast<BinaryImmOp*>(i2)->GetImm(), -2);
    auto i1 = i2->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(p0, {});
    CheckUsers(p0, { { i2->GetId(), 0 } });

    CheckInputs(i2, { { p0->GetId(), START } });
    CheckUsers(i2, { { i1->GetId(), 0 } });

    CheckInputs(i1, { { i2->GetId(), A } });
    CheckUsers(i1, {});
}

TEST(TestPeepholes, MatchXOR_fold_to_XORI_)
{
    // 1. XOR v0, CONST
    // ->
    // 1. XORI v0, CONST_VAL

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter(0);
    auto C0 = b.NewConst(0);
    auto C1 = b.NewConst(-2);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Opcode::XOR>();
    auto I1 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(I0, C1, P0);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    ASSERT_EQ(p0->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetOpcode(), Opcode::XORI);
    ASSERT_EQ(static_cast<BinaryImmOp*>(i2)->GetImm(), -2);
    auto i1 = i2->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(p0, {});
    CheckUsers(p0, { { i2->GetId(), 0 } });

    CheckInputs(i2, { { p0->GetId(), START } });
    CheckUsers(i2, { { i1->GetId(), 0 } });

    CheckInputs(i1, { { i2->GetId(), A } });
    CheckUsers(i1, {});
}

#pragma GCC diagnostic pop
