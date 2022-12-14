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
    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    auto E = b.NewBlock();
    auto F = b.NewBlock();
    auto G = b.NewBlock();

    b.SetSuccessors(START, { A });
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

    std::vector<char> rpo_c{};

    for (auto bb : g.GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
        rpo_c.push_back(IdToChar(bb->GetId()));
    }

    ASSERT_EQ(rpo_c, std::vector<char>({ '0', 'A', 'B', 'C', 'D', 'F', 'E', 'G' }));

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

    g.GetAnalyser()->RunPass<DomTree>();
    CheckImmDoms();
}

TEST(TestDomTree, Example2)
{
    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
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

    b.SetSuccessors(START, { A });
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

    std::vector<char> rpo_c{};

    for (auto bb : g.GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
        rpo_c.push_back(IdToChar(bb->GetId()));
    }
    ASSERT_EQ(rpo_c,
              std::vector<char>({ '0', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'K', 'J' }));

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

    g.GetAnalyser()->RunPass<DomTree>();
    CheckImmDoms();
}

TEST(TestDomTree, Example3)
{
    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    auto E = b.NewBlock();
    auto F = b.NewBlock();
    auto G = b.NewBlock();
    auto H = b.NewBlock();
    auto I = b.NewBlock();

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

    for (auto bb : g.GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
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

    g.GetAnalyser()->RunPass<DomTree>();
    CheckImmDoms();
}

TEST(TestDomTree, Example4)
{
    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    auto E = b.NewBlock();

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { C, D });
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

    g.GetAnalyser()->RunPass<DomTree>();
    CheckImmDoms();
}

TEST(TestDomTree, Example5)
{
    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    auto E = b.NewBlock();
    auto F = b.NewBlock();

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { C });
    b.SetSuccessors(C, { D, F });
    b.SetSuccessors(D, { F, E });
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

    g.GetAnalyser()->RunPass<DomTree>();
    CheckImmDoms();
}

TEST(TestDomTree, Example6)
{
    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    auto E = b.NewBlock();
    auto F = b.NewBlock();
    auto G = b.NewBlock();
    auto H = b.NewBlock();

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { C, D });
    b.SetSuccessors(C, { F, E });
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

    g.GetAnalyser()->RunPass<DomTree>();
    CheckImmDoms();
}

TEST(TestDomTree, ExampleArticle)
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
    auto L = b.NewBlock();
    auto M = b.NewBlock();

    const auto START = Graph::BB_START_ID;
    b.SetSuccessors(START, { M });
    b.SetSuccessors(M, { C, B, A });
    b.SetSuccessors(A, { D });
    b.SetSuccessors(B, { E, A, D });
    b.SetSuccessors(C, { F, G });
    b.SetSuccessors(D, { L });
    b.SetSuccessors(E, { H });
    b.SetSuccessors(F, { I });
    b.SetSuccessors(G, { I, J });
    b.SetSuccessors(H, { E, K });
    b.SetSuccessors(I, { K });
    b.SetSuccessors(J, { I });
    b.SetSuccessors(K, { I, M });
    b.SetSuccessors(L, { H });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    const auto CheckImmDoms = [&]() {
        auto bb = g.GetBasicBlock(START);
        EXPECT_EQ(bb->GetImmDominator(), nullptr);
        bb = g.GetBasicBlock(M);
        EXPECT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(A);
        EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
        bb = g.GetBasicBlock(B);
        EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
        bb = g.GetBasicBlock(C);
        EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
        bb = g.GetBasicBlock(D);
        EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
        bb = g.GetBasicBlock(E);
        EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
        bb = g.GetBasicBlock(F);
        EXPECT_EQ(bb->GetImmDominator()->GetId(), C);
        bb = g.GetBasicBlock(G);
        EXPECT_EQ(bb->GetImmDominator()->GetId(), C);
        bb = g.GetBasicBlock(H);
        EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
        bb = g.GetBasicBlock(I);
        EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
        bb = g.GetBasicBlock(J);
        EXPECT_EQ(bb->GetImmDominator()->GetId(), G);
        bb = g.GetBasicBlock(K);
        EXPECT_EQ(bb->GetImmDominator()->GetId(), M);
        bb = g.GetBasicBlock(L);
        EXPECT_EQ(bb->GetImmDominator()->GetId(), D);
    };

    g.GetAnalyser()->RunPass<DomTree>();
    CheckImmDoms();
}
