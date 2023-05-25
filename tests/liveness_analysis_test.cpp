#include "bb.h"
#include "graph.h"
#include "graph_builder.h"

#include "gtest/gtest.h"

static unsigned GetInstLiveNumber(const LivenessAnalysis* pass, IdType id)
{
    for (const auto& elem : pass->GetInstLiveNumbers()) {
        if (elem.first->GetId() == id) {
            return elem.second;
        }
    }

    UNREACHABLE("fail");
}

static Range GetInstLiveRange(const LivenessAnalysis* pass, IdType id)
{
    for (const auto& elem : pass->GetInstLiveRanges()) {
        if (elem.first->GetId() == id) {
            return elem.second;
        }
    }

    UNREACHABLE("fail");
}

static Range GetBasicBlockLiveRange(const LivenessAnalysis* pass, IdType id)
{
    for (const auto& elem : pass->GetBasicBlockLiveRanges()) {
        if (elem.first->GetId() == id) {
            return elem.second;
        }
    }

    UNREACHABLE("fail");
}

static constexpr const char* OP_TO_STR[] = {
#define CREATE(OPCODE, ...) #OPCODE,
    ISA_INSTRUCTION_LIST(CREATE)
#undef CREATE
};

#define DUMP_LIVE_NUMBERS()                                                                       \
    for (const auto& elem : pass->GetInstLiveNumbers()) {                                         \
        LOG(OP_TO_STR[elem.first->GetOpcode()] << " " << elem.second);                            \
    }

#define DUMP_BB_RANGES()                                                                          \
    for (const auto& elem : pass->GetBasicBlockLiveRanges()) {                                    \
        LOG(elem.first->GetId() << " " << elem.second);                                           \
    }

#define DUMP_INST_RANGES()                                                                        \
    for (const auto& elem : pass->GetInstLiveRanges()) {                                          \
        LOG(OP_TO_STR[elem.first->GetOpcode()] << " " << elem.second);                            \
    }

#define DUMP_LIVENESS()                                                                           \
    DUMP_LIVE_NUMBERS();                                                                          \
    DUMP_BB_RANGES();                                                                             \
    DUMP_INST_RANGES();

TEST(TestLiveness, Example0)
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
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

    auto B = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::MUL>();
    auto I1 = b.NewInst<isa::inst::Opcode::SUB>();

    auto C = b.NewBlock();
    auto I2 = b.NewInst<isa::inst::Opcode::ADD>();
    auto RET = b.NewInst<isa::inst::Opcode::RETURN>();

    b.SetInputs(PHI0, { { C0, Graph::BB_START_ID }, { I0, B } });
    b.SetInputs(PHI1, { { C1, Graph::BB_START_ID }, { I1, B } });

    b.SetInputs(IF0, PHI0, C0);

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

    auto pass = g.GetPassManager()->GetValidPass<LivenessAnalysis>();
    auto i_ranges = pass->GetInstLiveRanges();
    auto bb_ranges = pass->GetBasicBlockLiveRanges();

    ASSERT_EQ(GetBasicBlockLiveRange(pass, START), Range(0, 8));
    ASSERT_EQ(GetInstLiveNumber(pass, C0), 2);
    ASSERT_EQ(GetInstLiveNumber(pass, C1), 4);
    ASSERT_EQ(GetInstLiveNumber(pass, C2), 6);

    // A PREHEADER
    ASSERT_EQ(GetBasicBlockLiveRange(pass, g.GetStartBasicBlock()->GetSuccessor(0)->GetId()),
              Range(8, 10));

    ASSERT_EQ(GetBasicBlockLiveRange(pass, A), Range(10, 14));
    ASSERT_EQ(GetInstLiveNumber(pass, PHI0), 10);
    ASSERT_EQ(GetInstLiveNumber(pass, PHI1), 10);
    ASSERT_EQ(GetInstLiveNumber(pass, IF0), 12);

    ASSERT_EQ(GetBasicBlockLiveRange(pass, B), Range(14, 20));
    ASSERT_EQ(GetInstLiveNumber(pass, I0), 16);
    ASSERT_EQ(GetInstLiveNumber(pass, I1), 18);

    ASSERT_EQ(GetBasicBlockLiveRange(pass, C), Range(20, 26));
    ASSERT_EQ(GetInstLiveNumber(pass, I2), 22);
    ASSERT_EQ(GetInstLiveNumber(pass, RET), 24);

    ASSERT_EQ(GetInstLiveRange(pass, C0), Range(2, 20));
    ASSERT_EQ(GetInstLiveRange(pass, C1), Range(4, 10));
    ASSERT_EQ(GetInstLiveRange(pass, C2), Range(6, 22));

    ASSERT_EQ(GetInstLiveRange(pass, PHI0), Range(10, 22));
    ASSERT_EQ(GetInstLiveRange(pass, PHI1), Range(10, 18));
    ASSERT_EQ(GetInstLiveRange(pass, IF0), Range(12, 14));

    ASSERT_EQ(GetInstLiveRange(pass, I0), Range(16, 20));
    ASSERT_EQ(GetInstLiveRange(pass, I1), Range(18, 20));
    ASSERT_EQ(GetInstLiveRange(pass, I2), Range(22, 24));

    ASSERT_EQ(GetInstLiveRange(pass, RET), Range(24, 26));
}

