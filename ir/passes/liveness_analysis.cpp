#include "liveness_analysis.h"
#include "bb.h"
#include "graph.h"

bool LivenessAnalysis::RunPass()
{
    ResetState();

    return true;
}

void LivenessAnalysis::ResetState()
{
}
