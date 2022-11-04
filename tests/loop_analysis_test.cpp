#include "bb.h"
#include "graph.h"
#include "graph_builder.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <set>

static void CheckLoopContents(Loop* loop, std::set<IdType> expected)
{
    std::vector<IdType> bbs_ids{};
    for (const auto& bb : loop->GetBlocks()) {
        bbs_ids.push_back(bb->GetId());
        ASSERT_EQ(bb->GetLoop()->GetId(), loop->GetId());
    }
    std::set<IdType> bb_ids = std::set<IdType>(bbs_ids.begin(), bbs_ids.end());

    ASSERT_EQ(bb_ids, expected);
}

static void CheckPredecessors(BasicBlock* bb, std::set<IdType> expected)
{
    std::vector<IdType> bb_res = {};
    for (const auto& pred : bb->GetPredecesors()) {
        bb_res.push_back(pred->GetId());
    }

    ASSERT_EQ(std::set<IdType>(bb_res.begin(), bb_res.end()), expected);
}

static void CheckSuccessors(BasicBlock* bb, std::set<IdType> expected)
{
    std::vector<IdType> bb_res = {};
    for (const auto& pred : bb->GetSuccessors()) {
        bb_res.push_back(pred->GetId());
    }

    ASSERT_EQ(std::set<IdType>(bb_res.begin(), bb_res.end()), expected);
}

TEST(TestLoopAnalysis, Example1)
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

    auto loop_pass = g.GetAnalyser()->GetValidPass<LoopAnalysis>();
    auto rpo_pass = g.GetAnalyser()->GetValidPass<RPO>();
    auto root = loop_pass->GetRootLoop();

    ASSERT_EQ(root->GetBlocks().size(), rpo_pass->GetBlocks().size());
    ASSERT_TRUE(root->GetBackEdges().empty());
    ASSERT_EQ(root->GetHeader(), nullptr);
    ASSERT_EQ(root->GetOuterLoop(), nullptr);
    ASSERT_TRUE(root->GetInnerLoops().empty());
    ASSERT_TRUE(root->GetPreHeader() == nullptr);
}

TEST(TestLoopAnalysis, Example2)
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

    auto loop_pass = g.GetAnalyser()->GetValidPass<LoopAnalysis>();
    auto root = loop_pass->GetRootLoop();

    ASSERT_TRUE(root->GetBackEdges().empty());
    ASSERT_EQ(root->GetHeader(), nullptr);
    ASSERT_EQ(root->GetOuterLoop(), nullptr);
    ASSERT_TRUE(root->GetPreHeader() == nullptr);

    ASSERT_EQ(g.GetBasicBlock(A)->GetSuccessors().size(), 1);
    auto B_ = g.GetBasicBlock(A)->GetSuccessors().at(0);
    CheckPredecessors(B_, { A });
    CheckSuccessors(B_, { B });

    ASSERT_EQ(g.GetBasicBlock(J)->GetSuccessors().size(), 1);
    auto C_ = g.GetBasicBlock(J)->GetSuccessors().at(0);
    CheckPredecessors(C_, { J, B });
    CheckSuccessors(C_, { C });

    ASSERT_EQ(g.GetBasicBlock(D)->GetSuccessors().size(), 2);
    auto E_ = g.GetBasicBlock(D)->GetSuccessors().at(0);
    ASSERT_NE(E_->GetId(), C);
    CheckPredecessors(E_, { D });
    CheckSuccessors(E_, { E });

    ASSERT_EQ(root->GetInnerLoops().size(), 1);
    CheckLoopContents(root, { START, A, I, K, B_->GetId() });

    auto loop = root->GetInnerLoops().at(0);
    ASSERT_TRUE(loop->IsReducible());
    ASSERT_EQ(loop->GetOuterLoop(), root);
    ASSERT_EQ(loop->GetHeader()->GetId(), g.GetBasicBlock(B)->GetId());
    ASSERT_EQ(loop->GetPreHeader()->GetId(), B_->GetId());
    ASSERT_EQ(loop->GetInnerLoops().size(), 2);
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges().at(0), g.GetBasicBlock(H));
    CheckLoopContents(loop, { J, G, H, C_->GetId(), E_->GetId() });

    loop = root->GetInnerLoops().at(0)->GetInnerLoops().at(0);
    ASSERT_TRUE(loop->IsReducible());
    ASSERT_EQ(loop->GetOuterLoop(), root->GetInnerLoops().at(0));
    ASSERT_EQ(loop->GetHeader()->GetId(), g.GetBasicBlock(E)->GetId());
    ASSERT_EQ(loop->GetPreHeader()->GetId(), E_->GetId());
    ASSERT_EQ(loop->GetInnerLoops().size(), 0);
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges().at(0), g.GetBasicBlock(F));
    CheckLoopContents(loop, { F });

    loop = root->GetInnerLoops().at(0)->GetInnerLoops().at(1);
    ASSERT_TRUE(loop->IsReducible());
    ASSERT_EQ(loop->GetOuterLoop(), root->GetInnerLoops().at(0));
    ASSERT_EQ(loop->GetHeader()->GetId(), g.GetBasicBlock(C)->GetId());
    ASSERT_EQ(loop->GetPreHeader()->GetId(), C_->GetId());
    ASSERT_EQ(loop->GetInnerLoops().size(), 0);
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges().at(0), g.GetBasicBlock(D));
    CheckLoopContents(loop, { D });
}

