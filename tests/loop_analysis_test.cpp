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
    for (const auto& pred : bb->GetPredecessors()) {
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

static void CheckUsers(Inst* inst, std::set<std::pair<IdType, int> > expected)
{
    std::vector<std::pair<IdType, int> > res = {};
    for (const auto& u : inst->GetUsers()) {
        res.push_back({ u.GetInst()->GetId(), u.GetIdx() });
    }

    auto res_set = std::set<std::pair<IdType, int> >(res.begin(), res.end());
    ASSERT_EQ(res_set, expected);
}

static void CheckInputs(Inst* inst, std::set<std::pair<IdType, IdType> > expected)
{
    std::vector<std::pair<IdType, IdType> > res = {};
    for (const auto& i : inst->GetInputs()) {
        res.push_back({ i.GetInst()->GetId(), i.GetSourceBB()->GetId() });
    }

    auto res_set = std::set<std::pair<IdType, IdType> >(res.begin(), res.end());
    ASSERT_EQ(res_set, expected);
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

    ASSERT_EQ(g.GetBasicBlock(A)->GetNumSuccessors(), 1);
    auto B_ = g.GetBasicBlock(A)->GetSuccessors().at(0);
    CheckPredecessors(B_, { A });
    CheckSuccessors(B_, { B });

    ASSERT_EQ(g.GetBasicBlock(J)->GetNumSuccessors(), 1);
    auto C_ = g.GetBasicBlock(J)->GetSuccessors().at(0);
    CheckPredecessors(C_, { J, B });
    CheckSuccessors(C_, { C });

    ASSERT_EQ(g.GetBasicBlock(D)->GetNumSuccessors(), 2);
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

    ASSERT_EQ(g.GetBasicBlock(A)->GetNumSuccessors(), 1);
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

    ASSERT_EQ(g.GetBasicBlock(A)->GetNumSuccessors(), 1);
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

    ASSERT_EQ(g.GetBasicBlock(A)->GetNumSuccessors(), 1);
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

    ASSERT_EQ(g.GetBasicBlock(START)->GetNumSuccessors(), 1);
    auto A_ = g.GetBasicBlock(START)->GetSuccessors().at(0);
    CheckPredecessors(A_, { START });
    CheckSuccessors(A_, { A });

    ASSERT_EQ(g.GetBasicBlock(A)->GetNumSuccessors(), 1);
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

    ASSERT_TRUE(start_bb->HasNoPredecessors());
    ASSERT_EQ(start_bb->GetNumSuccessors(), 1);

    ASSERT_EQ(g.GetBasicBlock(START)->GetNumSuccessors(), 1);
    auto H_ = g.GetBasicBlock(START)->GetSuccessors().at(0);
    CheckPredecessors(H_, { START });
    ASSERT_EQ(H_->GetNumSuccessors(), 1);
    auto H = H_->GetSuccessors().at(0)->GetId();
    CheckSuccessors(H_, { H });

    auto bb = g.GetBasicBlock(H);
    ASSERT_EQ(bb->GetNumPredecessors(), 2);
    ASSERT_EQ(bb->GetNumSuccessors(), 1);
    CheckPredecessors(bb, { A, H_->GetId() });

    ASSERT_EQ(g.GetBasicBlock(H)->GetNumSuccessors(), 1);
    auto G_ = g.GetBasicBlock(H)->GetSuccessors().at(0);
    CheckSuccessors(bb, { G_->GetId() });
    CheckPredecessors(G_, { H });
    ASSERT_EQ(G_->GetNumSuccessors(), 1);
    auto G = G_->GetSuccessors().at(0)->GetId();
    CheckSuccessors(G_, { G });

    bb = g.GetBasicBlock(G);
    ASSERT_EQ(bb->GetNumPredecessors(), 2);
    ASSERT_EQ(bb->GetNumSuccessors(), 1);
    CheckPredecessors(bb, { B, G_->GetId() });

    ASSERT_EQ(g.GetBasicBlock(G)->GetNumSuccessors(), 1);
    auto F_ = g.GetBasicBlock(G)->GetSuccessors().at(0);
    CheckSuccessors(bb, { F_->GetId() });
    CheckPredecessors(F_, { G });
    ASSERT_EQ(F_->GetNumSuccessors(), 1);
    auto F = F_->GetSuccessors().at(0)->GetId();
    CheckSuccessors(F_, { F });

    bb = g.GetBasicBlock(F);
    ASSERT_EQ(bb->GetNumPredecessors(), 2);
    ASSERT_EQ(bb->GetNumSuccessors(), 1);
    CheckPredecessors(bb, { C, F_->GetId() });

    ASSERT_EQ(g.GetBasicBlock(F)->GetNumSuccessors(), 1);
    auto E_ = g.GetBasicBlock(F)->GetSuccessors().at(0);
    CheckSuccessors(bb, { E_->GetId() });
    CheckPredecessors(E_, { F });
    CheckSuccessors(E_, { E });

    bb = g.GetBasicBlock(E);
    ASSERT_EQ(bb->GetNumPredecessors(), 2);
    ASSERT_EQ(bb->GetNumSuccessors(), 1);
    CheckPredecessors(bb, { D, E_->GetId() });
    CheckSuccessors(bb, { D });

    bb = g.GetBasicBlock(D);
    ASSERT_EQ(bb->GetNumPredecessors(), 1);
    ASSERT_EQ(bb->GetNumSuccessors(), 2);
    CheckPredecessors(bb, { E });
    CheckSuccessors(bb, { E, C });

    bb = g.GetBasicBlock(C);
    ASSERT_EQ(bb->GetNumPredecessors(), 1);
    ASSERT_EQ(bb->GetNumSuccessors(), 2);
    CheckPredecessors(bb, { D });
    CheckSuccessors(bb, { B, F });

    bb = g.GetBasicBlock(B);
    ASSERT_EQ(bb->GetNumPredecessors(), 1);
    ASSERT_EQ(bb->GetNumSuccessors(), 2);
    CheckPredecessors(bb, { C });
    CheckSuccessors(bb, { A, G });

    bb = g.GetBasicBlock(A);
    ASSERT_EQ(bb->GetNumPredecessors(), 1);
    ASSERT_EQ(bb->GetNumSuccessors(), 1);
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

TEST(TestLoopAnalysis, TestAddPreheaderPhi1)
{
    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1U);

    auto A = b.NewBlock();
    auto I0 = b.NewInst<Inst::Opcode::PHI>();
    auto I1 = b.NewInst<Inst::Opcode::PHI>();

    auto B = b.NewBlock();
    auto I2 = b.NewInst<Inst::Opcode::ADDI>(10);

    auto C = b.NewBlock();
    auto I3 = b.NewInst<Inst::Opcode::ADDI>(10);

    auto D = b.NewBlock();
    auto I4 = b.NewInst<Inst::Opcode::ADDI>(10);

    b.SetInputs(I0, { { C0, Graph::BB_START_ID }, { I2, B }, { I3, C }, { I4, D } });
    b.SetInputs(I1, { { C0, Graph::BB_START_ID }, { I2, B }, { I3, C } });
    b.SetInputs(I2, C0);
    b.SetInputs(I3, C0);
    b.SetInputs(I4, C0);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B });
    b.SetSuccessors(B, { A, C });
    b.SetSuccessors(C, { A, D });
    b.SetSuccessors(D, { A });

    b.ConstructCFG();
    b.ConstructDFG();

    auto loop_pass = g.GetAnalyser()->GetValidPass<LoopAnalysis>();

    auto start_bb = g.GetStartBasicBlock();

    ASSERT_TRUE(start_bb->HasNoPredecessors());
    ASSERT_EQ(start_bb->GetNumSuccessors(), 1);

    ASSERT_EQ(g.GetBasicBlock(START)->GetNumSuccessors(), 1);
    auto I = g.GetBasicBlock(START)->GetSuccessors().at(0);
    CheckPredecessors(I, { START });
    ASSERT_EQ(I->GetNumSuccessors(), 1);
    auto F = I->GetSuccessors().at(0)->GetId();
    CheckSuccessors(I, { F });

    auto bb = g.GetBasicBlock(F);
    ASSERT_EQ(bb->GetNumPredecessors(), 2);
    ASSERT_EQ(bb->GetNumSuccessors(), 1);
    CheckPredecessors(bb, { I->GetId(), D });

    ASSERT_EQ(g.GetBasicBlock(F)->GetNumSuccessors(), 1);
    auto H = g.GetBasicBlock(F)->GetSuccessors().at(0);
    CheckSuccessors(bb, { H->GetId() });
    CheckPredecessors(H, { F });
    ASSERT_EQ(H->GetNumSuccessors(), 1);
    auto E = H->GetSuccessors().at(0)->GetId();
    CheckSuccessors(H, { E });

    bb = g.GetBasicBlock(E);
    ASSERT_EQ(bb->GetNumPredecessors(), 2);
    ASSERT_EQ(bb->GetNumSuccessors(), 1);
    CheckPredecessors(bb, { C, H->GetId() });

    ASSERT_EQ(g.GetBasicBlock(E)->GetNumSuccessors(), 1);
    auto G = g.GetBasicBlock(E)->GetSuccessors().at(0);
    CheckSuccessors(bb, { G->GetId() });
    CheckPredecessors(G, { E });
    ASSERT_EQ(G->GetNumSuccessors(), 1);
    CheckSuccessors(G, { A });

    bb = g.GetBasicBlock(A);
    ASSERT_EQ(bb->GetNumPredecessors(), 2);
    ASSERT_EQ(bb->GetNumSuccessors(), 1);
    CheckPredecessors(bb, { G->GetId(), B });
    CheckSuccessors(bb, { B });

    bb = g.GetBasicBlock(B);
    ASSERT_EQ(bb->GetNumPredecessors(), 1);
    ASSERT_EQ(bb->GetNumSuccessors(), 2);
    CheckPredecessors(bb, { A });
    CheckSuccessors(bb, { A, C });

    bb = g.GetBasicBlock(C);
    ASSERT_EQ(bb->GetNumPredecessors(), 1);
    ASSERT_EQ(bb->GetNumSuccessors(), 2);
    CheckPredecessors(bb, { B });
    CheckSuccessors(bb, { E, D });

    bb = g.GetBasicBlock(D);
    ASSERT_EQ(bb->GetNumPredecessors(), 1);
    ASSERT_EQ(bb->GetNumSuccessors(), 1);
    CheckPredecessors(bb, { C });
    CheckSuccessors(bb, { F });

    auto root = loop_pass->GetRootLoop();

    ASSERT_TRUE(root->GetBackEdges().empty());
    ASSERT_FALSE(root->IsReducible());
    ASSERT_EQ(root->GetHeader(), nullptr);
    ASSERT_EQ(root->GetOuterLoop(), nullptr);
    ASSERT_TRUE(root->GetPreHeader() == nullptr);
    ASSERT_EQ(root->GetInnerLoops().size(), 1);
    CheckLoopContents(root, { START, I->GetId() });

    auto loop1 = root->GetInnerLoops().at(0);
    ASSERT_TRUE(loop1->IsReducible());
    ASSERT_EQ(loop1->GetOuterLoop(), root);
    ASSERT_EQ(loop1->GetInnerLoops().size(), 1);
    ASSERT_EQ(loop1->GetBackEdges().size(), 1);
    ASSERT_EQ(loop1->GetBackEdges().at(0)->GetId(), D);
    ASSERT_EQ(loop1->GetHeader()->GetId(), F);
    CheckLoopContents(loop1, { D, H->GetId() });

    auto loop2 = loop1->GetInnerLoops().at(0);
    ASSERT_TRUE(loop2->IsReducible());
    ASSERT_EQ(loop2->GetOuterLoop(), loop1);
    ASSERT_EQ(loop2->GetInnerLoops().size(), 1);
    ASSERT_EQ(loop2->GetBackEdges().size(), 1);
    ASSERT_EQ(loop2->GetBackEdges().at(0)->GetId(), C);
    ASSERT_EQ(loop2->GetHeader()->GetId(), E);
    CheckLoopContents(loop2, { C, G->GetId() });

    auto loop3 = loop2->GetInnerLoops().at(0);
    ASSERT_TRUE(loop3->IsReducible());
    ASSERT_EQ(loop3->GetOuterLoop(), loop2);
    ASSERT_EQ(loop3->GetInnerLoops().size(), 0);
    ASSERT_EQ(loop3->GetBackEdges().size(), 1);
    ASSERT_EQ(loop3->GetBackEdges().at(0)->GetId(), B);
    ASSERT_EQ(loop3->GetHeader()->GetId(), A);
    CheckLoopContents(loop3, { B });

    auto c0 = g.GetBasicBlock(START)->GetFirstInst();
    auto i0 = g.GetBasicBlock(A)->GetFirstPhi();
    auto i1 = i0->GetNext();
    auto i2 = g.GetBasicBlock(B)->GetFirstInst();
    auto i3 = g.GetBasicBlock(C)->GetFirstInst();
    auto i4 = g.GetBasicBlock(D)->GetFirstInst();
    auto i5 = g.GetBasicBlock(E)->GetFirstPhi();
    auto i6 = i5->GetNext();
    auto i7 = g.GetBasicBlock(F)->GetFirstPhi();

    ASSERT_EQ(c0->GetOpcode(), Inst::Opcode::CONST);
    CheckInputs(c0, {});
    CheckUsers(c0, { { i7->GetId(), -1 },
                     { i6->GetId(), -1 },
                     { i2->GetId(), 0 },
                     { i3->GetId(), 0 },
                     { i4->GetId(), 0 } });

    ASSERT_EQ(i7->GetOpcode(), Inst::Opcode::PHI);
    CheckInputs(i7, { { i4->GetId(), D }, { c0->GetId(), I->GetId() } });
    CheckUsers(i7, { { i5->GetId(), -1 } });

    ASSERT_EQ(i5->GetOpcode(), Inst::Opcode::PHI);
    CheckInputs(i5, { { i3->GetId(), C }, { i7->GetId(), H->GetId() } });
    CheckUsers(i5, { { i0->GetId(), -1 } });

    ASSERT_EQ(i6->GetOpcode(), Inst::Opcode::PHI);
    CheckInputs(i6, { { i3->GetId(), C }, { c0->GetId(), H->GetId() } });
    CheckUsers(i6, { { i1->GetId(), -1 } });

    ASSERT_EQ(i0->GetOpcode(), Inst::Opcode::PHI);
    CheckInputs(i0, { { i2->GetId(), B }, { i5->GetId(), G->GetId() } });
    CheckUsers(i0, {});

    ASSERT_EQ(i1->GetOpcode(), Inst::Opcode::PHI);
    CheckInputs(i1, { { i2->GetId(), B }, { i6->GetId(), G->GetId() } });
    CheckUsers(i1, {});

    ASSERT_EQ(i2->GetOpcode(), Inst::Opcode::ADDI);
    CheckInputs(i2, { { c0->GetId(), Graph::BB_START_ID } });
    CheckUsers(i2, { { i0->GetId(), -1 }, { i1->GetId(), -1 } });

    ASSERT_EQ(i3->GetOpcode(), Inst::Opcode::ADDI);
    CheckInputs(i3, { { c0->GetId(), Graph::BB_START_ID } });
    CheckUsers(i3, { { i5->GetId(), -1 }, { i6->GetId(), -1 } });

    ASSERT_EQ(i4->GetOpcode(), Inst::Opcode::ADDI);
    CheckInputs(i4, { { c0->GetId(), Graph::BB_START_ID } });
    CheckUsers(i4, { { i7->GetId(), -1 } });
}