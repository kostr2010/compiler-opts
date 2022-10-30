
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

    const auto START = g.GetStartBasicBlockId();
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

    g.GetAnalyser()->RunPass<RPO>();
    // g.GetAnalyser()->RunPass<DomTree>();

    // for (const auto* bb : g.GetAnalyser()->GetPass<RPO>()->GetBlocks()) {
    //     std::cout << IdToChar(bb->GetId()) << " : "
    //               << ((bb->GetImmDominator()) ? IdToChar(bb->GetImmDominator()->GetId()) : '-')
    //               << "\n";
    // }

    g.GetAnalyser()->RunPass<DomTree>();

    return 0;
}

#pragma GCC diagnostic pop