TEST(TestLoopAnalysis, Example3)
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

    auto loop_pass = g.GetAnalyser()->GetValidPass<LoopAnalysis>();
    auto root = loop_pass->GetRootLoop();

    ASSERT_TRUE(root->GetBackEdges().empty());
    ASSERT_EQ(root->GetHeader(), nullptr);
    ASSERT_EQ(root->GetOuterLoop(), nullptr);
    ASSERT_TRUE(root->GetPreHeader() == nullptr);

    // FIXME: is it correct?

    ASSERT_EQ(g.GetBasicBlock(A)->GetSuccessors().size(), 1);
    auto B_ = g.GetBasicBlock(A)->GetSuccessors().at(0);
    CheckPredecessors(B_, { A });
    CheckSuccessors(B_, { B });

    ASSERT_EQ(root->GetInnerLoops().size(), 2);
    CheckLoopContents(root, { START, A, H, I, C, B_->GetId() });

    auto loop = root->GetInnerLoops().at(0);
    ASSERT_FALSE(loop->IsReducible());
    ASSERT_EQ(loop->GetOuterLoop(), root);
    ASSERT_EQ(loop->GetHeader(), g.GetBasicBlock(G));
    ASSERT_EQ(loop->GetPreHeader(), nullptr);
    ASSERT_EQ(loop->GetInnerLoops().size(), 0);
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges().at(0), g.GetBasicBlock(D));
    CheckLoopContents(loop, { D });

    loop = root->GetInnerLoops().at(1);
    ASSERT_TRUE(loop->IsReducible());
    ASSERT_EQ(loop->GetOuterLoop(), root);
    ASSERT_EQ(loop->GetHeader(), g.GetBasicBlock(B));
    ASSERT_EQ(loop->GetPreHeader(), B_);
    ASSERT_EQ(loop->GetInnerLoops().size(), 0);
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges().at(0), g.GetBasicBlock(F));
    CheckLoopContents(loop, { E, F });
}

