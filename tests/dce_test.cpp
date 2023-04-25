#include "bb.h"
#include "graph.h"
#include "graph_builder.h"

#include "gtest/gtest.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static void CheckUsers(InstBase* inst, std::set<std::pair<IdType, int> > expected)
{
    std::vector<std::pair<IdType, int> > res = {};
    for (const auto& u : inst->GetUsers()) {
        res.push_back({ u.GetInst()->GetId(), u.GetIdx() });
    }

    auto res_set = std::set<std::pair<IdType, int> >(res.begin(), res.end());
    ASSERT_EQ(res_set, expected);
}

static void CheckInputs(InstBase* inst, std::set<std::pair<IdType, IdType> > expected)
{
    std::vector<std::pair<IdType, IdType> > res = {};
    for (const auto& i : inst->GetInputs()) {
        res.push_back({ i.GetInst()->GetId(), i.GetSourceBB()->GetId() });
    }

    auto res_set = std::set<std::pair<IdType, IdType> >(res.begin(), res.end());
    ASSERT_EQ(res_set, expected);
}

TEST(TestDCE, Example1)
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
    auto C0 = b.NewConst(1U);
    auto C1 = b.NewConst(2U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::ADD>();
    auto I1 = b.NewInst<isa::inst::Opcode::RETURN>();

    b.SetInputs(I0, C1, C0);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto c0 = bb_start->GetFirstInst();
    ASSERT_EQ(c0->GetId(), C0);
    auto c1 = c0->GetNext();
    ASSERT_NE(c1, nullptr);
    ASSERT_EQ(c1->GetId(), C1);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i0 = bb_a->GetFirstInst();
    ASSERT_EQ(i0->GetId(), I0);
    auto i1 = i0->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(c0, {});
    CheckUsers(c0, { { I0, 1 } });

    CheckInputs(c1, {});
    CheckUsers(c1, { { I0, 0 } });

    CheckInputs(i0, { { C1, START }, { C0, START } });
    CheckUsers(i0, { { I1, 0 } });

    CheckInputs(i1, { { I0, A } });
    CheckUsers(i1, {});
}

