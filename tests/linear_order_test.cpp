#include "bb.h"
#include "graph.h"
#include "graph_builder.h"

#include "gtest/gtest.h"

TEST(TestLinearOrder, Example0)
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
    +---+     +-------+
    | C | <-- |   B   |
    +---+     +-------+
      |         |
      |         |
      |         v
      |       +-------+     +---+
      |       |   F   | --> | G |
      |       +-------+     +---+
      |         |             |
      |         |             |
      |         v             |
      |       +-------+       |
      |       |   E   |       |
      |       +-------+       |
      |         |             |
      |         |             |
      |         v             |
      |       +-------+       |
      +-----> |   D   | <-----+
              +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(2);

    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    (void)b.NewInst<isa::inst::Opcode::RETURN_VOID>();
    auto E = b.NewBlock();
    auto F = b.NewBlock();
    auto IF1 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto G = b.NewBlock();

    b.SetInputs(IF0, C0, C1);
    b.SetInputs(IF1, C0, C1);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { F, C });
    b.SetSuccessors(C, { D });
    b.SetSuccessors(D, {});
    b.SetSuccessors(E, { D });
    b.SetSuccessors(F, { E, G });
    b.SetSuccessors(G, { D });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<LinearOrder>();
}

TEST(TestLinearOrder, Example0_inverted)
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
    +---+     +-------+
    | C | <-- |   B   |
    +---+     +-------+
      |         |
      |         |
      |         v
      |       +-------+     +---+
      |       |   F   | --> | G |
      |       +-------+     +---+
      |         |             |
      |         |             |
      |         v             |
      |       +-------+       |
      |       |   E   |       |
      |       +-------+       |
      |         |             |
      |         |             |
      |         v             |
      |       +-------+       |
      +-----> |   D   | <-----+
              +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(2);

    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    (void)b.NewInst<isa::inst::Opcode::RETURN_VOID>();
    auto E = b.NewBlock();
    auto F = b.NewBlock();
    auto IF1 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto G = b.NewBlock();

    b.SetInputs(IF0, C0, C1);
    b.SetInputs(IF1, C0, C1);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { C, F });
    b.SetSuccessors(C, { D });
    b.SetSuccessors(D, {});
    b.SetSuccessors(E, { D });
    b.SetSuccessors(F, { G, E });
    b.SetSuccessors(G, { D });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<LinearOrder>();
}

TEST(TestLinearOrder, Example1)
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
    +---+     +-------+
    | C | <-- |   B   |
    +---+     +-------+
      |         |
      |         |
      |         v
      |       +-------+
      |       |   F   |
      |       +-------+
      |         |
      |         |
      |         v
      |       +-------+
      |       |   E   |
      |       +-------+
      |         |
      |         |
      |         v
      |       +-------+
      +-----> |   D   |
              +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(2);

    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    (void)b.NewInst<isa::inst::Opcode::RETURN_VOID>();
    auto E = b.NewBlock();
    auto F = b.NewBlock();

    b.SetInputs(IF0, C0, C1);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { F, C });
    b.SetSuccessors(C, { D });
    b.SetSuccessors(D, {});
    b.SetSuccessors(E, { D });
    b.SetSuccessors(F, { E });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<LinearOrder>();
}

TEST(TestLinearOrder, Diamond)
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
      |         |
      |         |
      |         v
      |       +-------+
      |       |   B   |
      |       +-------+
      |         |
      |         |
      |         v
      |       +-------+
      +-----> |   D   |
              +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(2);

    auto A = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto B = b.NewBlock();
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    (void)b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(IF0, C0, C1);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B, C });
    b.SetSuccessors(B, { D });
    b.SetSuccessors(C, { D });
    b.SetSuccessors(D, {});

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<LinearOrder>();
}

