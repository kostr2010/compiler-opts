#ifndef __DEFAULT_PASSES_H_INCLUDED__
#define __DEFAULT_PASSES_H_INCLUDED__

#include "pass/pass_list.h"

#include "pass/bfs.h"
#include "pass/check_elimination.h"
#include "pass/dbe.h"
#include "pass/dce.h"
#include "pass/dfs.h"
#include "pass/dom_tree.h"
#include "pass/inlining.h"
#include "pass/linear_order.h"
#include "pass/liveness_analysis.h"
#include "pass/loop_analysis.h"
#include "pass/peepholes.h"
#include "pass/po.h"
#include "pass/rpo.h"

using DefaultPasses = PassList<DomTree, LoopAnalysis, DFS, BFS, RPO, PO, Peepholes, DCE, Inlining,
                               DBE, CheckElimination, LinearOrder, LivenessAnalysis>;

#endif