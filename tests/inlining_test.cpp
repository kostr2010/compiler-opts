#include "bb.h"
#include "graph.h"
#include "graph_builder.h"
#include "macros.h"

#include "gtest/gtest.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

TEST(TestPeepholes, Example0)
{
    /*
              ret
    */
    Graph callee;
    {
        GraphBuilder b{ &callee };

        auto START = Graph::BB_START_ID;
        auto P0 = b.NewParameter();

        auto A = b.NewBlock();
        auto I0 = b.NewInst<Opcode::RETURN>();

        b.SetInputs(I0, P0);

        b.SetSuccessors(START, { A });

        b.ConstructCFG();
        b.ConstructDFG();
        ASSERT_TRUE(b.RunChecks());
    }

    Graph caller;
    {
        GraphBuilder b(&caller);

        auto START = Graph::BB_START_ID;
        auto P0 = b.NewParameter();

        auto A = b.NewBlock();
        auto I0 = b.NewInst<Opcode::CALL_STATIC>(&callee);
        auto I1 = b.NewInst<Opcode::RETURN>();

        b.SetInputs(I0, P0);
        b.SetInputs(I1, I0);

        b.SetSuccessors(START, { A });

        b.ConstructCFG();
        b.ConstructDFG();
        ASSERT_TRUE(b.RunChecks());
    }

    caller.GetAnalyser()->GetValidPass<Inlining>();
    caller.GetAnalyser()->GetValidPass<DCE>();
    caller.GetAnalyser()->GetValidPass<DBE>();

    DUMP(caller);
}

TEST(TestPeepholes, Example1)
{
    /*
            .--A--.
            |     |
            v     v
            B     C
            |     |
            .->D<-.
               |
               v
               E
              ret
    */
    Graph callee;
    {
        GraphBuilder b{ &callee };

        auto START = Graph::BB_START_ID;
        auto P0 = b.NewParameter();
        auto P1 = b.NewParameter();
        auto C0 = b.NewConst(0);
        auto C1 = b.NewConst(-2);

        auto A = b.NewBlock();
        auto I0 = b.NewInst<Opcode::IF>(CondType::COND_EQ);

        auto B = b.NewBlock();
        auto I1 = b.NewInst<Opcode::MUL>();

        auto C = b.NewBlock();
        auto I2 = b.NewInst<Opcode::ADD>();

        auto D = b.NewBlock();
        auto I3 = b.NewInst<Opcode::PHI>();
        auto I4 = b.NewInst<Opcode::RETURN>();

        b.SetInputs(I0, P0, C1);
        b.SetInputs(I1, C1, P0);
        b.SetInputs(I2, C0, P1);
        b.SetInputs(I3, { { I1, B }, { I2, C } });
        b.SetInputs(I4, I3);

        b.SetSuccessors(START, { A });
        b.SetSuccessors(A, { B, C });
        b.SetSuccessors(B, { D });
        b.SetSuccessors(C, { D });

        b.ConstructCFG();
        b.ConstructDFG();
        ASSERT_TRUE(b.RunChecks());
    }

    Graph caller;
    {
        GraphBuilder b(&caller);

        auto START = Graph::BB_START_ID;
        auto P0 = b.NewParameter();
        auto C0 = b.NewConst(-2);

        auto A = b.NewBlock();
        auto I0 = b.NewInst<Opcode::CALL_STATIC>(&callee);
        auto I1 = b.NewInst<Opcode::RETURN>();

        b.SetInputs(I0, C0, P0);
        b.SetInputs(I1, I0);

        b.SetSuccessors(START, { A });

        b.ConstructCFG();
        b.ConstructDFG();
        ASSERT_TRUE(b.RunChecks());
    }

    DUMP(caller);
    DUMP(callee);

    caller.GetAnalyser()->GetValidPass<Inlining>();
    caller.GetAnalyser()->GetValidPass<DCE>();
    caller.GetAnalyser()->GetValidPass<DBE>();

    DUMP(caller);
    DUMP(callee);
}

#pragma GCC diagnostic pop