TEST(TestLiveness, Example0_inverted)
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
    auto IF0 = b.NewInst<isa::inst::Opcode::IF>(Conditional::Type::EQ);

    auto B = b.NewBlock();
    auto I0 = b.NewInst<isa::inst::Opcode::MUL>();
    auto I1 = b.NewInst<isa::inst::Opcode::SUB>();

    auto C = b.NewBlock();
    auto I2 = b.NewInst<isa::inst::Opcode::ADD>();
    auto RET = b.NewInst<isa::inst::Opcode::RETURN>();

    b.SetInputs(PHI0, { { C0, Graph::BB_START_ID }, { I0, B } });
    b.SetInputs(PHI1, { { C1, Graph::BB_START_ID }, { I1, B } });

    b.SetInputs(IF0, C0, PHI0);

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

    auto pass = g.GetPassManager()->GetValidPass<LivenessAnalysis>();

    ASSERT_EQ(GetBasicBlockLiveRange(pass, START), Range(0, 8));
    ASSERT_EQ(GetInstLiveNumber(pass, C0), 2);
    ASSERT_EQ(GetInstLiveNumber(pass, C1), 4);
    ASSERT_EQ(GetInstLiveNumber(pass, C2), 6);

    // A PREHEADER
    ASSERT_EQ(GetBasicBlockLiveRange(pass, g.GetStartBasicBlock()->GetSuccessor(0)->GetId()),
              Range(8, 10));

    ASSERT_EQ(GetBasicBlockLiveRange(pass, A), Range(10, 14));
    ASSERT_EQ(GetInstLiveNumber(pass, PHI0), 10);
    ASSERT_EQ(GetInstLiveNumber(pass, PHI1), 10);
    ASSERT_EQ(GetInstLiveNumber(pass, IF0), 12);

    ASSERT_EQ(GetBasicBlockLiveRange(pass, B), Range(14, 20));
    ASSERT_EQ(GetInstLiveNumber(pass, I0), 16);
    ASSERT_EQ(GetInstLiveNumber(pass, I1), 18);

    ASSERT_EQ(GetBasicBlockLiveRange(pass, C), Range(20, 26));
    ASSERT_EQ(GetInstLiveNumber(pass, I2), 22);
    ASSERT_EQ(GetInstLiveNumber(pass, RET), 24);

    ASSERT_EQ(GetInstLiveRange(pass, C0), Range(2, 20));
    ASSERT_EQ(GetInstLiveRange(pass, C1), Range(4, 10));
    ASSERT_EQ(GetInstLiveRange(pass, C2), Range(6, 22));

    ASSERT_EQ(GetInstLiveRange(pass, PHI0), Range(10, 22));
    ASSERT_EQ(GetInstLiveRange(pass, PHI1), Range(10, 18));
    ASSERT_EQ(GetInstLiveRange(pass, IF0), Range(12, 14));

    ASSERT_EQ(GetInstLiveRange(pass, I0), Range(16, 20));
    ASSERT_EQ(GetInstLiveRange(pass, I1), Range(18, 20));
    ASSERT_EQ(GetInstLiveRange(pass, I2), Range(22, 24));

    ASSERT_EQ(GetInstLiveRange(pass, RET), Range(24, 26));
}

