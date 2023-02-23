
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
    std::cout << marking::Marker::GetMarkOffset<DomTree>() << "\n";
    std::cout << marking::Marker::GetMarkOffset<LoopAnalysis>() << "\n";
    std::cout << marking::Marker::GetMarkOffset<DFS>() << "\n";
    std::cout << marking::Marker::GetMarkOffset<BFS>() << "\n";
    std::cout << marking::Marker::GetMarkOffset<RPO>() << "\n";
    std::cout << marking::Marker::GetMarkOffset<PO>() << "\n";
    std::cout << marking::Marker::GetMarkOffset<Peepholes>() << "\n";
    std::cout << marking::Marker::GetMarkOffset<DCE>() << "\n";
    std::cout << marking::Marker::GetMarkOffset<Inlining>() << "\n";
    std::cout << marking::Marker::GetMarkOffset<DBE>() << "\n";

    return 0;
}

#pragma GCC diagnostic pop
