
#include "ir/bb.h"
#include "ir/graph.h"
#include "ir/graph_builder.h"

#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

int main()
{
    Graph g;
    GraphBuilder b(&g);

    auto p0 = b.NewParameter(0);

    // FIXME: type specifiers. For now, there are nesessary fields, but there isn't needed
    // functionality to work with it
    auto c0 = b.NewConst(1U); // res{1U}
    b.SetType(c0, DataType::INT);
    auto c1 = b.NewConst(2U); // i{2U}
    b.SetType(c1, DataType::INT);

    auto b0 = b.NewBlock();
    auto i0 = b.NewInst<Opcode::PHI>(); // i
    auto i1 = b.NewInst<Opcode::IF>(CondType::COND_LEQ);

    auto b1 = b.NewBlock();
    auto i2 = b.NewInst<Opcode::PHI>(); // res
    auto i3 = b.NewInst<Opcode::MUL>();
    auto i4 = b.NewInst<Opcode::ADDI>(10);

    auto b2 = b.NewBlock();
    auto i5 = b.NewInst<Opcode::RETURN>();

    b.SetInputs(i0, { { c1, g.BB_START_ID }, { i4, b1 } });
    b.SetInputs(i1, i0, p0);
    b.SetInputs(i2, { { c0, g.BB_START_ID }, { i3, b1 } });
    b.SetInputs(i3, i2, i0);
    b.SetInputs(i4, i0);
    b.SetInputs(i5, i2);

    b.SetSuccessors(b0, { b1, b2 });
    b.SetSuccessors(b1, { b0 });
    b.SetSuccessors(b2, {});

    b.ConstructCFG();
    b.ConstructDFG();
    b.RunChecks();

    g.Dump();

    return 0;
}

#pragma GCC diagnostic pop