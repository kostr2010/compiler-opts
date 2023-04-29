#include "pass_manager.h"

PassManager::PassManager(Graph* graph)
{
    Allocate(graph, std::make_index_sequence<DefaultPasses::NumPasses::value>{});
}

void PassManager::InvalidateCFGSensitiveActivePasses()
{
    InvalidateCFGSensitivePasses(std::make_index_sequence<DefaultPasses::NumPasses::value>{});
}