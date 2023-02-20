#include "bb.h"
#include "graph.h"
#include "graph_builder.h"

#include "gtest/gtest.h"

TEST(BasicTests, Example1)
{
    // SSA form of
    // uint64_t fact(uint32_t n)
    // {
    //     uint64_t res{ 1U };
    //     for (uint32_t i{ 2U }; i <= n; ++i) {
    //         res *= i;
    //     }
    //     return res;
    // }

    Graph g;
    GraphBuilder b(&g);

    auto p0 = b.NewParameter();

    auto c0 = b.NewConst(1U); // res{1U}
    b.SetType(c0, DataType::INT);
    auto c1 = b.NewConst(2U); // i{2U}
    b.SetType(c1, DataType::INT);

    auto b0 = b.NewBlock();
    auto i0 = b.NewInst<Opcode::PHI>(); // i
    auto i1 = b.NewInst<Opcode::PHI>(); // res
    auto i2 = b.NewInst<Opcode::IF>(CondType::COND_LEQ);

    auto b1 = b.NewBlock();
    auto i3 = b.NewInst<Opcode::MUL>();
    auto i4 = b.NewInst<Opcode::ADDI>(10);

    auto b2 = b.NewBlock();
    auto i5 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(i0, { { c1, Graph::BB_START_ID }, { i4, b1 } });
    b.SetInputs(i2, i0, p0);
    b.SetInputs(i1, { { c0, Graph::BB_START_ID }, { i3, b1 } });
    b.SetInputs(i3, i1, i0);
    b.SetInputs(i4, i0);
    b.SetInputs(i5, i1);

    b.SetSuccessors(Graph::BB_START_ID, { b0 });
    b.SetSuccessors(b0, { b1, b2 });
    b.SetSuccessors(b1, { b0 });
    b.SetSuccessors(b2, {});

    b.ConstructCFG();
    b.ConstructDFG();
    b.RunChecks();

    ASSERT_TRUE(g.GetStartBasicBlock() != nullptr);

    auto bb = g.GetStartBasicBlock();
    auto bb_pred = bb->GetPredecesors();
    auto bb_succ = bb->GetSuccessors();

    ASSERT_TRUE(bb_pred.empty());
    ASSERT_EQ(bb_succ.size(), 1);
    ASSERT_EQ(bb_succ[0]->GetId(), b0);

    bb = bb_succ[0];
    bb_pred = bb->GetPredecesors();
    bb_succ = bb->GetSuccessors();

    ASSERT_EQ(bb->GetId(), b0);
    ASSERT_EQ(bb_succ.size(), 2);
    ASSERT_EQ(bb_succ[0]->GetId(), b1);
    ASSERT_EQ(bb_succ[1]->GetId(), b2);
    ASSERT_EQ(bb_pred.size(), 2);
    ASSERT_EQ(bb_pred[0]->GetId(), b1);
    ASSERT_EQ(bb_pred[1]->GetId(), g.BB_START_ID);

    bb = bb_succ[0];
    auto bb2 = bb_succ[1];
    bb_pred = bb->GetPredecesors();
    bb_succ = bb->GetSuccessors();

    ASSERT_EQ(bb->GetId(), b1);
    ASSERT_EQ(bb_succ.size(), 1);
    ASSERT_EQ(bb_succ[0]->GetId(), b0);
    ASSERT_EQ(bb_pred.size(), 1);
    ASSERT_EQ(bb_pred[0]->GetId(), b0);

    bb_pred = bb2->GetPredecesors();
    bb_succ = bb2->GetSuccessors();

    ASSERT_EQ(bb2->GetId(), b2);
    ASSERT_EQ(bb_succ.size(), 0);
    ASSERT_EQ(bb_pred.size(), 1);
    ASSERT_EQ(bb_pred[0]->GetId(), b0);
}
