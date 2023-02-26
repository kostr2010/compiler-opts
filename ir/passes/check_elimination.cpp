#include "check_elimination.h"
#include "bb.h"
#include "graph.h"

bool CheckElimination::RunPass()
{
    ResetState();

    return true;
}

void CheckElimination::ResetState()
{
    zero_checks_.clear();
}
