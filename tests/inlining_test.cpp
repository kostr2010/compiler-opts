#include "bb.h"
#include "graph.h"
#include "graph_builder.h"
#include "utils/macros.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <set>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

static void CheckUsers(InstBase* inst, std::set<std::pair<IdType, int> > expected)
{
    std::vector<std::pair<IdType, int> > res = {};
    for (const auto& u : inst->GetUsers()) {
        res.push_back({ u.GetInst()->GetId(), u.GetIdx() });
    }

    auto res_set = std::set<std::pair<IdType, int> >(res.begin(), res.end());
    ASSERT_EQ(res_set, expected);
}

static void CheckInputs(InstBase* inst, std::set<std::pair<IdType, IdType> > expected)
{
    std::vector<std::pair<IdType, IdType> > res = {};
    for (const auto& i : inst->GetInputs()) {
        if (inst->IsPhi()) {
            res.push_back({ i.GetInst()->GetId(), i.GetSourceBB()->GetId() });
        } else {
            res.push_back({ i.GetInst()->GetId(), i.GetInst()->GetBasicBlock()->GetId() });
        }
    }

    auto res_set = std::set<std::pair<IdType, IdType> >(res.begin(), res.end());
    ASSERT_EQ(res_set, expected);
}

TEST(TestInlining, Example0)
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
        auto I0 = b.NewInst<isa::inst::Opcode::RETURN>();

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
        auto I0 = b.NewInst<isa::inst::Opcode::CALL_STATIC>(&callee);
        auto I1 = b.NewInst<isa::inst::Opcode::RETURN>();

        b.SetInputs(I0, P0);
        b.SetInputs(I1, I0);

        b.SetSuccessors(START, { A });

        b.ConstructCFG();
        b.ConstructDFG();
        ASSERT_TRUE(b.RunChecks());
    }

    // DUMP(caller);
    // DUMP(callee);

    caller.GetPassManager()->GetValidPass<Inlining>();
    caller.GetPassManager()->GetValidPass<DCE>();
    caller.GetPassManager()->GetValidPass<DBE>();

    // DUMP(caller);
    // DUMP(callee);

    ASSERT_EQ(callee.GetStartBasicBlock(), nullptr);

    auto start = caller.GetStartBasicBlock();
    ASSERT_TRUE(start->HasNoPredecessors());
    ASSERT_EQ(start->GetNumSuccessors(), 1);
    ASSERT_EQ(start->GetFirstPhi(), nullptr);
    ASSERT_EQ(start->GetLastPhi(), nullptr);
    ASSERT_NE(start->GetFirstInst(), nullptr);
    ASSERT_NE(start->GetLastInst(), nullptr);
    ASSERT_EQ(start->GetLastInst(), start->GetFirstInst());
    auto p0 = start->GetFirstInst();
    auto P0 = p0->GetId();
    ASSERT_EQ(p0->GetOpcode(), isa::inst::Opcode::PARAM);
    ASSERT_EQ(p0->GetNext(), nullptr);
    ASSERT_FALSE(p0->GetUsers().empty());
    ASSERT_EQ(p0->GetUsers().size(), 1);
    ASSERT_TRUE(p0->GetInputs().empty());

    auto a = start->GetSuccessor(0);
    ASSERT_TRUE(a->HasNoSuccessors());
    ASSERT_EQ(a->GetNumPredecessors(), 1);
    ASSERT_EQ(a->GetFirstPhi(), nullptr);
    ASSERT_EQ(a->GetLastPhi(), nullptr);
    ASSERT_NE(a->GetFirstInst(), nullptr);
    ASSERT_NE(a->GetLastInst(), nullptr);
    ASSERT_EQ(a->GetLastInst(), a->GetFirstInst());
    auto i0 = a->GetFirstInst();
    auto I0 = i0->GetId();
    ASSERT_EQ(i0->GetOpcode(), isa::inst::Opcode::RETURN);
    ASSERT_EQ(i0->GetNext(), nullptr);
    ASSERT_FALSE(i0->GetInputs().empty());
    ASSERT_EQ(i0->GetNumInputs(), 1);

    ASSERT_EQ(p0->GetUsers().front().GetInst()->GetId(), I0);
    ASSERT_EQ(i0->GetInput(0).GetInst()->GetId(), P0);
    ASSERT_TRUE(i0->GetUsers().empty());
}

