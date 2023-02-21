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
              ret P0
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
        auto C1 = b.NewConst(2);

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
        auto C0 = b.NewConst(2);

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

TEST(TestInlining, Example2)
{
    /*
            A<-.
            |  |
            v  |
            B--.
            |
            v
            C
         ret_void
    */
    Graph callee;
    {
        GraphBuilder b{ &callee };

        auto START = Graph::BB_START_ID;
        auto P0 = b.NewParameter();
        auto C0 = b.NewConst(0);
        auto C1 = b.NewConst(2);

        auto A = b.NewBlock();
        auto I0 = b.NewInst<Opcode::MUL>();

        auto B = b.NewBlock();
        auto I1 = b.NewInst<Opcode::ADD>();
        auto I2 = b.NewInst<Opcode::IF>(CondType::COND_EQ);

        auto C = b.NewBlock();
        auto I3 = b.NewInst<Opcode::RETURN_VOID>();

        b.SetInputs(I0, P0, C0);
        b.SetInputs(I1, C1, P0);
        b.SetInputs(I2, I0, I1);

        b.SetSuccessors(START, { A });
        b.SetSuccessors(A, { B });
        b.SetSuccessors(B, { A, C });

        b.ConstructCFG();
        b.ConstructDFG();
        ASSERT_TRUE(b.RunChecks());
    }

    Graph caller;
    {
        GraphBuilder b(&caller);

        auto START = Graph::BB_START_ID;
        auto P0 = b.NewParameter();
        auto C0 = b.NewConst(2);

        auto A = b.NewBlock();
        auto I0 = b.NewInst<Opcode::MUL>();
        auto I1 = b.NewInst<Opcode::CALL_STATIC>(&callee);
        auto I2 = b.NewInst<Opcode::RETURN>();

        b.SetInputs(I0, C0, P0);
        b.SetInputs(I1, I0);
        b.SetInputs(I2, I0);

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

TEST(TestInlining, Example3)
{
    /*
            .--A--.
            |     |
            v     v
            B     C
           ret   ret
    */
    Graph callee;
    {
        GraphBuilder b{ &callee };

        auto START = Graph::BB_START_ID;
        auto P0 = b.NewParameter();
        auto P1 = b.NewParameter();
        auto P2 = b.NewParameter();
        auto C0 = b.NewConst(0);

        auto A = b.NewBlock();
        auto I0 = b.NewInst<Opcode::MUL>();
        auto If = b.NewInst<Opcode::IF>(CondType::COND_EQ);

        auto B = b.NewBlock();
        auto I1 = b.NewInst<Opcode::ADD>();
        auto I2 = b.NewInst<Opcode::RETURN>();

        auto C = b.NewBlock();
        auto I3 = b.NewInst<Opcode::SUB>();
        auto I4 = b.NewInst<Opcode::RETURN>();

        b.SetInputs(I0, P2, C0);
        b.SetInputs(If, P2, I0);
        b.SetInputs(I1, I0, P0);
        b.SetInputs(I2, I1);
        b.SetInputs(I3, I0, P1);
        b.SetInputs(I4, I3);

        b.SetSuccessors(START, { A });
        b.SetSuccessors(A, { B, C });

        b.ConstructCFG();
        b.ConstructDFG();
        ASSERT_TRUE(b.RunChecks());
    }

    Graph caller;
    {
        GraphBuilder b(&caller);

        auto START = Graph::BB_START_ID;
        auto P0 = b.NewParameter();
        auto C0 = b.NewConst(2);
        auto C1 = b.NewConst(2);

        auto A = b.NewBlock();
        auto I0 = b.NewInst<Opcode::CALL_STATIC>(&callee);
        auto I1 = b.NewInst<Opcode::RETURN>();

        b.SetInputs(I0, C0, P0, C1);
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
