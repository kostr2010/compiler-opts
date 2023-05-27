#include "bb.h"
#include "graph.h"
#include "graph_builder.h"

#include "gtest/gtest.h"

template <>
struct arch::ArchInfo<arch::Arch::UNSET>
{
    static constexpr unsigned NUM_REGISTERS = 3;
};

#define CHECK_REGALLOC(ID, LOC, SLOT)                                                             \
    do {                                                                                          \
        auto ranges = pass->GetLiveRanges();                                                      \
        for (unsigned i = 0; i < ranges.size(); ++i) {                                            \
            if (ranges[i].inst->GetId() == ID) {                                                  \
                ASSERT_EQ(ranges[i].inst->GetLocation(), Location(Location::Where::LOC, SLOT));   \
                break;                                                                            \
            }                                                                                     \
            if (i + 1 == ranges.size()) {                                                         \
                UNREACHABLE("no such id in live ranges!");                                        \
            }                                                                                     \
        }                                                                                         \
    } while (false);

TEST(RegallocTests, LinearScanTest0)
{
    /*
          +-------+
          | START |
          +-------+
            |
            |
            v
+---+     +-------+
| C | <-- |   A   | <+
+---+     +-------+  |
            |        |
            |        |
            v        |
          +-------+  |
          |   B   | -+
          +-------+
*/

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(10);
    auto C2 = b.NewConst(20);

    auto A = b.NewBlock();
    auto PHI0 = b.NewInst<isa::inst::Opcode::PHI>();
    auto PHI1 = b.NewInst<isa::inst::Opcode::PHI>();
    auto CMP = b.NewInst<isa::inst::Opcode::SUB>();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

    auto B = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::MUL>();
    auto I1 = b.NewInst<isa::inst::Opcode::SUB>();

    auto C = b.NewBlock();
    auto I2 = b.NewInst<isa::inst::Opcode::ADD>();
    auto RET = b.NewInst<isa::inst::Opcode::RETURN>();

    b.SetInputs(PHI0, { { C0, Graph::BB_START_ID }, { I0, B } });
    b.SetInputs(PHI1, { { C1, Graph::BB_START_ID }, { I1, B } });

    b.SetInputs(CMP, PHI1, C0);
    b.SetInputs(IF0, CMP, C0);

    b.SetInputs(I0, PHI0, PHI1);
    b.SetInputs(I1, PHI1, C0);
    b.SetInputs(I2, C2, PHI0);
    b.SetInputs(RET, I2);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { B, C });
    b.SetSuccessors(B, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    auto pass = g.GetPassManager()->GetPass<LinearScan>();
    pass->SetArch<arch::Arch::UNSET>();
    pass->Run();

    CHECK_REGALLOC(C0, REGISTER, 0);
    CHECK_REGALLOC(C1, REGISTER, 1);
    CHECK_REGALLOC(C2, STACK, 1);

    CHECK_REGALLOC(PHI0, STACK, 0);
    CHECK_REGALLOC(PHI1, REGISTER, 1);

    CHECK_REGALLOC(CMP, REGISTER, 2);

    CHECK_REGALLOC(I0, REGISTER, 2);
    CHECK_REGALLOC(I1, REGISTER, 1);
    CHECK_REGALLOC(I2, REGISTER, 0);
}

