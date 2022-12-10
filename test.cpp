
#include "ir/bb.h"
#include "ir/graph.h"
#include "ir/graph_builder.h"
#include "ir/marker.h"

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
    // using ActivePasses = PassList<DomTree, LoopAnalysis, DFS, BFS, RPO, PO>;
    // std::cout << "DomTree " << marking::Marker::GetMarkOffset<DomTree>() << "\n";
    // std::cout << "LoopAnalysis " << marking::Marker::GetMarkOffset<LoopAnalysis>() << "\n";
    // std::cout << "DFS " << marking::Marker::GetMarkOffset<DFS>() << "\n";
    // std::cout << "BFS " << marking::Marker::GetMarkOffset<BFS>() << "\n";
    // std::cout << "RPO " << marking::Marker::GetMarkOffset<RPO>() << "\n";
    // std::cout << "PO " << marking::Marker::GetMarkOffset<PO>() << "\n";
    return 0;
}

#pragma GCC diagnostic pop