TEST(TestLinearOrder, DoubleDiamond)
{
    /*

      +----------------------------+
      |                            |
      |                 +-------+  |
      |                 | START |  |
      |                 +-------+  |
      |                   |        |
      |                   |        |
      v                   v        |
    +---+     +---+     +-------+  |
    | E | <-- | C | <-- |   A   |  |
    +---+     +---+     +-------+  |
                |         |        |
                |         |        |
                |         v        |
                |       +-------+  |
                |       |   B   | -+
                |       +-------+
                |         |
                |         |
                |         v
                |       +-------+
                +-----> |   D   |
                        +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(2);

    auto A = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto B = b.NewBlock();
    auto IF1 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto C = b.NewBlock();
    auto IF2 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto D = b.NewBlock();
    (void)b.NewInst<isa::inst::Opcode::RETURN_VOID>();
    auto E = b.NewBlock();
    (void)b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(IF0, C0, C1);
    b.SetInputs(IF1, C0, C1);
    b.SetInputs(IF2, C0, C1);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B, C });
    b.SetSuccessors(B, { D, E });
    b.SetSuccessors(C, { D, E });
    b.SetSuccessors(D, {});
    b.SetSuccessors(E, {});

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<LinearOrder>();
}

TEST(TestLinearOrder, TripleDiamond)
{
    /*

       +---------------------------------+
       |                                 |
       |                 +-------+       |
       |                 | START |       |
       |                 +-------+       |
       |                   |             |
       |                   |             |
       v                   v             |
     +---+     +---+     +-------+       |
  +> | G | <-- | C | <-- |   A   |       |
  |  +---+     +---+     +-------+       |
  |              |         |             |
  |              |         |             |
  |              |         v             |
  |              |       +-------+     +---+
  |              |       |   B   | --> | E |
  |              |       +-------+     +---+
  |              |         |             |
  |              |         |             |
  |              |         v             |
  |              |       +-------+       |
  +--------------+------ |   D   |       |
                 |       +-------+       |
                 |         |             |
                 |         |             |
                 |         v             |
                 |       +-------+       |
                 +-----> |   F   | <-----+
                         +-------+
*/

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(2);

    auto A = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto B = b.NewBlock();
    auto IF1 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto C = b.NewBlock();
    auto IF2 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto D = b.NewBlock();
    auto IF3 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto E = b.NewBlock();
    auto IF4 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto F = b.NewBlock();
    (void)b.NewInst<isa::inst::Opcode::RETURN_VOID>();
    auto G = b.NewBlock();
    (void)b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(IF0, C0, C1);
    b.SetInputs(IF1, C0, C1);
    b.SetInputs(IF2, C0, C1);
    b.SetInputs(IF3, C0, C1);
    b.SetInputs(IF4, C0, C1);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { C, B });
    b.SetSuccessors(B, { D, E });
    b.SetSuccessors(C, { F, G });
    b.SetSuccessors(D, { F, G });
    b.SetSuccessors(E, { F, G });
    b.SetSuccessors(F, {});
    b.SetSuccessors(G, {});

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<LinearOrder>();
}

TEST(TestLinearOrder, LoopExample2)
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
      +-----> |   B   | -+
      |       +-------+  |
      |         |        |
      |         |        |
      |         v        |
      |       +-------+  |
      |       |   J   |  |
      |       +-------+  |
      |         |        |
      |         |        |
      |         v        |
      |       +-------+  |
      |    +> |   C   | <+
      |    |  +-------+
      |    |    |
      |    |    |
      |    |    v
      |    |  +-------+
      |    +- |   D   |
      |       +-------+
      |         |
      |         |
      |         v
      |       +-------+
      |       |   E   | <+
      |       +-------+  |
      |         |        |
      |         |        |
      |         v        |
      |       +-------+  |
      |       |   F   | -+
      |       +-------+
      |         |
      |         |
      |         v
    +---+     +-------+
    | H | <-- |   G   |
    +---+     +-------+
                |
                |
                v
              +-------+
              |   I   |
              +-------+
                |
                |
                v
              +-------+
              |   K   |
              +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(2);

    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    auto IF1 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto E = b.NewBlock();
    auto F = b.NewBlock();
    auto IF2 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto G = b.NewBlock();
    auto IF3 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto H = b.NewBlock();
    auto I = b.NewBlock();
    auto J = b.NewBlock();
    auto K = b.NewBlock();
    (void)b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(IF0, C0, C1);
    b.SetInputs(IF1, C0, C1);
    b.SetInputs(IF2, C0, C1);
    b.SetInputs(IF3, C0, C1);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { J, C });
    b.SetSuccessors(C, { D });
    b.SetSuccessors(D, { E, C });
    b.SetSuccessors(E, { F });
    b.SetSuccessors(F, { G, E });
    b.SetSuccessors(G, { I, H });
    b.SetSuccessors(H, { B });
    b.SetSuccessors(I, { K });
    b.SetSuccessors(J, { C });
    b.SetSuccessors(K, {});

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<LinearOrder>();
}

