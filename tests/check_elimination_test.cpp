#include "bb.h"
#include "graph.h"
#include "graph_builder.h"

#include "gtest/gtest.h"

static void CheckUsers(InstBase* inst, std::set<std::pair<IdType, int> > expected)
{
    std::vector<std::pair<IdType, int> > res = {};
    for (const auto& u : inst->GetUsers()) {
        res.push_back({ u.GetInst()->GetId(), u.GetIdx() });
    }

    auto user_set = std::set<std::pair<IdType, int> >(res.begin(), res.end());
    ASSERT_EQ(user_set, expected);
}

static void CheckInputs(InstBase* inst, std::set<std::pair<IdType, IdType> > expected)
{
    std::vector<std::pair<IdType, IdType> > res = {};
    for (const auto& i : inst->GetInputs()) {
        res.push_back({ i.GetInst()->GetId(), i.GetSourceBB()->GetId() });
    }

    auto input_set = std::set<std::pair<IdType, IdType> >(res.begin(), res.end());
    ASSERT_EQ(input_set, expected);
}

TEST(TestCheckElimination, TestSameCheckSameInput)
{
    /*
    +-------+
    | START |
    +-------+
      |
      |
      v
    +-------+
    |   A   |
    +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(0U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::MULI>();
    auto I1 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I2 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I3 = b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(I0, C0);
    b.SetImmediate(I0, 0, 10);
    b.SetInputs(I1, I0);
    b.SetInputs(I2, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<CheckElimination>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto c0 = bb_start->GetFirstInst();
    ASSERT_EQ(c0->GetId(), C0);
    ASSERT_EQ(c0->GetNext(), nullptr);

    auto bb_a = bb_start->GetSuccessor(0);
    ASSERT_NE(bb_a, nullptr);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i0 = bb_a->GetFirstInst();
    ASSERT_EQ(i0->GetId(), I0);
    ASSERT_EQ(i0->GetOpcode(), isa::inst::Opcode::MULI);
    auto i1 = i0->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);
    ASSERT_EQ(i1->GetOpcode(), isa::inst::Opcode::CHECK_ZERO);
    auto i3 = i1->GetNext();
    ASSERT_NE(i3, nullptr);
    ASSERT_EQ(i3->GetId(), I3);
    ASSERT_EQ(i3->GetOpcode(), isa::inst::Opcode::RETURN_VOID);
    ASSERT_EQ(i3->GetNext(), nullptr);

    CheckInputs(c0, {});
    CheckUsers(c0, { { I0, 0 } });

    CheckInputs(i0, { { C0, START } });
    CheckUsers(i0, { { I1, 0 } });

    CheckInputs(i1, { { I0, A } });
    CheckUsers(i1, {});

    CheckInputs(i3, {});
    CheckUsers(i3, {});
}

TEST(TestCheckElimination, TestSameCheckDiffInput)
{
    /*
    +-------+
    | START |
    +-------+
      |
      |
      v
    +-------+
    |   A   |
    +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(0U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::MULI>();
    auto I1 = b.NewInst<isa::inst::Opcode::MULI>();
    auto I2 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I3 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I4 = b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(I0, C0);
    b.SetImmediate(I0, 0, 1);
    b.SetInputs(I1, C0);
    b.SetImmediate(I1, 0, 11);
    b.SetInputs(I2, I0);
    b.SetInputs(I3, I1);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<CheckElimination>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto c0 = bb_start->GetFirstInst();
    ASSERT_EQ(c0->GetId(), C0);
    ASSERT_EQ(c0->GetNext(), nullptr);
    ASSERT_EQ(bb_start->GetNumSuccessors(), 1);

    auto bb_a = bb_start->GetSuccessor(0);
    ASSERT_NE(bb_a, nullptr);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i0 = bb_a->GetFirstInst();
    ASSERT_EQ(i0->GetId(), I0);
    ASSERT_EQ(i0->GetOpcode(), isa::inst::Opcode::MULI);
    auto i1 = i0->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);
    ASSERT_EQ(i1->GetOpcode(), isa::inst::Opcode::MULI);
    auto i2 = i1->GetNext();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetId(), I2);
    ASSERT_EQ(i2->GetOpcode(), isa::inst::Opcode::CHECK_ZERO);
    auto i3 = i2->GetNext();
    ASSERT_NE(i3, nullptr);
    ASSERT_EQ(i3->GetId(), I3);
    ASSERT_EQ(i3->GetOpcode(), isa::inst::Opcode::CHECK_ZERO);
    auto i4 = i3->GetNext();
    ASSERT_NE(i4, nullptr);
    ASSERT_EQ(i4->GetId(), I4);
    ASSERT_EQ(i4->GetOpcode(), isa::inst::Opcode::RETURN_VOID);
    ASSERT_EQ(i4->GetNext(), nullptr);
    ASSERT_EQ(bb_a->GetNumSuccessors(), 0);

    CheckInputs(c0, {});
    CheckUsers(c0, { { I0, 0 }, { I1, 0 } });

    CheckInputs(i0, { { C0, START } });
    CheckUsers(i0, { { I2, 0 } });

    CheckInputs(i1, { { C0, START } });
    CheckUsers(i1, { { I3, 0 } });

    CheckInputs(i2, { { I0, A } });
    CheckUsers(i2, {});

    CheckInputs(i3, { { I1, A } });
    CheckUsers(i3, {});

    CheckInputs(i4, {});
    CheckUsers(i4, {});
}

TEST(TestCheckElimination, TestDiffCheckSameInput)
{
    /*
    +-------+
    | START |
    +-------+
      |
      |
      v
    +-------+
    |   A   |
    +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(0U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::MULI>();
    auto I1 = b.NewInst<isa::inst::Opcode::CHECK_NULL>();
    auto I2 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I3 = b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(I0, C0);
    b.SetImmediate(I0, 0, 10);
    b.SetInputs(I1, I0);
    b.SetInputs(I2, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<CheckElimination>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto c0 = bb_start->GetFirstInst();
    ASSERT_EQ(c0->GetId(), C0);
    ASSERT_EQ(c0->GetNext(), nullptr);
    ASSERT_EQ(bb_start->GetNumSuccessors(), 1);

    auto bb_a = bb_start->GetSuccessor(0);
    ASSERT_NE(bb_a, nullptr);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i0 = bb_a->GetFirstInst();
    ASSERT_EQ(i0->GetId(), I0);
    ASSERT_EQ(i0->GetOpcode(), isa::inst::Opcode::MULI);
    auto i1 = i0->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);
    ASSERT_EQ(i1->GetOpcode(), isa::inst::Opcode::CHECK_NULL);
    auto i2 = i1->GetNext();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetId(), I2);
    ASSERT_EQ(i2->GetOpcode(), isa::inst::Opcode::CHECK_ZERO);
    auto i3 = i2->GetNext();
    ASSERT_NE(i3, nullptr);
    ASSERT_EQ(i3->GetId(), I3);
    ASSERT_EQ(i3->GetOpcode(), isa::inst::Opcode::RETURN_VOID);
    ASSERT_EQ(i3->GetNext(), nullptr);
    ASSERT_EQ(bb_a->GetNumSuccessors(), 0);

    CheckInputs(c0, {});
    CheckUsers(c0, { { I0, 0 } });

    CheckInputs(i0, { { C0, START } });
    CheckUsers(i0, { { I1, 0 }, { I2, 0 } });

    CheckInputs(i1, { { I0, A } });
    CheckUsers(i1, {});

    CheckInputs(i2, { { I0, A } });
    CheckUsers(i2, {});

    CheckInputs(i3, {});
    CheckUsers(i3, {});
}

TEST(TestCheckElimination, TestDiffCheckDiffInput)
{
    /*
    +-------+
    | START |
    +-------+
      |
      |
      v
    +-------+
    |   A   |
    +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(0U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::MULI>();
    auto I1 = b.NewInst<isa::inst::Opcode::MULI>();
    auto I2 = b.NewInst<isa::inst::Opcode::CHECK_NULL>();
    auto I3 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I4 = b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(I0, C0);
    b.SetImmediate(I0, 0, 1);
    b.SetInputs(I1, C0);
    b.SetImmediate(I1, 0, 11);
    b.SetInputs(I2, I0);
    b.SetInputs(I3, I1);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<CheckElimination>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto c0 = bb_start->GetFirstInst();
    ASSERT_EQ(c0->GetId(), C0);
    ASSERT_EQ(c0->GetNext(), nullptr);
    ASSERT_EQ(bb_start->GetNumSuccessors(), 1);

    auto bb_a = bb_start->GetSuccessor(0);
    ASSERT_NE(bb_a, nullptr);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i0 = bb_a->GetFirstInst();
    ASSERT_EQ(i0->GetId(), I0);
    ASSERT_EQ(i0->GetOpcode(), isa::inst::Opcode::MULI);
    auto i1 = i0->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);
    ASSERT_EQ(i1->GetOpcode(), isa::inst::Opcode::MULI);
    auto i2 = i1->GetNext();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetId(), I2);
    ASSERT_EQ(i2->GetOpcode(), isa::inst::Opcode::CHECK_NULL);
    auto i3 = i2->GetNext();
    ASSERT_NE(i3, nullptr);
    ASSERT_EQ(i3->GetId(), I3);
    ASSERT_EQ(i3->GetOpcode(), isa::inst::Opcode::CHECK_ZERO);
    auto i4 = i3->GetNext();
    ASSERT_NE(i4, nullptr);
    ASSERT_EQ(i4->GetId(), I4);
    ASSERT_EQ(i4->GetOpcode(), isa::inst::Opcode::RETURN_VOID);
    ASSERT_EQ(i4->GetNext(), nullptr);
    ASSERT_EQ(bb_a->GetNumSuccessors(), 0);

    CheckInputs(c0, {});
    CheckUsers(c0, { { I0, 0 }, { I1, 0 } });

    CheckInputs(i0, { { C0, START } });
    CheckUsers(i0, { { I2, 0 } });

    CheckInputs(i1, { { C0, START } });
    CheckUsers(i1, { { I3, 0 } });

    CheckInputs(i2, { { I0, A } });
    CheckUsers(i2, {});

    CheckInputs(i3, { { I1, A } });
    CheckUsers(i3, {});

    CheckInputs(i4, {});
    CheckUsers(i4, {});
}

TEST(TestCheckElimination, TestMultipleSameCheckSameInput)
{
    /*
    +-------+
    | START |
    +-------+
      |
      |
      v
    +-------+
    |   A   |
    +-------+
      |
      |
      v
    +-------+
    |   B   |
    +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(0U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::MULI>();
    auto I1 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I2 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I3 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();

    [[maybe_unused]] auto B = b.NewBlock();

    auto I4 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I5 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I6 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto R0 = b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(I0, C0);
    b.SetImmediate(I0, 0, 10);
    b.SetInputs(I1, I0);
    b.SetInputs(I2, I0);
    b.SetInputs(I3, I0);
    b.SetInputs(I4, I0);
    b.SetInputs(I5, I0);
    b.SetInputs(I6, I0);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<CheckElimination>();
    g.GetAnalyser()->GetValidPass<DBE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto c0 = bb_start->GetFirstInst();
    ASSERT_EQ(c0->GetId(), C0);
    ASSERT_EQ(c0->GetNext(), nullptr);
    ASSERT_EQ(bb_start->GetNumSuccessors(), 1);

    auto bb_a = bb_start->GetSuccessor(0);
    ASSERT_NE(bb_a, nullptr);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i0 = bb_a->GetFirstInst();
    ASSERT_EQ(i0->GetId(), I0);
    ASSERT_EQ(i0->GetOpcode(), isa::inst::Opcode::MULI);
    auto i1 = i0->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);
    ASSERT_EQ(i1->GetOpcode(), isa::inst::Opcode::CHECK_ZERO);
    ASSERT_EQ(bb_a->GetNumSuccessors(), 1);

    auto bb_b = bb_a->GetSuccessor(0);
    ASSERT_NE(bb_b, nullptr);
    ASSERT_NE(bb_b->GetFirstInst(), nullptr);
    auto r0 = bb_b->GetFirstInst();
    ASSERT_NE(r0, nullptr);
    ASSERT_EQ(r0->GetId(), R0);
    ASSERT_EQ(r0->GetOpcode(), isa::inst::Opcode::RETURN_VOID);
    ASSERT_EQ(r0->GetNext(), nullptr);
    ASSERT_EQ(bb_b->GetNumSuccessors(), 0);

    CheckInputs(c0, {});
    CheckUsers(c0, { { I0, 0 } });

    CheckInputs(i0, { { C0, START } });
    CheckUsers(i0, { { I1, 0 } });

    CheckInputs(i1, { { I0, A } });
    CheckUsers(i1, {});

    CheckInputs(r0, {});
    CheckUsers(r0, {});
}

TEST(TestCheckElimination, TestMultipleSameCheckSameInputLeaveNotDominated)
{
    /*
              +-------+
              | START |
              +-------+
                |
                |
                v
    +---+     +-------+
    | C | <-- |   A   |
    +---+     +-------+
                |
                |
                v
              +-------+
              |   B   |
              +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(0U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::MULI>();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

    auto B = b.NewBlock();
    auto I1 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I2 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I3 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto R0 = b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    auto C = b.NewBlock();
    auto I4 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I5 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I6 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto R1 = b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(IF0, C0, C0);
    b.SetInputs(I0, C0);
    b.SetImmediate(I0, 0, 10);
    b.SetInputs(I1, I0);
    b.SetInputs(I2, I0);
    b.SetInputs(I3, I0);
    b.SetInputs(I4, I0);
    b.SetInputs(I5, I0);
    b.SetInputs(I6, I0);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B, C });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<CheckElimination>();
    g.GetAnalyser()->GetValidPass<DBE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto c0 = bb_start->GetFirstInst();
    ASSERT_EQ(c0->GetId(), C0);
    ASSERT_EQ(c0->GetNext(), nullptr);
    ASSERT_EQ(bb_start->GetNumSuccessors(), 1);

    auto bb_a = bb_start->GetSuccessor(0);
    ASSERT_NE(bb_a, nullptr);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i0 = bb_a->GetFirstInst();
    ASSERT_EQ(i0->GetId(), I0);
    ASSERT_EQ(i0->GetOpcode(), isa::inst::Opcode::MULI);
    auto if0 = i0->GetNext();
    ASSERT_NE(if0, nullptr);
    ASSERT_EQ(if0->GetId(), IF0);
    ASSERT_EQ(if0->GetOpcode(), isa::inst::Opcode::IF);
    ASSERT_EQ(if0->GetNext(), nullptr);
    ASSERT_EQ(bb_a->GetNumSuccessors(), 2);

    auto bb_b = bb_a->GetSuccessor(0);
    ASSERT_NE(bb_b->GetFirstInst(), nullptr);
    auto i1 = bb_b->GetFirstInst();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);
    ASSERT_EQ(i1->GetOpcode(), isa::inst::Opcode::CHECK_ZERO);
    auto r0 = i1->GetNext();
    ASSERT_NE(r0, nullptr);
    ASSERT_EQ(r0->GetId(), R0);
    ASSERT_EQ(r0->GetOpcode(), isa::inst::Opcode::RETURN_VOID);
    ASSERT_EQ(r0->GetNext(), nullptr);
    ASSERT_EQ(bb_b->GetNumSuccessors(), 0);

    auto bb_c = bb_a->GetSuccessor(1);
    ASSERT_NE(bb_c->GetFirstInst(), nullptr);
    auto i4 = bb_c->GetFirstInst();
    ASSERT_NE(i4, nullptr);
    ASSERT_EQ(i4->GetId(), I4);
    ASSERT_EQ(i4->GetOpcode(), isa::inst::Opcode::CHECK_ZERO);
    auto r1 = i4->GetNext();
    ASSERT_NE(r1, nullptr);
    ASSERT_EQ(r1->GetId(), R1);
    ASSERT_EQ(r1->GetOpcode(), isa::inst::Opcode::RETURN_VOID);
    ASSERT_EQ(r1->GetNext(), nullptr);
    ASSERT_EQ(bb_c->GetNumSuccessors(), 0);

    CheckInputs(c0, {});
    CheckUsers(c0, { { I0, 0 }, { IF0, 0 }, { IF0, 1 } });

    CheckInputs(i0, { { C0, START } });
    CheckUsers(i0, { { I1, 0 }, { I4, 0 } });

    CheckInputs(i1, { { I0, A } });
    CheckUsers(i1, {});

    CheckInputs(i4, { { I0, A } });
    CheckUsers(i4, {});

    CheckInputs(r0, {});
    CheckUsers(r0, {});

    CheckInputs(r1, {});
    CheckUsers(r1, {});
}

TEST(TestCheckElimination, TestCheckSizeEquivalence)
{
    /*
              +-------+
              | START |
              +-------+
                |
                |
                v
    +---+     +-------+
    | C | <-- |   A   |
    +---+     +-------+
                |
                |
                v
              +-------+
              |   B   |
              +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(0U);
    auto C1 = b.NewConst(1U);
    auto C2 = b.NewConst(0U);
    auto C3 = b.NewConst(2U);

    auto A = b.NewBlock();
    // dummy for array of something like that. not important for the sake of this pass
    auto ARR = b.NewInst<isa::inst::Opcode::ADDI>();
    auto I0 = b.NewInst<isa::inst::Opcode::CHECK_SIZE>();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

    auto B = b.NewBlock();
    auto I1 = b.NewInst<isa::inst::Opcode::CHECK_SIZE>();
    auto I2 = b.NewInst<isa::inst::Opcode::CHECK_SIZE>();
    auto R0 = b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    auto C = b.NewBlock();
    auto I3 = b.NewInst<isa::inst::Opcode::CHECK_SIZE>();
    auto I4 = b.NewInst<isa::inst::Opcode::CHECK_SIZE>();
    auto R1 = b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(IF0, C0, C0);
    b.SetInputs(ARR, C0);
    b.SetImmediate(ARR, 0, 11);
    b.SetInputs(I0, ARR, C0);
    b.SetInputs(I1, ARR, C0);
    b.SetInputs(I2, ARR, C1);
    b.SetInputs(I3, ARR, C3);
    b.SetInputs(I4, ARR, C2);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B, C });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<CheckElimination>();
    g.GetAnalyser()->GetValidPass<DCE>();
    g.GetAnalyser()->GetValidPass<DBE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto c0 = bb_start->GetFirstInst();
    ASSERT_EQ(c0->GetId(), C0);
    ASSERT_NE(c0->GetNext(), nullptr);
    auto c1 = c0->GetNext();
    ASSERT_EQ(c1->GetId(), C1);
    ASSERT_NE(c1->GetNext(), nullptr);
    auto c3 = c1->GetNext();
    ASSERT_EQ(c3->GetId(), C3);
    ASSERT_EQ(c3->GetNext(), nullptr);
    ASSERT_EQ(bb_start->GetNumSuccessors(), 1);

    auto bb_a = bb_start->GetSuccessor(0);
    ASSERT_NE(bb_a, nullptr);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto arr = bb_a->GetFirstInst();
    ASSERT_EQ(arr->GetId(), ARR);
    ASSERT_EQ(arr->GetOpcode(), isa::inst::Opcode::ADDI);
    ASSERT_NE(arr->GetNext(), nullptr);
    auto i0 = arr->GetNext();
    ASSERT_NE(i0, nullptr);
    ASSERT_EQ(i0->GetId(), I0);
    ASSERT_EQ(i0->GetOpcode(), isa::inst::Opcode::CHECK_SIZE);
    auto if0 = i0->GetNext();
    ASSERT_NE(if0, nullptr);
    ASSERT_EQ(if0->GetId(), IF0);
    ASSERT_EQ(if0->GetOpcode(), isa::inst::Opcode::IF);
    ASSERT_EQ(if0->GetNext(), nullptr);
    ASSERT_EQ(bb_a->GetNumSuccessors(), 2);

    auto bb_b = bb_a->GetSuccessor(0);
    ASSERT_NE(bb_b->GetFirstInst(), nullptr);
    auto i2 = bb_b->GetFirstInst();
    ASSERT_NE(i2, nullptr);
    ASSERT_EQ(i2->GetId(), I2);
    ASSERT_EQ(i2->GetOpcode(), isa::inst::Opcode::CHECK_SIZE);
    auto r0 = i2->GetNext();
    ASSERT_NE(r0, nullptr);
    ASSERT_EQ(r0->GetId(), R0);
    ASSERT_EQ(r0->GetOpcode(), isa::inst::Opcode::RETURN_VOID);
    ASSERT_EQ(r0->GetNext(), nullptr);
    ASSERT_EQ(bb_b->GetNumSuccessors(), 0);

    auto bb_c = bb_a->GetSuccessor(1);
    ASSERT_NE(bb_c->GetFirstInst(), nullptr);
    auto i3 = bb_c->GetFirstInst();
    ASSERT_NE(i3, nullptr);
    ASSERT_EQ(i3->GetId(), I3);
    ASSERT_EQ(i3->GetOpcode(), isa::inst::Opcode::CHECK_SIZE);
    auto r1 = i3->GetNext();
    ASSERT_NE(r1, nullptr);
    ASSERT_EQ(r1->GetId(), R1);
    ASSERT_EQ(r1->GetOpcode(), isa::inst::Opcode::RETURN_VOID);
    ASSERT_EQ(r1->GetNext(), nullptr);
    ASSERT_EQ(bb_c->GetNumSuccessors(), 0);

    CheckInputs(c0, {});
    CheckUsers(c0, { { ARR, 0 }, { I0, 1 }, { IF0, 0 }, { IF0, 1 } });

    CheckInputs(c1, {});
    CheckUsers(c1, { { I2, 1 } });

    CheckInputs(c3, {});
    CheckUsers(c3, { { I3, 1 } });

    CheckInputs(arr, { { C0, START } });
    CheckUsers(arr, { { I0, 0 }, { I2, 0 }, { I3, 0 } });

    CheckInputs(i0, { { ARR, A }, { C0, START } });
    CheckUsers(i0, {});

    CheckInputs(i2, { { ARR, A }, { C1, START } });
    CheckUsers(i2, {});

    CheckInputs(i3, { { ARR, A }, { C3, START } });
    CheckUsers(i3, {});

    CheckInputs(r0, {});
    CheckUsers(r0, {});

    CheckInputs(r1, {});
    CheckUsers(r1, {});
}

TEST(TestCheckElimination, TestRemoveRedundantChecks)
{
    /*
    +-------+
    | START |
    +-------+
      |
      |
      v
    +-------+
    |   A   |
    +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C1 = b.NewConst(0U);
    auto C0 = b.NewConst(1U);
    auto C3 = b.NewConst(0U);
    auto C2 = b.NewConst(1U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::CHECK_NULL>();
    auto I1 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I2 = b.NewInst<isa::inst::Opcode::CHECK_NULL>();
    auto I3 = b.NewInst<isa::inst::Opcode::CHECK_NULL>();
    auto I4 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I5 = b.NewInst<isa::inst::Opcode::CHECK_NULL>();
    auto I6 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I7 = b.NewInst<isa::inst::Opcode::CHECK_NULL>();
    auto I8 = b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(I0, C1);
    b.SetInputs(I1, C3);
    b.SetInputs(I2, C0);
    b.SetInputs(I3, C2);
    b.SetInputs(I4, C1);
    b.SetInputs(I5, C3);
    b.SetInputs(I6, C1);
    b.SetInputs(I7, C0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<CheckElimination>();
    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<DCE>();
    g.GetAnalyser()->GetValidPass<DBE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto c1 = bb_start->GetFirstInst();
    ASSERT_EQ(c1->GetId(), C1);
    ASSERT_NE(c1->GetNext(), nullptr);
    auto c3 = c1->GetNext();
    ASSERT_EQ(c3->GetId(), C3);
    ASSERT_EQ(c3->GetNext(), nullptr);
    ASSERT_EQ(bb_start->GetNumSuccessors(), 1);

    auto bb_a = bb_start->GetSuccessor(0);
    ASSERT_NE(bb_a, nullptr);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i0 = bb_a->GetFirstInst();
    ASSERT_EQ(i0->GetId(), I0);
    ASSERT_EQ(i0->GetOpcode(), isa::inst::Opcode::CHECK_NULL);
    ASSERT_NE(i0->GetNext(), nullptr);
    auto i1 = i0->GetNext();
    ASSERT_EQ(i1->GetId(), I1);
    ASSERT_EQ(i1->GetOpcode(), isa::inst::Opcode::CHECK_ZERO);
    ASSERT_NE(i1->GetNext(), nullptr);
    auto i4 = i1->GetNext();
    ASSERT_EQ(i4->GetId(), I4);
    ASSERT_EQ(i4->GetOpcode(), isa::inst::Opcode::CHECK_ZERO);
    ASSERT_NE(i4->GetNext(), nullptr);
    auto i5 = i4->GetNext();
    ASSERT_NE(i5, nullptr);
    ASSERT_EQ(i5->GetId(), I5);
    ASSERT_EQ(i5->GetOpcode(), isa::inst::Opcode::CHECK_NULL);
    auto i8 = i5->GetNext();
    ASSERT_NE(i8, nullptr);
    ASSERT_EQ(i8->GetId(), I8);
    ASSERT_EQ(i8->GetOpcode(), isa::inst::Opcode::RETURN_VOID);
    ASSERT_EQ(i8->GetNext(), nullptr);
    ASSERT_EQ(bb_a->GetNumSuccessors(), 0);

    CheckInputs(c1, {});
    CheckUsers(c1, { { I0, 0 }, { I4, 0 } });

    CheckInputs(c3, {});
    CheckUsers(c3, { { I1, 0 }, { I5, 0 } });

    CheckInputs(i0, { { C1, START } });
    CheckUsers(i0, {});

    CheckInputs(i1, { { C3, START } });
    CheckUsers(i1, {});

    CheckInputs(i4, { { C1, START } });
    CheckUsers(i4, {});

    CheckInputs(i5, { { C3, START } });
    CheckUsers(i5, {});

    CheckInputs(i8, {});
    CheckUsers(i8, {});
}

TEST(TestCheckElimination, TestWithPeepholes)
{
    /*
    +-------+
    | START |
    +-------+
      |
      |
      v
    +-------+
    |   A   |
    +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(0U);
    auto C1 = b.NewConst(10U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::ADD>();
    auto I1 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto R0 = b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(I0, C0, C1);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<CheckElimination>();
    g.GetAnalyser()->GetValidPass<DCE>();
    g.GetAnalyser()->GetValidPass<DBE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_EQ(bb_start->GetFirstInst(), nullptr);
    ASSERT_EQ(bb_start->GetNumSuccessors(), 1);

    auto bb_a = bb_start->GetSuccessor(0);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto r0 = bb_a->GetFirstInst();
    ASSERT_NE(r0, nullptr);
    ASSERT_EQ(r0->GetId(), R0);
    ASSERT_EQ(r0->GetOpcode(), isa::inst::Opcode::RETURN_VOID);
    ASSERT_EQ(r0->GetNext(), nullptr);
    ASSERT_EQ(bb_a->GetNumSuccessors(), 0);
}

TEST(TestCheckElimination, TestNoRemoveNotEquivalent)
{
    /*
    +-------+
    | START |
    +-------+
      |
      |
      v
    +-------+
    |   A   |
    +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(0U);
    auto C1 = b.NewConst(0U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto I1 = b.NewInst<isa::inst::Opcode::CHECK_ZERO>();
    auto R0 = b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(I0, C0);
    b.SetInputs(I1, C1);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<Peepholes>();
    g.GetAnalyser()->GetValidPass<CheckElimination>();
    g.GetAnalyser()->GetValidPass<DCE>();
    g.GetAnalyser()->GetValidPass<DBE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto c0 = bb_start->GetFirstInst();
    ASSERT_EQ(c0->GetId(), C0);
    ASSERT_NE(c0->GetNext(), nullptr);
    auto c1 = c0->GetNext();
    ASSERT_EQ(c1->GetId(), C1);
    ASSERT_EQ(c1->GetNext(), nullptr);
    ASSERT_EQ(bb_start->GetNumSuccessors(), 1);

    auto bb_a = bb_start->GetSuccessor(0);
    ASSERT_NE(bb_a, nullptr);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i0 = bb_a->GetFirstInst();
    ASSERT_EQ(i0->GetId(), I0);
    ASSERT_EQ(i0->GetOpcode(), isa::inst::Opcode::CHECK_ZERO);
    ASSERT_NE(i0->GetNext(), nullptr);
    auto i1 = i0->GetNext();
    ASSERT_EQ(i1->GetId(), I1);
    ASSERT_EQ(i1->GetOpcode(), isa::inst::Opcode::CHECK_ZERO);
    auto r0 = i1->GetNext();
    ASSERT_NE(r0, nullptr);
    ASSERT_EQ(r0->GetId(), R0);
    ASSERT_EQ(r0->GetOpcode(), isa::inst::Opcode::RETURN_VOID);
    ASSERT_EQ(r0->GetNext(), nullptr);

    CheckInputs(c0, {});
    CheckUsers(c0, { { I0, 0 } });

    CheckInputs(c1, {});
    CheckUsers(c1, { { I1, 0 } });

    CheckInputs(i0, { { C0, START } });
    CheckUsers(i0, {});

    CheckInputs(i1, { { C1, START } });
    CheckUsers(i1, {});

    CheckInputs(r0, {});
    CheckUsers(r0, {});
}
