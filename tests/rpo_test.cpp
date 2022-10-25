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
    Graph g;
    GraphBuilder b(&g);

    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    auto E = b.NewBlock();
    auto F = b.NewBlock();
    auto G = b.NewBlock();

    b.SetSuccessors(g.GetStartBasicBlockId(), { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { C, F });
    b.SetSuccessors(C, { D });
    b.SetSuccessors(D, {});
    b.SetSuccessors(E, { D });
    b.SetSuccessors(F, { E, G });
    b.SetSuccessors(G, { D });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    ASSERT_TRUE(g.RunPass<RPO>());
    auto rpo = g.GetPass<RPO>()->GetBlocks();

    std::vector<char> rpo_c{};

    for (auto bb : rpo) {
        rpo_c.push_back(IdToChar(bb->GetId()));
    }

    ASSERT_EQ(rpo_c, std::vector<char>({ '0', 'A', 'B', 'C', 'D', 'F', 'E', 'G' }));
}

TEST(TestRPO, Example2)
{
    Graph g;
    GraphBuilder b(&g);

    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    auto E = b.NewBlock();
    auto F = b.NewBlock();
    auto G = b.NewBlock();
    auto H = b.NewBlock();
    auto I = b.NewBlock();
    auto J = b.NewBlock();
    auto K = b.NewBlock();

    b.SetSuccessors(g.GetStartBasicBlockId(), { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { C, J });
    b.SetSuccessors(C, { D });
    b.SetSuccessors(D, { E, C });
    b.SetSuccessors(E, { F });
    b.SetSuccessors(F, { E, G });
    b.SetSuccessors(G, { H, I });
    b.SetSuccessors(H, { B });
    b.SetSuccessors(I, { K });
    b.SetSuccessors(J, { C });
    b.SetSuccessors(K, {});

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    ASSERT_TRUE(g.RunPass<RPO>());
    auto rpo = g.GetPass<RPO>()->GetBlocks();

    std::vector<char> rpo_c{};

    for (auto bb : rpo) {
        rpo_c.push_back(IdToChar(bb->GetId()));
    }
    ASSERT_EQ(rpo_c,
              std::vector<char>({ '0', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'K', 'J' }));
}

TEST(TestRPO, Example3)
{
    Graph g;
    GraphBuilder b(&g);

    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    auto E = b.NewBlock();
    auto F = b.NewBlock();
    auto G = b.NewBlock();
    auto H = b.NewBlock();
    auto I = b.NewBlock();

    b.SetSuccessors(g.GetStartBasicBlockId(), { A });
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

    ASSERT_TRUE(g.RunPass<RPO>());
    auto rpo = g.GetPass<RPO>()->GetBlocks();

    std::vector<char> rpo_c{};

    for (auto bb : rpo) {
        rpo_c.push_back(IdToChar(bb->GetId()));
    }

    ASSERT_EQ(rpo_c, std::vector<char>({ '0', 'A', 'B', 'E', 'F', 'H', 'I', 'G', 'C', 'D' }));
}
