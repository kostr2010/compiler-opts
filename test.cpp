
// #include "ir/bb.h"
// #include "ir/graph.h"
// #include "ir/graph_builder.h"
// #include "pass/pass_manager_default_passes.h"

#include <array>
#include <bitset>
#include <iostream>
#include <vector>

#include "ir/graph.h"
#include "utils/macros.h"
#include "utils/marker/marker.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wmissing-declarations"

// file for tomfoolery and experiments

using Markers = marker::Marker[2];

void bar(Markers m)
{
    LOG(m[0].IsUnset());
}

void foo()
{
    Markers m{};

    bar(m);
}

int main()
{
    foo();

    std::cout << Pass::PassTraits<LivenessAnalysis>::is_cfg_sensitive::value << "\n";

    return 0;
}

#pragma GCC diagnostic pop
