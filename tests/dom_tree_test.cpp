#include "bb.h"
#include "graph.h"
#include "graph_builder.h"
#include "gtest/gtest.h"
#include <algorithm>

static inline char IdToChar(IdType id)
{
    return (id == 0) ? '0' : (char)('A' + id - 1);
}

TEST(TestDomTree, Example1)
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

    std::vector<char> rpo_c{};

    for (auto bb : g.GetPassManager()->GetValidPass<RPO>()->GetBlocks()) {
        rpo_c.push_back(IdToChar(bb->GetId()));
    }

    ASSERT_EQ(rpo_c, std::vector<char>({ '0', 'A', 'B', 'F', 'E', 'D', 'G', 'C' }));

    const auto CheckImmDoms = [&]() {
        auto bb = g.GetBasicBlock(START);
        ASSERT_EQ(bb->GetImmDominator(), nullptr);
        bb = g.GetBasicBlock(A);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(B);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), A);
        bb = g.GetBasicBlock(C);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(D);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(E);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), F);
        bb = g.GetBasicBlock(F);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(G);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), F);
    };

    g.GetPassManager()->RunPass<DomTree>();
    CheckImmDoms();
}

TEST(TestDomTree, Example2)
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

    std::vector<char> rpo_c{};

    for (auto bb : g.GetPassManager()->GetValidPass<RPO>()->GetBlocks()) {
        rpo_c.push_back(IdToChar(bb->GetId()));
    }
    ASSERT_EQ(rpo_c,
              std::vector<char>({ '0', 'A', 'B', 'J', 'C', 'D', 'E', 'F', 'G', 'I', 'K', 'H' }));

    const auto CheckImmDoms = [&]() {
        auto bb = g.GetBasicBlock(START);
        ASSERT_EQ(bb->GetImmDominator(), nullptr);
        bb = g.GetBasicBlock(A);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(B);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), A);
        bb = g.GetBasicBlock(C);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(D);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), C);
        bb = g.GetBasicBlock(E);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), D);
        bb = g.GetBasicBlock(F);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), E);
        bb = g.GetBasicBlock(G);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), F);
        bb = g.GetBasicBlock(H);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), G);
        bb = g.GetBasicBlock(I);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), G);
        bb = g.GetBasicBlock(J);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(K);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), I);
    };

    g.GetPassManager()->RunPass<DomTree>();
    CheckImmDoms();
}

TEST(TestDomTree, Example3)
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

    std::vector<char> rpo_c{};

    for (auto bb : g.GetPassManager()->GetValidPass<RPO>()->GetBlocks()) {
        rpo_c.push_back(IdToChar(bb->GetId()));
    }

    ASSERT_EQ(rpo_c, std::vector<char>({ '0', 'A', 'B', 'E', 'F', 'H', 'I', 'G', 'C', 'D' }));

    const auto CheckImmDoms = [&]() {
        auto bb = g.GetBasicBlock(START);
        ASSERT_EQ(bb->GetImmDominator(), nullptr);
        bb = g.GetBasicBlock(A);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(B);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), A);
        bb = g.GetBasicBlock(C);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(D);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(E);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(F);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), E);
        bb = g.GetBasicBlock(G);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(H);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), F);
        bb = g.GetBasicBlock(I);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
    };

    g.GetPassManager()->RunPass<DomTree>();
    CheckImmDoms();
}

TEST(TestDomTree, Example4)
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

    const auto CheckImmDoms = [&]() {
        auto bb = g.GetBasicBlock(START);
        ASSERT_EQ(bb->GetImmDominator(), nullptr);
        bb = g.GetBasicBlock(A);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(B);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), A);
        bb = g.GetBasicBlock(C);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(D);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(E);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), D);
    };

    g.GetPassManager()->RunPass<DomTree>();
    CheckImmDoms();
}

TEST(TestDomTree, Example5)
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

    const auto CheckImmDoms = [&]() {
        auto bb = g.GetBasicBlock(START);
        ASSERT_EQ(bb->GetImmDominator(), nullptr);
        bb = g.GetBasicBlock(A);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(B);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), A);
        bb = g.GetBasicBlock(C);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(D);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), C);
        bb = g.GetBasicBlock(E);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), D);
        bb = g.GetBasicBlock(F);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), C);
    };

    g.GetPassManager()->RunPass<DomTree>();
    CheckImmDoms();
}

TEST(TestDomTree, Example6)
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

    const auto CheckImmDoms = [&]() {
        auto bb = g.GetBasicBlock(START);
        ASSERT_EQ(bb->GetImmDominator(), nullptr);
        bb = g.GetBasicBlock(A);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(B);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), A);
        bb = g.GetBasicBlock(C);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(D);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(E);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), B);
        bb = g.GetBasicBlock(F);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), C);
        bb = g.GetBasicBlock(G);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), E);
        bb = g.GetBasicBlock(H);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), G);
    };

    g.GetPassManager()->RunPass<DomTree>();
    CheckImmDoms();
}

