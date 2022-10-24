
#include "ir/bb.h"
#include "ir/graph.h"
#include "ir/graph_builder.h"

#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

char IdToChar(IdType id)
{
    return (id == 0) ? '0' : (char)('A' + id - 1);
}

int main()
{
    // Graph g;
    // GraphBuilder b(&g);

    // auto p0 = b.NewParameter(0);

    // // FIXME: type specifiers. For now, there are nesessary fields, but there isn't needed
    // // functionality to work with it
    // auto c0 = b.NewConst(1U); // res{1U}
    // b.SetType(c0, DataType::INT);
    // auto c1 = b.NewConst(2U); // i{2U}
    // b.SetType(c1, DataType::INT);

    // auto b0 = b.NewBlock();
    // auto i0 = b.NewInst<Opcode::PHI>(); // i
    // auto i1 = b.NewInst<Opcode::PHI>(); // res
    // auto i2 = b.NewInst<Opcode::IF>(CondType::COND_LEQ);

    // auto b1 = b.NewBlock();
    // auto i3 = b.NewInst<Opcode::MUL>();
    // auto i4 = b.NewInst<Opcode::ADDI>(10);

    // auto b2 = b.NewBlock();
    // auto i5 = b.NewInst<Opcode::RETURN>();

    // b.SetInputs(i0, { { c1, g.BB_START_ID }, { i4, b1 } });
    // b.SetInputs(i2, i0, p0);
    // b.SetInputs(i1, { { c0, g.BB_START_ID }, { i3, b1 } });
    // b.SetInputs(i3, i1, i0);
    // b.SetInputs(i4, i0);
    // b.SetInputs(i5, i1);

    // b.SetSuccessors(b0, { b1, b2 });
    // b.SetSuccessors(b1, { b0 });
    // b.SetSuccessors(b2, {});

    // b.ConstructCFG();
    // b.ConstructDFG();

    // g.Dump();

    // if (!b.RunChecks()) {
    //     return -1;
    // }

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
    // ASSERT_TRUE(b.RunChecks());

    // assert(g.RunPass<RPO>());
    // auto rpo = g.GetPass<RPO>()->GetBlocks();

    auto rpo = g.RPOPass();
    std::vector<char> rpo_c{};

    for (auto bb : rpo) {
        rpo_c.push_back(IdToChar(bb->GetId()));
    }
    std::cout << "\n";

    return 0;
}

#pragma GCC diagnostic pop