#include "bb.h"
#include "graph.h"
#include "graph_builder.h"

#include "gtest/gtest.h"

static inline char IdToChar(IdType id)
{
    return (id == 0) ? '0' : (char)('A' + id - 1);
}

TEST(TestRPO, Example1)
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

    auto rpo = g.GetPassManager()->GetValidPass<RPO>()->GetBlocks();

    std::vector<char> rpo_c{};

    for (auto bb : rpo) {
        rpo_c.push_back(IdToChar(bb->GetId()));
    }

    ASSERT_EQ(rpo_c, std::vector<char>({ '0', 'A', 'B', 'F', 'E', 'D', 'G', 'C' }));
}

TEST(TestRPO, Example2)
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

    auto rpo = g.GetPassManager()->GetValidPass<RPO>()->GetBlocks();

    std::vector<char> rpo_c{};

    for (auto bb : rpo) {
        rpo_c.push_back(IdToChar(bb->GetId()));
    }
    ASSERT_EQ(rpo_c,
              std::vector<char>({ '0', 'A', 'B', 'J', 'C', 'D', 'E', 'F', 'G', 'I', 'K', 'H' }));
}

TEST(TestRPO, Example3)
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

    auto rpo = g.GetPassManager()->GetValidPass<RPO>()->GetBlocks();

    std::vector<char> rpo_c{};

    for (auto bb : rpo) {
        rpo_c.push_back(IdToChar(bb->GetId()));
    }

    ASSERT_EQ(rpo_c, std::vector<char>({ '0', 'A', 'B', 'E', 'F', 'H', 'I', 'G', 'C', 'D' }));
}