TEST(TestLinearOrder, LoopExamplePhi)
{
    /*
         +-------+
         | START |
         +-------+
           |
           |
           v
         +-----------------+
      +> |        A        |
      |  +-----------------+
      |    |        ^    ^
      |    |        |    |
      |    v        |    |
      |  +-------+  |    |
      |  |   B   | -+    |
      |  +-------+       |
      |    |             |
      |    |             |
      |    v             |
      |  +-------+       |
      |  |   C   | ------+
      |  +-------+
      |    |
      |    |
      |    v
      |  +-------+
      +- |   D   |
         +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::PHI>();
    auto I1 = b.NewInst<isa::inst::Opcode::PHI>();

    auto B = b.NewBlock();
    auto I2 = b.NewInst<isa::inst::Opcode::ADDI>();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

    auto C = b.NewBlock();
    auto I3 = b.NewInst<isa::inst::Opcode::ADDI>();
    auto IF1 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

    auto D = b.NewBlock();
    auto I4 = b.NewInst<isa::inst::Opcode::ADDI>();

    b.SetInputs(IF0, C0, C0);
    b.SetInputs(IF1, C0, C0);
    b.SetInputs(I0, { { C0, Graph::BB_START_ID }, { I2, B }, { I3, C }, { I4, D } });
    b.SetInputs(I1, { { C0, Graph::BB_START_ID }, { I2, B }, { I3, C } });
    b.SetInputs(I2, C0);
    b.SetImmediate(I2, 0, 10);
    b.SetInputs(I3, C0);
    b.SetImmediate(I3, 0, 10);
    b.SetInputs(I4, C0);
    b.SetImmediate(I4, 0, 10);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { A, C });
    b.SetSuccessors(C, { A, D });
    b.SetSuccessors(D, { A });

    b.ConstructCFG();
    b.ConstructDFG();

    g.GetAnalyser()->GetValidPass<LinearOrder>();
}

TEST(TestLinearOrder, LoopExampleSeparateBck)
{
    /*
             +-------+
             | START |
             +-------+
               |
               |
               v
             +-----------------+
          +> |        E        | <+
          |  +-----------------+  |
          |    |        ^    ^    |
          |    |        |    |    |
          |    v        |    |    |
          |  +-------+  |    |    |
          |  |   D   | -+    |    |
          |  +-------+       |    |
          |    |             |    |
          |    |             |    |
          |    v             |    |
          |  +-------+       |    |
          |  |   C   | ------+    |
          |  +-------+            |
          |    |                  |
          |    |                  |
          |    v                  |
          |  +-------+            |
          +- |   B   |            |
             +-------+            |
               |                  |
               |                  |
               v                  |
             +-------+            |
             |   A   | -----------+
             +-------+
        */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(2);
    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto C = b.NewBlock();
    auto IF1 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto D = b.NewBlock();
    auto IF2 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto E = b.NewBlock();

    b.SetInputs(IF0, C0, C1);
    b.SetInputs(IF1, C0, C1);
    b.SetInputs(IF2, C0, C1);

    b.SetSuccessors(START, { E });
    b.SetSuccessors(A, { E });
    b.SetSuccessors(B, { A, E });
    b.SetSuccessors(C, { E, B });
    b.SetSuccessors(D, { C, E });
    b.SetSuccessors(E, { D });

    b.ConstructCFG();
    b.ConstructDFG();
    g.GetAnalyser()->GetValidPass<LinearOrder>();
}

TEST(TestLinearOrder, LoopExample4)
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
    +---+     +-------+
    | C | <-- |   B   | <+
    +---+     +-------+  |
                |        |
                |        |
                v        |
              +-------+  |
              |   D   |  |
              +-------+  |
                |        |
                |        |
                v        |
              +-------+  |
              |   E   | -+
              +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(2);
    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto C = b.NewBlock();
    (void)b.NewInst<isa::inst::Opcode::RETURN_VOID>();
    auto D = b.NewBlock();
    auto E = b.NewBlock();

    b.SetInputs(IF0, C0, C1);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { D, C });
    b.SetSuccessors(C, {});
    b.SetSuccessors(D, { E });
    b.SetSuccessors(E, { B });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    g.GetAnalyser()->GetValidPass<LinearOrder>();
}

