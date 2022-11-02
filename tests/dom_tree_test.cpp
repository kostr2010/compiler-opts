#include "bb.h"
#include "graph.h"
#include "graph_builder.h"
#include "gtest/gtest.h"
#include <algorithm>

static inline char IdToChar(IdType id)
{
    return (id == 0) ? '0' : (char)('A' + id - 1);
}

class DomCheck
{
  public:
    NO_DEFAULT_CTOR(DomCheck);
    NO_DEFAULT_DTOR(DomCheck);

    static void CheckDominators(BasicBlock* bb, std::vector<IdType>&& expected_dominators)
    {
        auto bb_dominators = bb->GetDominators();

        std::vector<IdType> dominators{};
        dominators.reserve(bb_dominators.size());
        for (auto b : bb_dominators) {
            dominators.push_back(b->GetId());
        }

        ASSERT_EQ(VecToSet(dominators), VecToSet(expected_dominators));

        // assert that every dominator block knows it's dominated
        for (const auto dom : bb_dominators) {
            ASSERT_TRUE(std::find_if(dom->GetDominated().begin(), dom->GetDominated().end(),
                                     [bb](BasicBlock* bb_dom) {
                                         return bb->GetId() == bb_dom->GetId();
                                     }) != dom->GetDominated().end());
        }
    }

    static void CheckDominated(BasicBlock* bb, std::vector<IdType>&& expected_dominated)
    {
        auto bb_dominated = bb->GetDominated();

        std::vector<IdType> dominated{};
        dominated.reserve(bb_dominated.size());
        for (auto b : bb_dominated) {
            dominated.push_back(b->GetId());
        }

        ASSERT_EQ(VecToSet(dominated), VecToSet(expected_dominated));

        // assert that every dominated block knows it's dominator
        for (const auto dom : bb_dominated) {
            ASSERT_TRUE(std::find_if(dom->GetDominators().begin(), dom->GetDominators().end(),
                                     [bb](BasicBlock* bb_dom) {
                                         return bb->GetId() == bb_dom->GetId();
                                     }) != dom->GetDominators().end());
        }
    }