TEST(TestDCE, Example2)
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
    auto C0 = b.NewConst(1U);
    auto C1 = b.NewConst(2U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::ADD>();
    auto I1 = b.NewInst<isa::inst::Opcode::RETURN>();

    b.SetInputs(I0, C1, C1);
    b.SetInputs(I1, I0);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto c1 = bb_start->GetFirstInst();
    ASSERT_EQ(c1->GetId(), C1);
    ASSERT_EQ(c1->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i0 = bb_a->GetFirstInst();
    ASSERT_EQ(i0->GetId(), I0);
    auto i1 = i0->GetNext();
    ASSERT_NE(i1, nullptr);
    ASSERT_EQ(i1->GetId(), I1);

    CheckInputs(c1, {});
    CheckUsers(c1, { { I0, 0 }, { I0, 1 } });

    CheckInputs(i0, { { C1, START }, { C1, START } });
    CheckUsers(i0, { { I1, 0 } });

    CheckInputs(i1, { { I0, A } });
    CheckUsers(i1, {});
}

TEST(TestDCE, Example3)
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
    auto P0 = b.NewParameter();
    auto C0 = b.NewConst(1U);
    auto C1 = b.NewConst(2U);
    auto C2 = b.NewConst(3U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::ADD>();
    auto I1 = b.NewInst<isa::inst::Opcode::SUB>();
    auto I2 = b.NewInst<isa::inst::Opcode::MUL>();
    auto I3 = b.NewInst<isa::inst::Opcode::DIV>();
    auto I4 = b.NewInst<isa::inst::Opcode::RETURN>();

    b.SetInputs(I0, C2, C1);
    b.SetInputs(I1, I0, C1);
    b.SetInputs(I2, C0, C1);
    b.SetInputs(I3, C1, I2);
    b.SetInputs(I4, I3);

    b.SetSuccessors(START, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_NE(p0, nullptr);
    ASSERT_EQ(p0->GetId(), P0);
    auto c0 = p0->GetNext();
    ASSERT_NE(c0, nullptr);
    ASSERT_EQ(c0->GetId(), C0);
    auto c1 = c0->GetNext();
    ASSERT_NE(c1, nullptr);
    ASSERT_EQ(c1->GetId(), C1);
    ASSERT_EQ(c1->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i2 = bb_a->GetFirstInst();
    ASSERT_EQ(i2->GetId(), I2);
    auto i3 = i2->GetNext();
    ASSERT_NE(i3, nullptr);
    ASSERT_EQ(i3->GetId(), I3);
    auto i4 = i3->GetNext();
    ASSERT_NE(i4, nullptr);
    ASSERT_EQ(i4->GetId(), I4);
    ASSERT_EQ(i4->GetNext(), nullptr);

    CheckInputs(p0, {});
    CheckUsers(p0, {});

    CheckInputs(c0, {});
    CheckUsers(c0, { { I2, 0 } });

    CheckInputs(c1, {});
    CheckUsers(c1, { { I3, 0 }, { I2, 1 } });

    CheckInputs(i2, { { C0, START }, { C1, START } });
    CheckUsers(i2, { { I3, 1 } });

    CheckInputs(i3, { { C1, START }, { I2, A } });
    CheckUsers(i3, { { I4, 0 } });

    CheckInputs(i4, { { I3, A } });
    CheckUsers(i4, {});
}

TEST(TestDCE, Example4)
{
    /*
              +-------+
              | START |
              +-------+
                |
                |
                v
    +---+     +-------+
    | B | <-- |   D   |
    +---+     +-------+
      |         |
      |         |
      |         v
      |       +-------+
      |       |   A   |
      |       +-------+
      |         |
      |         |
      |         v
      |       +-------+
      +-----> |   C   |
              +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto P0 = b.NewParameter();
    auto C0 = b.NewConst(1U);
    auto C1 = b.NewConst(2U);
    auto C2 = b.NewConst(3U);

    auto D = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::ADD>();
    auto I1 = b.NewInst<isa::inst::Opcode::SUB>();

    auto B = b.NewBlock();
    auto I2 = b.NewInst<isa::inst::Opcode::MUL>();
    auto I3 = b.NewInst<isa::inst::Opcode::DIV>();

    auto C = b.NewBlock();
    auto I4 = b.NewInst<isa::inst::Opcode::XOR>();
    auto I5 = b.NewInst<isa::inst::Opcode::AND>();
    auto I6 = b.NewInst<isa::inst::Opcode::PHI>();
    auto I7 = b.NewInst<isa::inst::Opcode::PHI>();
    auto I8 = b.NewInst<isa::inst::Opcode::RETURN>();

    b.SetInputs(IF0, P0, P0);
    b.SetInputs(I0, P0, C1);
    b.SetInputs(I1, I0, C1);
    b.SetInputs(I2, C0, C1);
    b.SetInputs(I3, C1, I2);
    b.SetInputs(I4, I3, I2);
    b.SetInputs(I5, C2, I1);
    b.SetInputs(I6, { { I3, B }, { I0, A } });
    b.SetInputs(I7, { { I2, B }, { I0, A } });
    b.SetInputs(I8, I6);

    b.SetSuccessors(START, { D });
    b.SetSuccessors(D, { A, B });
    b.SetSuccessors(A, { C });
    b.SetSuccessors(B, { C });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<DCE>();

    auto bb_start = g.GetBasicBlock(START);
    ASSERT_NE(bb_start->GetFirstInst(), nullptr);
    auto p0 = bb_start->GetFirstInst();
    ASSERT_EQ(p0->GetId(), P0);
    auto c0 = p0->GetNext();
    ASSERT_NE(c0, nullptr);
    ASSERT_EQ(c0->GetId(), C0);
    auto c1 = c0->GetNext();
    ASSERT_NE(c1, nullptr);
    ASSERT_EQ(c1->GetId(), C1);
    ASSERT_EQ(c1->GetNext(), nullptr);

    auto bb_d = g.GetBasicBlock(D);
    ASSERT_NE(bb_d->GetFirstInst(), nullptr);
    auto if0 = bb_d->GetFirstInst();
    ASSERT_EQ(if0->GetId(), IF0);
    ASSERT_EQ(if0->GetNext(), nullptr);

    auto bb_a = g.GetBasicBlock(A);
    ASSERT_NE(bb_a->GetFirstInst(), nullptr);
    auto i0 = bb_a->GetFirstInst();
    ASSERT_EQ(i0->GetId(), I0);
    ASSERT_EQ(i0->GetNext(), nullptr);

    auto bb_b = g.GetBasicBlock(B);
    ASSERT_NE(bb_b->GetFirstInst(), nullptr);
    auto i2 = bb_b->GetFirstInst();
    ASSERT_EQ(i2->GetId(), I2);
    ASSERT_NE(i2->GetNext(), nullptr);
    auto i3 = i2->GetNext();
    ASSERT_EQ(i3->GetId(), I3);
    ASSERT_EQ(i3->GetNext(), nullptr);

    auto bb_c = g.GetBasicBlock(C);
    ASSERT_NE(bb_c->GetFirstPhi(), nullptr);
    auto i6 = bb_c->GetFirstPhi();
    ASSERT_EQ(i6->GetId(), I6);
    ASSERT_EQ(i6->GetNext(), nullptr);
    ASSERT_NE(bb_c->GetFirstInst(), nullptr);
    auto i8 = bb_c->GetFirstInst();
    ASSERT_EQ(i8->GetId(), I8);
    ASSERT_EQ(i8->GetNext(), nullptr);

    CheckInputs(c0, {});
    CheckUsers(c0, { { I2, 0 } });

    CheckInputs(c1, {});
    CheckUsers(c1, { { I3, 0 }, { I2, 1 }, { I0, 1 } });

    CheckInputs(p0, {});
    CheckUsers(p0, { { I0, 0 }, { IF0, 0 }, { IF0, 1 } });

    CheckInputs(if0, { { P0, START }, { P0, START } });
    CheckUsers(if0, {});

    CheckInputs(i0, { { P0, START }, { C1, START } });
    CheckUsers(i0, { { I6, -1 } });

    CheckInputs(i2, { { C0, START }, { C1, START } });
    CheckUsers(i2, { { I3, 1 } });

    CheckInputs(i3, { { C1, START }, { I2, B } });
    CheckUsers(i3, { { I6, -1 } });

    CheckInputs(i6, { { I3, B }, { I0, A } });
    CheckUsers(i6, { { I8, 0 } });

    CheckInputs(i8, { { I6, C } });
    CheckUsers(i8, {});
}

#pragma GCC diagnostic pop