TEST(RegallocTests, LinearScanTest1)
{
    /*
              +-------+
              | START |
              +-------+
                |
                |
                v
    +---+     +-------+
    | C | <-- |   A   | <+
    +---+     +-------+  |
                |        |
                |        |
                v        |
              +-------+  |
              |   B   | -+
              +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(10);
    auto C2 = b.NewConst(20);

    auto A = b.NewBlock();
    auto PHI0 = b.NewInst<isa::inst::Opcode::PHI>();
    auto PHI1 = b.NewInst<isa::inst::Opcode::PHI>();
    auto CMP = b.NewInst<isa::inst::Opcode::SUB>();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

    auto B = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::MUL>();
    auto I1 = b.NewInst<isa::inst::Opcode::SUB>();

    auto C = b.NewBlock();
    auto I2 = b.NewInst<isa::inst::Opcode::ADD>();
    auto RET = b.NewInst<isa::inst::Opcode::RETURN>();

    b.SetInputs(PHI0, { { C0, Graph::BB_START_ID }, { I0, B } });
    b.SetInputs(PHI1, { { C1, Graph::BB_START_ID }, { I1, B } });

    b.SetInputs(CMP, PHI1, C0);
    b.SetInputs(IF0, CMP, C0);

    b.SetInputs(I0, PHI0, PHI1);
    b.SetInputs(I1, PHI1, C0);
    b.SetInputs(I2, C2, PHI0);
    b.SetInputs(RET, I2);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { C, B });
    b.SetSuccessors(B, { A });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    auto pass = g.GetPassManager()->GetPass<LinearScan>();
    pass->SetArch<arch::Arch::UNSET>();
    pass->Run();

    CHECK_REGALLOC(C0, REGISTER, 0);
    CHECK_REGALLOC(C1, REGISTER, 1);
    CHECK_REGALLOC(C2, STACK, 1);

    CHECK_REGALLOC(PHI0, STACK, 0);
    CHECK_REGALLOC(PHI1, REGISTER, 1);

    CHECK_REGALLOC(CMP, REGISTER, 2);

    CHECK_REGALLOC(I0, REGISTER, 2);
    CHECK_REGALLOC(I1, REGISTER, 1);
    CHECK_REGALLOC(I2, REGISTER, 0);
}

TEST(RegallocTests, LinearScanTest2)
{
    /*
              +-------+
              | START |
              +-------+
                |
                |
                v
    +---+     +-------+
    | B | <-- |   A   |
    +---+     +-------+
                |
                |
                v
              +-------+
              |   C   |
              +-------+
                |
                |
                v
              +-------+
              |   D   |
              +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(10);
    auto C2 = b.NewConst(20);

    auto A = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

    auto B = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::MUL>();
    auto I1 = b.NewInst<isa::inst::Opcode::SUB>();
    auto RET0 = b.NewInst<isa::inst::Opcode::RETURN>();

    auto C = b.NewBlock();
    auto I2 = b.NewInst<isa::inst::Opcode::ADD>();

    auto D = b.NewBlock();
    auto RET1 = b.NewInst<isa::inst::Opcode::RETURN>();

    b.SetInputs(IF0, C0, C1);

    b.SetInputs(I0, C0, C1);
    b.SetInputs(I1, C1, I0);
    b.SetInputs(I2, C2, C0);

    b.SetInputs(RET0, I1);
    b.SetInputs(RET1, I2);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { C, B });
    b.SetSuccessors(C, { D });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    auto pass = g.GetPassManager()->GetPass<LinearScan>();
    pass->SetArch<arch::Arch::UNSET>();
    pass->Run();

    CHECK_REGALLOC(C0, REGISTER, 0);
    CHECK_REGALLOC(C1, REGISTER, 1);
    CHECK_REGALLOC(C2, REGISTER, 2);

    CHECK_REGALLOC(I0, REGISTER, 0);
    CHECK_REGALLOC(I1, REGISTER, 0);
    CHECK_REGALLOC(I2, REGISTER, 2);
}

TEST(RegallocTests, LinearScanTest3)
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
                |
                |
                v
              +-------+
              |   E   |
              +-------+
    */

    Graph g;
    GraphBuilder b(&g);

    auto START = Graph::BB_START_ID;
    auto C0 = b.NewConst(1);
    auto C1 = b.NewConst(10);
    auto C2 = b.NewConst(20);

    auto A = b.NewBlock();
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

    auto B = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::MUL>();
    auto I1 = b.NewInst<isa::inst::Opcode::SUB>();

    auto C = b.NewBlock();
    auto I2 = b.NewInst<isa::inst::Opcode::ADD>();

    auto D = b.NewBlock();
    auto PHI = b.NewInst<isa::inst::Opcode::PHI>();
    auto I3 = b.NewInst<isa::inst::Opcode::ADD>();

    auto E = b.NewBlock();
    auto RET = b.NewInst<isa::inst::Opcode::RETURN>();

    b.SetInputs(IF0, C1, C0);

    b.SetInputs(I0, C0, C1);
    b.SetInputs(I1, I0, C0);
    b.SetInputs(I2, C2, C0);
    b.SetInputs(I3, PHI, C1);

    b.SetInputs(PHI, { { I2, C }, { I1, B } });
    b.SetInputs(RET, I3);

    b.SetSuccessors(START, { A });
    b.SetSuccessors(A, { C, B });
    b.SetSuccessors(B, { D });
    b.SetSuccessors(C, { D });
    b.SetSuccessors(D, { E });

    b.ConstructCFG();
    b.ConstructDFG();
    ASSERT_TRUE(b.RunChecks());

    auto pass = g.GetPassManager()->GetPass<LinearScan>();
    pass->SetArch<arch::Arch::UNSET>();
    pass->Run();

    CHECK_REGALLOC(C0, REGISTER, 0);
    CHECK_REGALLOC(C1, REGISTER, 1);
    CHECK_REGALLOC(C2, REGISTER, 2);

    CHECK_REGALLOC(I0, REGISTER, 2);
    CHECK_REGALLOC(I1, REGISTER, 0);
    CHECK_REGALLOC(I2, REGISTER, 2);
    CHECK_REGALLOC(I3, REGISTER, 0);

    CHECK_REGALLOC(PHI, REGISTER, 0);
}