  private:
    static inline std::set<IdType> VecToSet(const std::vector<IdType>& v)
    {
        return std::set<IdType>(v.begin(), v.end());
    }
};

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

    ASSERT_TRUE(g.GetAnalyser()->RunPass<DomTreeSlow>());

    DomCheck::CheckDominators(g.GetBasicBlock(START), { START });
    DomCheck::CheckDominated(g.GetBasicBlock(START), { START, A, B, C, D, E, F, G });

    DomCheck::CheckDominators(g.GetBasicBlock(A), { START, A });
    DomCheck::CheckDominated(g.GetBasicBlock(A), { A, B, C, D, E, F, G });

    DomCheck::CheckDominators(g.GetBasicBlock(B), { START, A, B });
    DomCheck::CheckDominated(g.GetBasicBlock(B), { B, C, D, E, F, G, E, F, G });

    DomCheck::CheckDominators(g.GetBasicBlock(C), { START, A, B, C });
    DomCheck::CheckDominated(g.GetBasicBlock(C), { C });

    DomCheck::CheckDominators(g.GetBasicBlock(D), { START, A, B, D });
    DomCheck::CheckDominated(g.GetBasicBlock(D), { D });

    DomCheck::CheckDominators(g.GetBasicBlock(E), { START, A, B, F, E });
    DomCheck::CheckDominated(g.GetBasicBlock(E), { E });

    DomCheck::CheckDominators(g.GetBasicBlock(F), { START, A, B, F });
    DomCheck::CheckDominated(g.GetBasicBlock(F), { F, E, G });

    DomCheck::CheckDominators(g.GetBasicBlock(G), { START, A, B, F, G });
    DomCheck::CheckDominated(g.GetBasicBlock(G), { G });

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

    CheckImmDoms();

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

    ASSERT_TRUE(g.GetAnalyser()->RunPass<DomTreeSlow>());

    DomCheck::CheckDominators(g.GetBasicBlock(START), { START });
    DomCheck::CheckDominated(g.GetBasicBlock(START), { START, A, B, C, D, E, F, G, H, I, J, K });

    DomCheck::CheckDominators(g.GetBasicBlock(A), { START, A });
    DomCheck::CheckDominated(g.GetBasicBlock(A), { A, B, C, D, E, F, G, H, I, J, K });

    DomCheck::CheckDominators(g.GetBasicBlock(B), { START, A, B });
    DomCheck::CheckDominated(g.GetBasicBlock(B), { B, C, D, E, F, G, H, I, J, K });

    DomCheck::CheckDominators(g.GetBasicBlock(C), { START, C, A, B });
    DomCheck::CheckDominated(g.GetBasicBlock(C), { C, D, E, F, G, H, I, K });

    DomCheck::CheckDominators(g.GetBasicBlock(D), { START, D, A, B, C });
    DomCheck::CheckDominated(g.GetBasicBlock(D), { D, E, F, G, H, I, K });

    DomCheck::CheckDominators(g.GetBasicBlock(E), { START, E, A, B, C, D });
    DomCheck::CheckDominated(g.GetBasicBlock(E), { E, F, G, H, I, K });

    DomCheck::CheckDominators(g.GetBasicBlock(F), { START, F, A, B, C, D, E });
    DomCheck::CheckDominated(g.GetBasicBlock(F), { F, G, H, I, K });

    DomCheck::CheckDominators(g.GetBasicBlock(G), { START, A, B, C, D, E, F, G });
    DomCheck::CheckDominated(g.GetBasicBlock(G), { G, I, K, H });

    DomCheck::CheckDominators(g.GetBasicBlock(H), { START, H, A, B, C, D, E, F, G });
    DomCheck::CheckDominated(g.GetBasicBlock(H), { H });

    DomCheck::CheckDominators(g.GetBasicBlock(I), { START, I, A, B, C, D, E, F, G });
    DomCheck::CheckDominated(g.GetBasicBlock(I), { I, K });

    DomCheck::CheckDominators(g.GetBasicBlock(J), { START, J, A, B });
    DomCheck::CheckDominated(g.GetBasicBlock(J), { J });

    DomCheck::CheckDominators(g.GetBasicBlock(K), { START, K, A, B, C, D, E, F, G, I, K });
    DomCheck::CheckDominated(g.GetBasicBlock(K), { K });

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

    CheckImmDoms();

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

    ASSERT_TRUE(g.GetAnalyser()->RunPass<DomTreeSlow>());

    DomCheck::CheckDominators(g.GetBasicBlock(START), { START });
    DomCheck::CheckDominated(g.GetBasicBlock(START), { START, A, B, C, D, E, F, G, H, I });

    DomCheck::CheckDominators(g.GetBasicBlock(A), { START, A });
    DomCheck::CheckDominated(g.GetBasicBlock(A), { A, B, C, D, E, F, G, H, I });

    DomCheck::CheckDominators(g.GetBasicBlock(B), { START, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(B), { B, C, D, E, F, G, H, I });

    DomCheck::CheckDominators(g.GetBasicBlock(C), { START, C, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(C), { C });

    DomCheck::CheckDominators(g.GetBasicBlock(D), { START, D, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(D), { D });

    DomCheck::CheckDominators(g.GetBasicBlock(E), { START, E, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(E), { E, F, H });

    DomCheck::CheckDominators(g.GetBasicBlock(F), { START, F, B, A, E });
    DomCheck::CheckDominated(g.GetBasicBlock(F), { F, H });

    DomCheck::CheckDominators(g.GetBasicBlock(G), { START, G, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(G), { G });

    DomCheck::CheckDominators(g.GetBasicBlock(H), { START, H, B, A, E, F });
    DomCheck::CheckDominated(g.GetBasicBlock(H), { H });

    DomCheck::CheckDominators(g.GetBasicBlock(I), { START, I, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(I), { I });

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

    CheckImmDoms();

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

    ASSERT_TRUE(g.GetAnalyser()->RunPass<DomTreeSlow>());

    DomCheck::CheckDominators(g.GetBasicBlock(START), { START });
    DomCheck::CheckDominated(g.GetBasicBlock(START), { START, A, B, C, D, E });

    DomCheck::CheckDominators(g.GetBasicBlock(A), { START, A });
    DomCheck::CheckDominated(g.GetBasicBlock(A), { A, B, C, D, E });

    DomCheck::CheckDominators(g.GetBasicBlock(B), { START, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(B), { B, C, D, E });

    DomCheck::CheckDominators(g.GetBasicBlock(C), { START, C, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(C), { C });

    DomCheck::CheckDominators(g.GetBasicBlock(D), { START, D, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(D), { D, E });

    DomCheck::CheckDominators(g.GetBasicBlock(E), { START, E, B, A, D });
    DomCheck::CheckDominated(g.GetBasicBlock(E), { E });

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

    CheckImmDoms();

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

    ASSERT_TRUE(g.GetAnalyser()->RunPass<DomTreeSlow>());

    DomCheck::CheckDominators(g.GetBasicBlock(START), { START });
    DomCheck::CheckDominated(g.GetBasicBlock(START), { START, A, B, C, D, E, F });

    DomCheck::CheckDominators(g.GetBasicBlock(A), { START, A });
    DomCheck::CheckDominated(g.GetBasicBlock(A), { A, B, C, D, E, F });

    DomCheck::CheckDominators(g.GetBasicBlock(B), { START, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(B), { B, C, D, E, F });

    DomCheck::CheckDominators(g.GetBasicBlock(C), { START, C, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(C), { C, D, E, F });

    DomCheck::CheckDominators(g.GetBasicBlock(D), { START, D, B, A, C });
    DomCheck::CheckDominated(g.GetBasicBlock(D), { D, E });

    DomCheck::CheckDominators(g.GetBasicBlock(E), { START, E, B, A, C, D });
    DomCheck::CheckDominated(g.GetBasicBlock(E), { E });

    DomCheck::CheckDominators(g.GetBasicBlock(F), { START, F, B, A, C });
    DomCheck::CheckDominated(g.GetBasicBlock(F), { F });

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

    CheckImmDoms();

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

    ASSERT_TRUE(g.GetAnalyser()->RunPass<DomTreeSlow>());

    DomCheck::CheckDominators(g.GetBasicBlock(START), { START });
    DomCheck::CheckDominated(g.GetBasicBlock(START), { START, A, B, C, D, E, F, G, H });

    DomCheck::CheckDominators(g.GetBasicBlock(A), { START, A });
    DomCheck::CheckDominated(g.GetBasicBlock(A), { A, B, C, D, E, F, G, H });

    DomCheck::CheckDominators(g.GetBasicBlock(B), { START, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(B), { B, C, D, E, F, G, H });

    DomCheck::CheckDominators(g.GetBasicBlock(C), { START, C, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(C), { C, F });

    DomCheck::CheckDominators(g.GetBasicBlock(D), { START, D, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(D), { D });

    DomCheck::CheckDominators(g.GetBasicBlock(E), { START, E, B, A });
    DomCheck::CheckDominated(g.GetBasicBlock(E), { E, G, H });

    DomCheck::CheckDominators(g.GetBasicBlock(F), { START, F, B, A, C });
    DomCheck::CheckDominated(g.GetBasicBlock(F), { F });

    DomCheck::CheckDominators(g.GetBasicBlock(G), { START, G, A, B, E });
    DomCheck::CheckDominated(g.GetBasicBlock(G), { G, H });

    DomCheck::CheckDominators(g.GetBasicBlock(H), { START, H, A, B, E, G });
    DomCheck::CheckDominated(g.GetBasicBlock(H), { H });

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

    CheckImmDoms();

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

    const auto START = Graph::BB_START_ID;
    b.SetSuccessors(START, { C, B, A });
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
    b.SetSuccessors(K, { I, START });
    b.SetSuccessors(L, { H });

    b.ConstructCFG();
    b.ConstructDFG();

    const auto CheckImmDoms = [&]() {
        auto bb = g.GetBasicBlock(START);
        ASSERT_EQ(bb->GetImmDominator(), nullptr);
        bb = g.GetBasicBlock(A);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(B);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(C);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(D);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(E);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(F);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), C);
        bb = g.GetBasicBlock(G);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), C);
        bb = g.GetBasicBlock(H);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(I);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(J);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), G);
        bb = g.GetBasicBlock(K);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), START);
        bb = g.GetBasicBlock(L);
        ASSERT_EQ(bb->GetImmDominator()->GetId(), D);
    };

    g.GetAnalyser()->RunPass<DomTree>();
    CheckImmDoms();

    g.GetAnalyser()->RunPass<DomTreeSlow>();
    CheckImmDoms();
}