TEST(TestLoopAnalysis, Example4)
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

    auto loop_pass = g.GetAnalyser()->GetValidPass<LoopAnalysis>();
    auto root = loop_pass->GetRootLoop();

    ASSERT_TRUE(root->GetBackEdges().empty());
    ASSERT_EQ(root->GetHeader(), nullptr);
    ASSERT_EQ(root->GetOuterLoop(), nullptr);
    ASSERT_TRUE(root->GetPreHeader() == nullptr);

    ASSERT_EQ(g.GetBasicBlock(A)->GetSuccessors().size(), 1);
    auto B_ = g.GetBasicBlock(A)->GetSuccessors().at(0);
    CheckPredecessors(B_, { A });
    CheckSuccessors(B_, { B });

    ASSERT_EQ(root->GetInnerLoops().size(), 1);
    CheckLoopContents(root, { START, A, C, B_->GetId() });

    auto loop = root->GetInnerLoops().at(0);
    ASSERT_TRUE(loop->IsReducible());
    ASSERT_EQ(loop->GetOuterLoop(), root);
    ASSERT_EQ(loop->GetHeader(), g.GetBasicBlock(B));
    ASSERT_EQ(loop->GetPreHeader(), B_);
    ASSERT_EQ(loop->GetInnerLoops().size(), 0);
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges().at(0), g.GetBasicBlock(E));
    CheckLoopContents(loop, { D, E });
}

TEST(TestLoopAnalysis, Example5)
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

    auto loop_pass = g.GetAnalyser()->GetValidPass<LoopAnalysis>();
    auto root = loop_pass->GetRootLoop();

    ASSERT_TRUE(root->GetBackEdges().empty());
    ASSERT_EQ(root->GetHeader(), nullptr);
    ASSERT_EQ(root->GetOuterLoop(), nullptr);
    ASSERT_TRUE(root->GetPreHeader() == nullptr);

    ASSERT_EQ(g.GetBasicBlock(A)->GetSuccessors().size(), 1);
    auto B_ = g.GetBasicBlock(A)->GetSuccessors().at(0);
    CheckPredecessors(B_, { A });
    CheckSuccessors(B_, { B });

    ASSERT_EQ(root->GetInnerLoops().size(), 1);
    CheckLoopContents(root, { START, A, F, B_->GetId() });

    auto loop = root->GetInnerLoops().at(0);
    ASSERT_TRUE(loop->IsReducible());
    ASSERT_EQ(loop->GetOuterLoop(), root);
    ASSERT_EQ(loop->GetHeader(), g.GetBasicBlock(B));
    ASSERT_EQ(loop->GetPreHeader(), B_);
    ASSERT_EQ(loop->GetInnerLoops().size(), 0);
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges().at(0), g.GetBasicBlock(E));
    CheckLoopContents(loop, { C, D, E });
}

TEST(TestLoopAnalysis, Example6)
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

    auto loop_pass = g.GetAnalyser()->GetValidPass<LoopAnalysis>();
    auto root = loop_pass->GetRootLoop();

    ASSERT_TRUE(root->GetBackEdges().empty());
    ASSERT_EQ(root->GetHeader(), nullptr);
    ASSERT_EQ(root->GetOuterLoop(), nullptr);
    ASSERT_TRUE(root->GetPreHeader() == nullptr);

    ASSERT_EQ(g.GetBasicBlock(START)->GetSuccessors().size(), 1);
    auto A_ = g.GetBasicBlock(START)->GetSuccessors().at(0);
    CheckPredecessors(A_, { START });
    CheckSuccessors(A_, { A });

    ASSERT_EQ(g.GetBasicBlock(A)->GetSuccessors().size(), 1);
    auto B_ = g.GetBasicBlock(A)->GetSuccessors().at(0);
    CheckPredecessors(B_, { A });
    CheckSuccessors(B_, { B });

    ASSERT_EQ(root->GetInnerLoops().size(), 1);
    CheckLoopContents(root, { START, F, A_->GetId() });

    auto loop = root->GetInnerLoops().at(0);
    ASSERT_TRUE(loop->IsReducible());
    ASSERT_EQ(loop->GetOuterLoop(), root);
    ASSERT_EQ(loop->GetHeader(), g.GetBasicBlock(A));
    ASSERT_EQ(loop->GetPreHeader(), A_);
    ASSERT_EQ(loop->GetInnerLoops().size(), 1);
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges().at(0), g.GetBasicBlock(H));
    CheckLoopContents(loop, { H, B_->GetId() });

    loop = loop->GetInnerLoops().at(0);
    ASSERT_TRUE(loop->IsReducible());
    ASSERT_EQ(loop->GetOuterLoop(), root->GetInnerLoops().at(0));
    ASSERT_EQ(loop->GetHeader(), g.GetBasicBlock(B));
    ASSERT_EQ(loop->GetPreHeader(), B_);
    ASSERT_EQ(loop->GetInnerLoops().size(), 0);
    ASSERT_EQ(loop->GetBackEdges().size(), 1);
    ASSERT_EQ(loop->GetBackEdges().at(0), g.GetBasicBlock(G));
    CheckLoopContents(loop, { C, D, E, G });
}

