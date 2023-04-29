#include "liveness_analysis.h"
#include "ir/bb.h"
#include "ir/graph.h"

bool LivenessAnalysis::RunPass()
{
    ResetState();

    return true;
}

void LivenessAnalysis::ResetState()
{
}