TEST(TestLiveness, Example1)
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

    auto pass = g.GetPassManager()->GetValidPass<LivenessAnalysis>();

    ASSERT_EQ(GetBasicBlockLiveRange(pass, START), Range(0, 8));
    ASSERT_EQ(GetInstLiveNumber(pass, C0), 2);
    ASSERT_EQ(GetInstLiveNumber(pass, C1), 4);
    ASSERT_EQ(GetInstLiveNumber(pass, C2), 6);

    ASSERT_EQ(GetBasicBlockLiveRange(pass, A), Range(8, 12));
    ASSERT_EQ(GetInstLiveNumber(pass, IF0), 10);

    ASSERT_EQ(GetBasicBlockLiveRange(pass, B), Range(20, 28));
    ASSERT_EQ(GetInstLiveNumber(pass, I0), 22);
    ASSERT_EQ(GetInstLiveNumber(pass, I1), 24);
    ASSERT_EQ(GetInstLiveNumber(pass, RET0), 26);

    ASSERT_EQ(GetBasicBlockLiveRange(pass, C), Range(12, 16));
    ASSERT_EQ(GetInstLiveNumber(pass, I2), 14);

    ASSERT_EQ(GetBasicBlockLiveRange(pass, D), Range(16, 20));
    ASSERT_EQ(GetInstLiveNumber(pass, RET1), 18);

    ASSERT_EQ(GetInstLiveRange(pass, C0), Range(2, 22));
    ASSERT_EQ(GetInstLiveRange(pass, C1), Range(4, 24));
    ASSERT_EQ(GetInstLiveRange(pass, C2), Range(6, 14));

    ASSERT_EQ(GetInstLiveRange(pass, IF0), Range(10, 12));

    ASSERT_EQ(GetInstLiveRange(pass, I0), Range(22, 24));
    ASSERT_EQ(GetInstLiveRange(pass, I1), Range(24, 26));
    ASSERT_EQ(GetInstLiveRange(pass, I2), Range(14, 18));

    ASSERT_EQ(GetInstLiveRange(pass, RET0), Range(26, 28));
    ASSERT_EQ(GetInstLiveRange(pass, RET1), Range(18, 20));
}

TEST(TestLiveness, Example2)
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

    auto pass = g.GetPassManager()->GetValidPass<LivenessAnalysis>();

    ASSERT_EQ(GetBasicBlockLiveRange(pass, START), Range(0, 8));
    ASSERT_EQ(GetInstLiveNumber(pass, C0), 2);
    ASSERT_EQ(GetInstLiveNumber(pass, C1), 4);
    ASSERT_EQ(GetInstLiveNumber(pass, C2), 6);

    ASSERT_EQ(GetBasicBlockLiveRange(pass, A), Range(8, 12));
    ASSERT_EQ(GetInstLiveNumber(pass, IF0), 10);

    ASSERT_EQ(GetBasicBlockLiveRange(pass, C), Range(12, 16));
    ASSERT_EQ(GetInstLiveNumber(pass, I2), 14);

    ASSERT_EQ(GetBasicBlockLiveRange(pass, B), Range(16, 22));
    ASSERT_EQ(GetInstLiveNumber(pass, I0), 18);
    ASSERT_EQ(GetInstLiveNumber(pass, I1), 20);

    ASSERT_EQ(GetBasicBlockLiveRange(pass, D), Range(22, 26));
    ASSERT_EQ(GetInstLiveNumber(pass, PHI), 22);
    ASSERT_EQ(GetInstLiveNumber(pass, I3), 24);

    ASSERT_EQ(GetBasicBlockLiveRange(pass, E), Range(26, 30));
    ASSERT_EQ(GetInstLiveNumber(pass, RET), 28);

    ASSERT_EQ(GetInstLiveRange(pass, C0), Range(2, 20));
    ASSERT_EQ(GetInstLiveRange(pass, C1), Range(4, 24));
    ASSERT_EQ(GetInstLiveRange(pass, C2), Range(6, 14));

    ASSERT_EQ(GetInstLiveRange(pass, IF0), Range(10, 12));

    ASSERT_EQ(GetInstLiveRange(pass, I0), Range(18, 20));
    ASSERT_EQ(GetInstLiveRange(pass, I1), Range(20, 22));
    ASSERT_EQ(GetInstLiveRange(pass, I2), Range(14, 16));
    ASSERT_EQ(GetInstLiveRange(pass, I3), Range(24, 28));

    ASSERT_EQ(GetInstLiveRange(pass, PHI), Range(22, 24));
    ASSERT_EQ(GetInstLiveRange(pass, RET), Range(28, 30));
}

#undef DUMP_LIVE_RANGE
