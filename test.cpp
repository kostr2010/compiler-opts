
#include "ir/bb.h"
#include "ir/graph.h"
#include "ir/graph_builder.h"

#include <bitset>
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

// file for tomfoolery and experiments

static inline char IdToChar(IdType id)
{
    return (id == 0) ? '0' : (char)('A' + id - 1);
}

int main()
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

    g.GetAnalyser()->RunPass<LoopAnalysis>();

    return 0;
}

#pragma GCC diagnostic pop
