#ifndef __DEFAULT_PASSES_H_INCLUDED__
#define __DEFAULT_PASSES_H_INCLUDED__

#include "pass_list.h"

#include "bfs.h"
#include "check_elimination.h"
#include "dbe.h"
#include "dce.h"
#include "dfs.h"
#include "dom_tree.h"
#include "inlining.h"
#include "linear_order.h"
#include "linear_scan.h"
#include "liveness_analysis.h"
#include "loop_analysis.h"
#include "peepholes.h"
#include "po.h"
#include "rpo.h"

using DefaultPasses = PassList<DomTree, LoopAnalysis, DFS, BFS, RPO, PO, Peepholes, DCE, Inlining,
                               DBE, CheckElimination, LinearOrder, LivenessAnalysis, LinearScan>;

#endif