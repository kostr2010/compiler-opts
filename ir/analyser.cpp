#include "analyser.h"

Analyser::Analyser(Graph* graph)
{
    Allocate(graph, std::make_index_sequence<DefaultPasses::NumPasses::value>{});
}

void Analyser::InvalidateCFGSensitiveActivePasses()
{
    InvalidateCFGSensitivePasses(std::make_index_sequence<DefaultPasses::NumPasses::value>{});
}