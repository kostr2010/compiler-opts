
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
    auto c1 = b.NewConst(2U); // i{2U}

    auto b0 = b.NewBlock();
    auto i0 = b.NewInst(Opcode::PHI); // i
    auto i1 = b.NewInst(Opcode::IF);

    auto b1 = b.NewBlock();
    auto i2 = b.NewInst(Opcode::PHI); // res
    auto i3 = b.NewInst(Opcode::MUL);
    auto i4 = b.NewInst(Opcode::ADDI);

    auto b2 = b.NewBlock();
    auto i5 = b.NewInst(Opcode::RETURN);

    b.SetInstInputs(i0, c1, i4);
    b.SetInstInputs(i1, i0, p0);
    b.SetInstInputs(i2, c0, i3);
    b.SetInstInputs(i3, i2, i0);
    b.SetInstInputs(i4, i0);
    b.SetInstImm(i4, 1);
    b.SetInstInputs(i5, i2);

    b.SetSuccessors(b0, { b1, b2 });
    b.SetSuccessors(b1, { b0 });
    b.SetSuccessors(b2, {});

    b.ConstructCFG();
    b.ConstructDFG();

    g.Dump();

    return 0;
}

#pragma GCC diagnostic pop