TEST(TestLinearOrder, LoopExample5)
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
              |   B   | <+
              +-------+  |
                |        |
                |        |
                v        |
    +---+     +-------+  |
    | F | <-- |   C   |  |
    +---+     +-------+  |
      ^         |        |
      |         |        |
      |         v        |
      |       +-------+  |
      +------ |   D   |  |
              +-------+  |
                |        |
                |        |
                v        |
              +-------+  |
              |   E   | -+
              +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(2);
    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto C = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto D = b.NewBlock();
    auto IF1 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto E = b.NewBlock();
    auto F = b.NewBlock();
    (void)b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(IF0, C0, C1);
    b.SetInputs(IF1, C0, C1);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { C });
    b.SetSuccessors(C, { D, F });
    b.SetSuccessors(D, { E, F });
    b.SetSuccessors(E, { B });
    b.SetSuccessors(F, {});

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());
    g.GetAnalyser()->GetValidPass<LinearOrder>();
}

TEST(TestLinearOrder, LoopExample6)
{
    /*
                   +-------+
                   | START |
                   +-------+
                     |
                     |
                     v
                   +-------+
                +> |   A   |
                |  +-------+
                |    |
                |    |
                |    v
    +---+       |  +-------+
    | D | <-----+- |   B   | <+
    +---+       |  +-------+  |
      |         |    |        |
      |         |    |        |
      |         |    v        |
      |         |  +-------+  |       +---+
      |         |  |   C   | -+-----> | F |
      |         |  +-------+  |       +---+
      |         |    |        |
      |         |    |        |
      |         |    v        |
      |         |  +-------+  |
      +---------+> |   E   |  |
                |  +-------+  |
                |    |        |
                |    |        |
                |    v        |
                |  +-------+  |
                |  |   G   | -+
                |  +-------+
                |    |
                |    |
                |    v
                |  +-------+
                +- |   H   |
                   +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(2);
    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto C = b.NewBlock();
    auto IF1 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto D = b.NewBlock();
    auto E = b.NewBlock();
    auto F = b.NewBlock();
    (void)b.NewInst<isa::inst::Opcode::RETURN_VOID>();
    auto G = b.NewBlock();
    auto IF2 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto H = b.NewBlock();

    b.SetInputs(IF0, C0, C1);
    b.SetInputs(IF1, C0, C1);
    b.SetInputs(IF2, C0, C1);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { C, D });
    b.SetSuccessors(C, { E, F });
    b.SetSuccessors(D, { E });
    b.SetSuccessors(E, { G });
    b.SetSuccessors(F, {});
    b.SetSuccessors(G, { H, B });
    b.SetSuccessors(H, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());
    g.GetAnalyser()->GetValidPass<LinearOrder>();
}

TEST(TestLinearOrder, LoopExample3)
{
    /*
           +----------------------------+
           |                            |
           |                 +-------+  |
           |                 | START |  |
           |                 +-------+  |
           |                   |        |
           |                   |        |
           |                   v        |
           |                 +-------+  |
           |                 |   A   |  |
           |                 +-------+  |
           |                   |        |
           |                   |        |
           |                   v        |
         +---+     +---+     +-------+  |
         | F | <-- | E | <-- |   B   | <+
         +---+     +---+     +-------+
           |         |         |
           |         |         |
           v         |         v
         +---+       |       +-------+
      +- | H |       |       |   C   | <+
      |  +---+       |       +-------+  |
      |    |         |         |        |
      |    |         |         |        |
      |    |         |         v        |
      |    |         |       +-------+  |
      |    |         +-----> |   D   |  |
      |    |                 +-------+  |
      |    |                   |        |
      |    |                   |        |
      |    |                   v        |
      |    |                 +-------+  |
      |    +---------------> |   G   | -+
      |                      +-------+
      |                        |
      |                        |
      |                        v
      |                      +-------+
      |                      |   I   |
      |                      +-------+
      |                        ^
      +------------------------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(2);

    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    auto E = b.NewBlock();
    auto IF1 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto F = b.NewBlock();
    auto IF2 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto G = b.NewBlock();
    auto IF3 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto H = b.NewBlock();
    auto IF4 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);
    auto I = b.NewBlock();
    (void)b.NewInst<isa::inst::Opcode::RETURN_VOID>();

    b.SetInputs(IF0, C0, C1);
    b.SetInputs(IF1, C0, C1);
    b.SetInputs(IF2, C0, C1);
    b.SetInputs(IF3, C0, C1);
    b.SetInputs(IF4, C0, C1);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { E, C });
    b.SetSuccessors(C, { D });
    b.SetSuccessors(D, { G });
    b.SetSuccessors(E, { F, D });
    b.SetSuccessors(F, { H, B });
    b.SetSuccessors(G, { I, C });
    b.SetSuccessors(H, { I, G });
    b.SetSuccessors(I, {});

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());
    g.GetAnalyser()->GetValidPass<LinearOrder>();
}