TEST(TestLoopAnalysis, SeparateBackedges1)
{
    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto A = b.NewBlock();
    auto B = b.NewBlock();
    auto C = b.NewBlock();
    auto D = b.NewBlock();
    auto E = b.NewBlock();

    b.SetSuccessors(START, { E });
    b.SetSuccessors(A, { E });
    b.SetSuccessors(B, { A, E });
    b.SetSuccessors(C, { E, B });
    b.SetSuccessors(D, { C, E });
    b.SetSuccessors(E, { D });

    b.ConstructCFG();
    b.ConstructDFG();

    auto loop_pass = g.GetAnalyser()->GetValidPass<LoopAnalysis>();

    auto start_bb = g.GetStartBasicBlock();

    ASSERT_TRUE(start_bb->GetPredecesors().empty());
    ASSERT_EQ(start_bb->GetSuccessors().size(), 1);

    ASSERT_EQ(g.GetBasicBlock(START)->GetSuccessors().size(), 1);
    auto H_ = g.GetBasicBlock(START)->GetSuccessors().at(0);
    CheckPredecessors(H_, { START });
    ASSERT_EQ(H_->GetSuccessors().size(), 1);
    auto H = H_->GetSuccessors().at(0)->GetId();
    CheckSuccessors(H_, { H });

    auto bb = g.GetBasicBlock(H);
    ASSERT_EQ(bb->GetPredecesors().size(), 2);
    ASSERT_EQ(bb->GetSuccessors().size(), 1);
    CheckPredecessors(bb, { A, H_->GetId() });

    ASSERT_EQ(g.GetBasicBlock(H)->GetSuccessors().size(), 1);
    auto G_ = g.GetBasicBlock(H)->GetSuccessors().at(0);
    CheckSuccessors(bb, { G_->GetId() });
    CheckPredecessors(G_, { H });
    ASSERT_EQ(G_->GetSuccessors().size(), 1);
    auto G = G_->GetSuccessors().at(0)->GetId();
    CheckSuccessors(G_, { G });

    bb = g.GetBasicBlock(G);
    ASSERT_EQ(bb->GetPredecesors().size(), 2);
    ASSERT_EQ(bb->GetSuccessors().size(), 1);
    CheckPredecessors(bb, { B, G_->GetId() });

    ASSERT_EQ(g.GetBasicBlock(G)->GetSuccessors().size(), 1);
    auto F_ = g.GetBasicBlock(G)->GetSuccessors().at(0);
    CheckSuccessors(bb, { F_->GetId() });
    CheckPredecessors(F_, { G });
    ASSERT_EQ(F_->GetSuccessors().size(), 1);
    auto F = F_->GetSuccessors().at(0)->GetId();
    CheckSuccessors(F_, { F });

    bb = g.GetBasicBlock(F);
    ASSERT_EQ(bb->GetPredecesors().size(), 2);
    ASSERT_EQ(bb->GetSuccessors().size(), 1);
    CheckPredecessors(bb, { C, F_->GetId() });

    ASSERT_EQ(g.GetBasicBlock(F)->GetSuccessors().size(), 1);
    auto E_ = g.GetBasicBlock(F)->GetSuccessors().at(0);
    CheckSuccessors(bb, { E_->GetId() });
    CheckPredecessors(E_, { F });
    CheckSuccessors(E_, { E });

    bb = g.GetBasicBlock(E);
    ASSERT_EQ(bb->GetPredecesors().size(), 2);
    ASSERT_EQ(bb->GetSuccessors().size(), 1);
    CheckPredecessors(bb, { D, E_->GetId() });
    CheckSuccessors(bb, { D });

    bb = g.GetBasicBlock(D);
    ASSERT_EQ(bb->GetPredecesors().size(), 1);
    ASSERT_EQ(bb->GetSuccessors().size(), 2);
    CheckPredecessors(bb, { E });
    CheckSuccessors(bb, { E, C });

    bb = g.GetBasicBlock(C);
    ASSERT_EQ(bb->GetPredecesors().size(), 1);
    ASSERT_EQ(bb->GetSuccessors().size(), 2);
    CheckPredecessors(bb, { D });
    CheckSuccessors(bb, { B, F });

    bb = g.GetBasicBlock(B);
    ASSERT_EQ(bb->GetPredecesors().size(), 1);
    ASSERT_EQ(bb->GetSuccessors().size(), 2);
    CheckPredecessors(bb, { C });
    CheckSuccessors(bb, { A, G });

    bb = g.GetBasicBlock(A);
    ASSERT_EQ(bb->GetPredecesors().size(), 1);
    ASSERT_EQ(bb->GetSuccessors().size(), 1);
    CheckPredecessors(bb, { B });
    CheckSuccessors(bb, { H });

    auto root = loop_pass->GetRootLoop();

    ASSERT_TRUE(root->GetBackEdges().empty());
    ASSERT_FALSE(root->IsReducible());
    ASSERT_EQ(root->GetHeader(), nullptr);
    ASSERT_EQ(root->GetOuterLoop(), nullptr);
    ASSERT_TRUE(root->GetPreHeader() == nullptr);
    ASSERT_EQ(root->GetInnerLoops().size(), 1);
    CheckLoopContents(root, { START, H_->GetId() });

    auto loop1 = root->GetInnerLoops().at(0);
    ASSERT_TRUE(loop1->IsReducible());
    ASSERT_EQ(loop1->GetOuterLoop(), root);
    ASSERT_EQ(loop1->GetInnerLoops().size(), 1);
    ASSERT_EQ(loop1->GetBackEdges().size(), 1);
    ASSERT_EQ(loop1->GetBackEdges().at(0)->GetId(), A);
    ASSERT_EQ(loop1->GetHeader()->GetId(), H);
    CheckLoopContents(loop1, { A, G_->GetId() });

    auto loop2 = loop1->GetInnerLoops().at(0);
    ASSERT_TRUE(loop2->IsReducible());
    ASSERT_EQ(loop2->GetOuterLoop(), loop1);
    ASSERT_EQ(loop2->GetInnerLoops().size(), 1);
    ASSERT_EQ(loop2->GetBackEdges().size(), 1);
    ASSERT_EQ(loop2->GetBackEdges().at(0)->GetId(), B);
    ASSERT_EQ(loop2->GetHeader()->GetId(), G);
    CheckLoopContents(loop2, { B, F_->GetId() });

    auto loop3 = loop2->GetInnerLoops().at(0);
    ASSERT_TRUE(loop3->IsReducible());
    ASSERT_EQ(loop3->GetOuterLoop(), loop2);
    ASSERT_EQ(loop3->GetInnerLoops().size(), 1);
    ASSERT_EQ(loop3->GetBackEdges().size(), 1);
    ASSERT_EQ(loop3->GetBackEdges().at(0)->GetId(), C);
    ASSERT_EQ(loop3->GetHeader()->GetId(), F);
    CheckLoopContents(loop3, { C, E_->GetId() });

    auto loop4 = loop3->GetInnerLoops().at(0);
    ASSERT_TRUE(loop4->IsReducible());
    ASSERT_EQ(loop4->GetOuterLoop(), loop3);
    ASSERT_EQ(loop4->GetInnerLoops().size(), 0);
    ASSERT_EQ(loop4->GetBackEdges().size(), 1);
    ASSERT_EQ(loop4->GetBackEdges().at(0)->GetId(), D);
    ASSERT_EQ(loop4->GetHeader()->GetId(), E);
    CheckLoopContents(loop4, { D });
}