// disabled due to check for number of successors
// TEST(TestDomTree, ExampleArticle)
// {
/*

                                +----------------------------+
                                |                            |
                                |                            |
       +------------------------+-----------------------+    |
       |                        |                       |    |
       |                        |  +-------+            |    |
       |                        |  | START |            |    |
       |                        |  +-------+            |    |
       |                        |    |                  |    |
  +----+         +--------------+    |                  |    |
  |              |                   v                  |    |
  |  +---+     +---+     +---+     +------------+       |    |
  |  | J | <-- | G | <-- | C | <-- |     M      | <+    |    |
  |  +---+     +---+     +---+     +------------+  |    |    |
  |    |                   |         |        |    |    |    |
  |    |                   |         |        |    |    |    |
  |    |                   |         v        |    |    |    |
  |    |       +---+       |       +-------+  |    |    |    |
  |    |       | F | <-----+    +- |   B   | -+----+----+    |
  |    |       +---+            |  +-------+  |    |         |
  |    |         |              |    |        |    |         |
  |    |         |              |    |        |    |         |
  |    |         |              |    v        |    |         |
  |    |         |              |  +-------+  |    |         |
  |    |         |              |  |   A   | <+    |         |
  |    |         |              |  +-------+       |         |
  |    |         |              |    |             |         |
  |    |         |              |    |             |         |
  |    |         |              |    v             |         |
  |    |         |              |  +-------+       |         |
  |    |         |              +> |   D   |       |         |
  |    |         |                 +-------+       |         |
  |    |         |                   |             |         |
  |    |         |                   |             |         |
  |    |         |                   v             |         |
  |    |         |                 +-------+       |         |
  |    |         |                 |   L   |       |         |
  |    |         |                 +-------+       |         |
  |    |         |                   |             |         |
  |    |         |                   |             |         |
  |    |         |                   v             |         |
  |    |         |       +---+     +-------+       |         |
  +----+---------+-----> | E | <-- |   H   | <-----+----+    |
       |         |       +---+     +-------+       |    |    |
       |         |         |         |             |    |    |
       |         |         |         |             |    |    |
       |         |         |         v             |    |    |
       |         |         |       +-------+       |    |    |
       |         |         |    +> |   K   | ------+    |    |
       |         |         |    |  +-------+            |    |
       |         |         |    |    |                  |    |
       |         |         |    |    |                  |    |
       |         |         |    |    v                  |    |
       |         |         |    |  +------------+       |    |
       |         |         |    +- |     I      | <-----+----+
       |         |         |       +------------+       |
       |         |         |         ^        ^         |
       |         +---------+---------+        |         |
       |                   |                  |         |
       |                   |                  |         |
       +-------------------+------------------+         |
                           |                            |
                           |                            |
                           +----------------------------+
*/

//     Graph g;
//     GraphBuilder b(&g);

//     auto A = b.NewBlock();
//     auto B = b.NewBlock();
//     auto C = b.NewBlock();
//     auto D = b.NewBlock();
//     auto E = b.NewBlock();
//     auto F = b.NewBlock();
//     auto G = b.NewBlock();
//     auto H = b.NewBlock();
//     auto I = b.NewBlock();
//     auto J = b.NewBlock();
//     auto K = b.NewBlock();
//     auto L = b.NewBlock();
//     auto M = b.NewBlock();

//     const auto START = Graph::BB_START_ID;
//     b.SetSuccessors(START, { M });
//     b.SetSuccessors(M, { C, B, A });
//     b.SetSuccessors(A, { D });
//     b.SetSuccessors(B, { E, A, D });
//     b.SetSuccessors(C, { F, G });
//     b.SetSuccessors(D, { L });
//     b.SetSuccessors(E, { H });
//     b.SetSuccessors(F, { I });
//     b.SetSuccessors(G, { I, J });
//     b.SetSuccessors(H, { E, K });
//     b.SetSuccessors(I, { K });
//     b.SetSuccessors(J, { I });
//     b.SetSuccessors(K, { I, M });
//     b.SetSuccessors(L, { H });

//     b.ConstructCFG();
//     b.ConstructDFG();
//     ASSERT_TRUE(b.RunChecks());

//     const auto CheckImmDoms = [&]() {
//         auto bb = g.GetBasicBlock(START);
//         EXPECT_EQ(bb->GetImmDominator(), nullptr);
//         bb = g.GetBasicBlock(M);
//         EXPECT_EQ(bb->GetImmDominator()->GetId(), START);
//         bb = g.GetBasicBlock(A);
//         EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
//         bb = g.GetBasicBlock(B);
//         EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
//         bb = g.GetBasicBlock(C);
//         EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
//         bb = g.GetBasicBlock(D);
//         EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
//         bb = g.GetBasicBlock(E);
//         EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
//         bb = g.GetBasicBlock(F);
//         EXPECT_EQ(bb->GetImmDominator()->GetId(), C);
//         bb = g.GetBasicBlock(G);
//         EXPECT_EQ(bb->GetImmDominator()->GetId(), C);
//         bb = g.GetBasicBlock(H);
//         EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
//         bb = g.GetBasicBlock(I);
//         EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
//         bb = g.GetBasicBlock(J);
//         EXPECT_EQ(bb->GetImmDominator()->GetId(), G);
//         bb = g.GetBasicBlock(K);
//         EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
//         bb = g.GetBasicBlock(L);
//         EXPECT_EQ(bb->GetImmDominator()->GetId(), D);
//     };

//     g.GetPassManager()->RunPass<DomTree>();
//     CheckImmDoms();
// }