TEST(TestInlining, Example1)
{
    /*
            .--A--.
            |     |
            v     v
            B     C
            |     |
            .->D<-.
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
        auto I0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

        auto B = b.NewBlock();
        auto I1 = b.NewInst<isa::inst::Opcode::MUL>();

        auto C = b.NewBlock();
        auto I2 = b.NewInst<isa::inst::Opcode::ADD>();

        auto D = b.NewBlock();
        auto I3 = b.NewInst<isa::inst::Opcode::PHI>();
        auto I4 = b.NewInst<isa::inst::Opcode::RETURN>();

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
        auto C1 = b.NewConst(3);

        auto A = b.NewBlock();
        auto I0 = b.NewInst<isa::inst::Opcode::CALL_STATIC>(&callee);
        auto I1 = b.NewInst<isa::inst::Opcode::RETURN>();

        b.SetInputs(I0, C0, P0);
        b.SetInputs(I1, I0);

        b.SetSuccessors(START, { A });

        b.ConstructCFG();
        b.ConstructDFG();
        ASSERT_TRUE(b.RunChecks());
    }

    // DUMP(caller);
    // DUMP(callee);

    caller.GetPassManager()->GetValidPass<Inlining>();
    caller.GetPassManager()->GetValidPass<DCE>();
    caller.GetPassManager()->GetValidPass<DBE>();

    // DUMP(caller);
    // DUMP(callee);

    ASSERT_EQ(callee.GetStartBasicBlock(), nullptr);

    // CFG
    auto start = caller.GetStartBasicBlock();
    ASSERT_TRUE(start->HasNoPredecessors());
    ASSERT_EQ(start->GetNumSuccessors(), 1);
    ASSERT_EQ(start->GetFirstPhi(), nullptr);
    ASSERT_EQ(start->GetLastPhi(), nullptr);
    ASSERT_NE(start->GetFirstInst(), nullptr);
    ASSERT_NE(start->GetLastInst(), nullptr);
    auto p0 = start->GetFirstInst();
    auto P0 = p0->GetId();
    ASSERT_EQ(p0->GetOpcode(), isa::inst::Opcode::PARAM);
    ASSERT_NE(p0->GetNext(), nullptr);
    auto c0 = p0->GetNext();
    auto C0 = c0->GetId();
    ASSERT_TRUE(c0->IsConst());
    ASSERT_NE(c0->GetNext(), nullptr);
    auto c1 = c0->GetNext();
    auto C1 = c1->GetId();
    ASSERT_TRUE(c1->IsConst());
    ASSERT_NE(c1->GetNext(), nullptr);
    auto c2 = c1->GetNext();
    auto C2 = c2->GetId();
    ASSERT_TRUE(c2->IsConst());
    ASSERT_EQ(c2->GetNext(), nullptr);
    ASSERT_EQ(C2, start->GetLastInst()->GetId());

    ASSERT_EQ(start->GetNumSuccessors(), 1);
    auto a = start->GetSuccessor(0);
    ASSERT_EQ(a->GetNumPredecessors(), 1);
    ASSERT_EQ(a->GetPredecessor(0)->GetId(), start->GetId());
    ASSERT_EQ(a->GetFirstPhi(), nullptr);
    ASSERT_EQ(a->GetLastPhi(), nullptr);
    ASSERT_NE(a->GetFirstInst(), nullptr);
    ASSERT_NE(a->GetLastInst(), nullptr);
    auto i0 = a->GetFirstInst();
    auto I0 = i0->GetId();
    ASSERT_EQ(i0->GetOpcode(), isa::inst::Opcode::IF);
    ASSERT_EQ(i0->GetNext(), nullptr);
    ASSERT_EQ(I0, a->GetLastInst()->GetId());
    ASSERT_EQ(a->GetNumSuccessors(), 2);

    auto b = a->GetSuccessor(0);
    ASSERT_EQ(b->GetNumPredecessors(), 1);
    ASSERT_EQ(b->GetPredecessor(0)->GetId(), a->GetId());
    ASSERT_EQ(b->GetFirstPhi(), nullptr);
    ASSERT_EQ(b->GetLastPhi(), nullptr);
    ASSERT_NE(b->GetFirstInst(), nullptr);
    ASSERT_NE(b->GetLastInst(), nullptr);
    auto i1 = b->GetFirstInst();
    auto I1 = i1->GetId();
    ASSERT_EQ(i1->GetOpcode(), isa::inst::Opcode::MUL);
    ASSERT_EQ(i1->GetNext(), nullptr);
    ASSERT_EQ(I1, b->GetLastInst()->GetId());
    ASSERT_EQ(b->GetNumSuccessors(), 1);

    auto c = a->GetSuccessor(1);
    ASSERT_EQ(c->GetNumPredecessors(), 1);
    ASSERT_EQ(c->GetPredecessor(0)->GetId(), a->GetId());
    ASSERT_EQ(c->GetFirstPhi(), nullptr);
    ASSERT_EQ(c->GetLastPhi(), nullptr);
    ASSERT_NE(c->GetFirstInst(), nullptr);
    ASSERT_NE(c->GetLastInst(), nullptr);
    auto i2 = c->GetFirstInst();
    auto I2 = i2->GetId();
    ASSERT_EQ(i2->GetOpcode(), isa::inst::Opcode::ADD);
    ASSERT_EQ(i2->GetNext(), nullptr);
    ASSERT_EQ(I2, c->GetLastInst()->GetId());
    ASSERT_EQ(c->GetNumSuccessors(), 1);

    ASSERT_EQ(c->GetSuccessor(0)->GetId(), b->GetSuccessor(0)->GetId());
    auto d = c->GetSuccessor(0);
    ASSERT_EQ(d->GetNumPredecessors(), 2);
    ASSERT_EQ(d->GetPredecessor(0)->GetId(), c->GetId());
    ASSERT_EQ(d->GetPredecessor(1)->GetId(), b->GetId());
    ASSERT_EQ(d->GetFirstInst(), nullptr);
    ASSERT_EQ(d->GetLastInst(), nullptr);
    ASSERT_NE(d->GetFirstPhi(), nullptr);
    ASSERT_NE(d->GetLastPhi(), nullptr);
    auto i3 = d->GetFirstPhi();
    auto I3 = i3->GetId();
    ASSERT_EQ(i3->GetOpcode(), isa::inst::Opcode::PHI);
    ASSERT_EQ(i3->GetNext(), nullptr);
    ASSERT_EQ(I3, d->GetLastPhi()->GetId());
    ASSERT_EQ(d->GetNumSuccessors(), 1);

    auto e = d->GetSuccessor(0);
    ASSERT_EQ(e->GetNumPredecessors(), 1);
    ASSERT_EQ(e->GetPredecessor(0)->GetId(), d->GetId());
    ASSERT_EQ(e->GetFirstPhi(), nullptr);
    ASSERT_EQ(e->GetLastPhi(), nullptr);
    ASSERT_NE(e->GetFirstInst(), nullptr);
    ASSERT_NE(e->GetLastInst(), nullptr);
    auto i4 = e->GetFirstInst();
    auto I4 = i4->GetId();
    ASSERT_EQ(i4->GetOpcode(), isa::inst::Opcode::RETURN);
    ASSERT_EQ(i4->GetNext(), nullptr);
    ASSERT_EQ(I4, e->GetLastInst()->GetId());
    ASSERT_TRUE(e->HasNoSuccessors());

    // DFG
    CheckInputs(p0, {});
    CheckUsers(p0, { { I2, 1 } });

    CheckInputs(c0, {});
    CheckUsers(c0, { { I0, 0 }, { I1, 1 } });

    CheckInputs(c1, {});
    CheckUsers(c1, { { I2, 0 } });

    CheckInputs(c2, {});
    CheckUsers(c2, { { I0, 1 }, { I1, 0 } });

    CheckInputs(i0, { { C0, start->GetId() }, { C2, start->GetId() } });
    CheckUsers(i0, {});

    CheckInputs(i1, { { C2, start->GetId() }, { C0, start->GetId() } });
    CheckUsers(i1, { { I3, -1 } });

    CheckInputs(i2, { { C1, start->GetId() }, { P0, start->GetId() } });
    CheckUsers(i2, { { I3, -1 } });

    CheckInputs(i3, { { I1, b->GetId() }, { I2, c->GetId() } });
    CheckUsers(i3, { { I4, 0 } });

    CheckInputs(i4, { { I3, d->GetId() } });
    CheckUsers(i4, {});
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
        auto I0 = b.NewInst<isa::inst::Opcode::MUL>();

        auto B = b.NewBlock();
        auto I1 = b.NewInst<isa::inst::Opcode::ADD>();
        auto I2 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

        auto C = b.NewBlock();
        auto I3 = b.NewInst<isa::inst::Opcode::RETURN_VOID>();

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
        auto I0 = b.NewInst<isa::inst::Opcode::MUL>();
        auto I1 = b.NewInst<isa::inst::Opcode::CALL_STATIC>(&callee);
        auto I2 = b.NewInst<isa::inst::Opcode::RETURN>();

        b.SetInputs(I0, C0, P0);
        b.SetInputs(I1, I0);
        b.SetInputs(I2, I0);

        b.SetSuccessors(START, { A });

        b.ConstructCFG();
        b.ConstructDFG();
        ASSERT_TRUE(b.RunChecks());
    }

    // DUMP(caller);
    // DUMP(callee);

    caller.GetPassManager()->GetValidPass<Inlining>();
    caller.GetPassManager()->GetValidPass<DCE>();
    caller.GetPassManager()->GetValidPass<DBE>();

    // DUMP(caller);
    // DUMP(callee);

    ASSERT_EQ(callee.GetStartBasicBlock(), nullptr);

    // CFG
    auto start = caller.GetStartBasicBlock();
    ASSERT_TRUE(start->HasNoPredecessors());
    ASSERT_EQ(start->GetNumSuccessors(), 1);
    ASSERT_EQ(start->GetFirstPhi(), nullptr);
    ASSERT_EQ(start->GetLastPhi(), nullptr);
    ASSERT_NE(start->GetFirstInst(), nullptr);
    ASSERT_NE(start->GetLastInst(), nullptr);
    auto p0 = start->GetFirstInst();
    auto P0 = p0->GetId();
    ASSERT_EQ(p0->GetOpcode(), isa::inst::Opcode::PARAM);
    ASSERT_NE(p0->GetNext(), nullptr);
    auto c0 = p0->GetNext();
    auto C0 = c0->GetId();
    ASSERT_TRUE(c0->IsConst());
    ASSERT_NE(c0->GetNext(), nullptr);
    auto c1 = c0->GetNext();
    auto C1 = c1->GetId();
    ASSERT_TRUE(c1->IsConst());
    ASSERT_NE(c1->GetNext(), nullptr);
    auto c2 = c1->GetNext();
    auto C2 = c2->GetId();
    ASSERT_TRUE(c2->IsConst());
    ASSERT_EQ(c2->GetNext(), nullptr);
    ASSERT_EQ(C2, start->GetLastInst()->GetId());

    ASSERT_EQ(start->GetNumSuccessors(), 1);
    auto a = start->GetSuccessor(0);
    ASSERT_EQ(a->GetNumPredecessors(), 1);
    ASSERT_EQ(a->GetPredecessor(0)->GetId(), start->GetId());
    ASSERT_EQ(a->GetFirstPhi(), nullptr);
    ASSERT_EQ(a->GetLastPhi(), nullptr);
    ASSERT_NE(a->GetFirstInst(), nullptr);
    ASSERT_NE(a->GetLastInst(), nullptr);
    auto i0 = a->GetFirstInst();
    auto I0 = i0->GetId();
    ASSERT_EQ(i0->GetOpcode(), isa::inst::Opcode::MUL);
    ASSERT_EQ(i0->GetNext(), nullptr);
    ASSERT_EQ(I0, a->GetLastInst()->GetId());
    ASSERT_EQ(a->GetNumSuccessors(), 1);

    auto b = a->GetSuccessor(0);
    ASSERT_EQ(b->GetNumPredecessors(), 2);
    ASSERT_EQ(b->GetPredecessor(1)->GetId(), a->GetId());
    ASSERT_EQ(b->GetFirstPhi(), nullptr);
    ASSERT_EQ(b->GetLastPhi(), nullptr);
    ASSERT_NE(b->GetFirstInst(), nullptr);
    ASSERT_NE(b->GetLastInst(), nullptr);
    auto i1 = b->GetFirstInst();
    auto I1 = i1->GetId();
    ASSERT_EQ(i1->GetOpcode(), isa::inst::Opcode::MUL);
    ASSERT_EQ(i1->GetNext(), nullptr);
    ASSERT_EQ(I1, b->GetLastInst()->GetId());
    ASSERT_EQ(b->GetNumSuccessors(), 1);

    auto c = b->GetSuccessor(0);
    ASSERT_EQ(b->GetPredecessor(0)->GetId(), c->GetId());
    ASSERT_EQ(c->GetNumPredecessors(), 1);
    ASSERT_EQ(c->GetPredecessor(0)->GetId(), b->GetId());
    ASSERT_EQ(c->GetFirstPhi(), nullptr);
    ASSERT_EQ(c->GetLastPhi(), nullptr);
    ASSERT_NE(c->GetFirstInst(), nullptr);
    ASSERT_NE(c->GetLastInst(), nullptr);
    auto i2 = c->GetFirstInst();
    auto I2 = i2->GetId();
    ASSERT_EQ(i2->GetOpcode(), isa::inst::Opcode::ADD);
    ASSERT_NE(i2->GetNext(), nullptr);
    auto i3 = i2->GetNext();
    auto I3 = i3->GetId();
    ASSERT_EQ(i3->GetOpcode(), isa::inst::Opcode::IF);
    ASSERT_EQ(i3->GetNext(), nullptr);
    ASSERT_EQ(I3, c->GetLastInst()->GetId());
    ASSERT_EQ(c->GetNumSuccessors(), 2);

    ASSERT_EQ(c->GetSuccessor(0)->GetId(), b->GetId());
    auto d = c->GetSuccessor(1);
    ASSERT_EQ(d->GetNumPredecessors(), 1);
    ASSERT_EQ(d->GetPredecessor(0)->GetId(), c->GetId());
    ASSERT_EQ(d->GetFirstPhi(), nullptr);
    ASSERT_EQ(d->GetLastPhi(), nullptr);
    ASSERT_NE(d->GetFirstInst(), nullptr);
    ASSERT_NE(d->GetLastInst(), nullptr);
    auto i4 = d->GetFirstInst();
    auto I4 = i4->GetId();
    ASSERT_EQ(i4->GetOpcode(), isa::inst::Opcode::RETURN);
    ASSERT_EQ(i4->GetNext(), nullptr);
    ASSERT_EQ(I4, d->GetLastInst()->GetId());
    ASSERT_TRUE(d->HasNoSuccessors());

    // DFG
    CheckInputs(p0, {});
    CheckUsers(p0, { { I0, 1 } });

    CheckInputs(c0, {});
    CheckUsers(c0, { { I0, 0 } });

    CheckInputs(c1, {});
    CheckUsers(c1, { { I1, 1 } });

    CheckInputs(c2, {});
    CheckUsers(c2, { { I2, 0 } });

    CheckInputs(i0, { { C0, start->GetId() }, { P0, start->GetId() } });
    CheckUsers(i0, { { I1, 0 }, { I2, 1 }, { I4, 0 } });

    CheckInputs(i1, { { I0, a->GetId() }, { C1, start->GetId() } });
    CheckUsers(i1, { { I3, 0 } });

    CheckInputs(i2, { { C2, start->GetId() }, { I0, a->GetId() } });
    CheckUsers(i2, { { I3, 1 } });

    CheckInputs(i3, { { I1, b->GetId() }, { I2, c->GetId() } });
    CheckUsers(i3, {});

    CheckInputs(i4, { { I0, a->GetId() } });
    CheckUsers(i4, {});
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
        auto I0 = b.NewInst<isa::inst::Opcode::MUL>();
        auto If = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

        auto B = b.NewBlock();
        auto I1 = b.NewInst<isa::inst::Opcode::ADD>();
        auto I2 = b.NewInst<isa::inst::Opcode::RETURN>();

        auto C = b.NewBlock();
        auto I3 = b.NewInst<isa::inst::Opcode::SUB>();
        auto I4 = b.NewInst<isa::inst::Opcode::RETURN>();

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
        auto I0 = b.NewInst<isa::inst::Opcode::CALL_STATIC>(&callee);
        auto I1 = b.NewInst<isa::inst::Opcode::RETURN>();

        b.SetInputs(I0, C0, P0, C1);
        b.SetInputs(I1, I0);

        b.SetSuccessors(START, { A });

        b.ConstructCFG();
        b.ConstructDFG();
        ASSERT_TRUE(b.RunChecks());
    }

    // DUMP(caller);
    // DUMP(callee);

    caller.GetPassManager()->GetValidPass<Inlining>();
    caller.GetPassManager()->GetValidPass<DCE>();
    caller.GetPassManager()->GetValidPass<DBE>();

    // DUMP(caller);
    // DUMP(callee);

    // CFG
    auto start = caller.GetStartBasicBlock();
    ASSERT_TRUE(start->HasNoPredecessors());
    ASSERT_EQ(start->GetNumSuccessors(), 1);
    ASSERT_EQ(start->GetFirstPhi(), nullptr);
    ASSERT_EQ(start->GetLastPhi(), nullptr);
    ASSERT_NE(start->GetFirstInst(), nullptr);
    ASSERT_NE(start->GetLastInst(), nullptr);
    auto p0 = start->GetFirstInst();
    auto P0 = p0->GetId();
    ASSERT_EQ(p0->GetOpcode(), isa::inst::Opcode::PARAM);
    ASSERT_NE(p0->GetNext(), nullptr);
    auto c0 = p0->GetNext();
    auto C0 = c0->GetId();
    ASSERT_TRUE(c0->IsConst());
    ASSERT_NE(c0->GetNext(), nullptr);
    auto c1 = c0->GetNext();
    auto C1 = c1->GetId();
    ASSERT_TRUE(c1->IsConst());
    ASSERT_NE(c1->GetNext(), nullptr);
    auto c2 = c1->GetNext();
    auto C2 = c2->GetId();
    ASSERT_TRUE(c2->IsConst());
    ASSERT_EQ(c2->GetNext(), nullptr);
    ASSERT_EQ(C2, start->GetLastInst()->GetId());

    ASSERT_EQ(start->GetNumSuccessors(), 1);
    auto a = start->GetSuccessor(0);
    ASSERT_EQ(a->GetNumPredecessors(), 1);
    ASSERT_EQ(a->GetPredecessor(0)->GetId(), start->GetId());
    ASSERT_EQ(a->GetFirstPhi(), nullptr);
    ASSERT_EQ(a->GetLastPhi(), nullptr);
    ASSERT_NE(a->GetFirstInst(), nullptr);
    ASSERT_NE(a->GetLastInst(), nullptr);
    auto i0 = a->GetFirstInst();
    auto I0 = i0->GetId();
    ASSERT_EQ(i0->GetOpcode(), isa::inst::Opcode::MUL);
    ASSERT_NE(i0->GetNext(), nullptr);
    auto i1 = i0->GetNext();
    auto I1 = i1->GetId();
    ASSERT_EQ(i1->GetOpcode(), isa::inst::Opcode::IF);
    ASSERT_EQ(i1->GetNext(), nullptr);
    ASSERT_EQ(I1, a->GetLastInst()->GetId());
    ASSERT_EQ(a->GetNumSuccessors(), 2);

    auto b = a->GetSuccessor(0);
    ASSERT_EQ(b->GetNumPredecessors(), 1);
    ASSERT_EQ(b->GetPredecessor(0)->GetId(), a->GetId());
    ASSERT_EQ(b->GetFirstPhi(), nullptr);
    ASSERT_EQ(b->GetLastPhi(), nullptr);
    ASSERT_NE(b->GetFirstInst(), nullptr);
    ASSERT_NE(b->GetLastInst(), nullptr);
    auto i2 = b->GetFirstInst();
    auto I2 = i2->GetId();
    ASSERT_EQ(i2->GetOpcode(), isa::inst::Opcode::ADD);
    ASSERT_EQ(i2->GetNext(), nullptr);
    ASSERT_EQ(I2, b->GetLastInst()->GetId());
    ASSERT_EQ(b->GetNumSuccessors(), 1);

    auto c = a->GetSuccessor(1);
    ASSERT_EQ(c->GetNumPredecessors(), 1);
    ASSERT_EQ(c->GetPredecessor(0)->GetId(), a->GetId());
    ASSERT_EQ(c->GetFirstPhi(), nullptr);
    ASSERT_EQ(c->GetLastPhi(), nullptr);
    ASSERT_NE(c->GetFirstInst(), nullptr);
    ASSERT_NE(c->GetLastInst(), nullptr);
    auto i3 = c->GetFirstInst();
    auto I3 = i3->GetId();
    ASSERT_EQ(i3->GetOpcode(), isa::inst::Opcode::SUB);
    ASSERT_EQ(i3->GetNext(), nullptr);
    ASSERT_EQ(I3, c->GetLastInst()->GetId());
    ASSERT_EQ(c->GetNumSuccessors(), 1);

    auto d = c->GetSuccessor(0);
    ASSERT_EQ(d->GetNumPredecessors(), 2);
    ASSERT_EQ(d->GetPredecessor(1)->GetId(), c->GetId());
    ASSERT_EQ(d->GetPredecessor(0)->GetId(), b->GetId());
    ASSERT_NE(d->GetFirstPhi(), nullptr);
    ASSERT_NE(d->GetLastPhi(), nullptr);
    auto i4 = d->GetFirstPhi();
    auto I4 = i4->GetId();
    ASSERT_EQ(i4->GetOpcode(), isa::inst::Opcode::PHI);
    ASSERT_EQ(i4->GetNext(), nullptr);
    ASSERT_EQ(I4, d->GetLastPhi()->GetId());
    ASSERT_TRUE(d->HasNoSuccessors());
    ASSERT_NE(d->GetFirstInst(), nullptr);
    ASSERT_NE(d->GetLastInst(), nullptr);
    auto i5 = d->GetFirstInst();
    auto I5 = i5->GetId();
    ASSERT_EQ(i5->GetOpcode(), isa::inst::Opcode::RETURN);
    ASSERT_EQ(i5->GetNext(), nullptr);
    ASSERT_EQ(I5, d->GetLastInst()->GetId());
    ASSERT_TRUE(d->HasNoSuccessors());

    // DFG
    CheckInputs(p0, {});
    CheckUsers(p0, { { I3, 1 } });

    CheckInputs(c0, {});
    CheckUsers(c0, { { I2, 1 } });

    CheckInputs(c1, {});
    CheckUsers(c1, { { I1, 0 }, { I0, 0 } });

    CheckInputs(c2, {});
    CheckUsers(c2, { { I0, 1 } });

    CheckInputs(i0, { { C1, start->GetId() }, { C2, start->GetId() } });
    CheckUsers(i0, { { I1, 1 }, { I2, 0 }, { I3, 0 } });

    CheckInputs(i1, { { C1, start->GetId() }, { I0, a->GetId() } });
    CheckUsers(i1, {});

    CheckInputs(i2, { { I0, a->GetId() }, { C0, start->GetId() } });
    CheckUsers(i2, { { I4, -1 } });

    CheckInputs(i3, { { I0, a->GetId() }, { P0, start->GetId() } });
    CheckUsers(i3, { { I4, -1 } });

    CheckInputs(i4, { { I2, b->GetId() }, { I3, c->GetId() } });
    CheckUsers(i4, { { I5, 0 } });

    CheckInputs(i5, { { I4, d->GetId() } });
    CheckUsers(i5, {});
}

#pragma GCC diagnostic pop
