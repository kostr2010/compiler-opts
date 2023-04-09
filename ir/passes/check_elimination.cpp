#include "check_elimination.h"
#include "bb.h"
#include "graph.h"

GEN_VISIT_FUNCTIONS_WITH_BLOCK_ORDER(CheckElimination, RPO);

bool CheckElimination::RunPass()
{
    ResetState();

    return true;
}

void CheckElimination::ResetState()
{
    zero_checks_.clear();
    null_checks_.clear();
    size_checks_.clear();
}
