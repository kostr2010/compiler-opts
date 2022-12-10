#include "analyser.h"

void Analyser::InvalidateCfgDependentActivePasses()
{
    GetPass<DFS>()->SetValid(false);
    GetPass<BFS>()->SetValid(false);
    GetPass<RPO>()->SetValid(false);
    GetPass<PO>()->SetValid(false);
    GetPass<DomTree>()->SetValid(false);
    GetPass<LoopAnalysis>()->SetValid(false);
}