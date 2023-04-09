#ifndef __DEFAULT_PASSES_H_INCLUDED__
#define __DEFAULT_PASSES_H_INCLUDED__

#include "passes/pass_list.h"

#include "passes/bfs.h"
#include "passes/check_elimination.h"
#include "passes/dbe.h"
#include "passes/dce.h"
#include "passes/dfs.h"
#include "passes/dom_tree.h"
#include "passes/inlining.h"
#include "passes/linear_order.h"
#include "passes/loop_analysis.h"
#include "passes/peepholes.h"
#include "passes/po.h"
#include "passes/rpo.h"

using DefaultPasses = PassList<DomTree, LoopAnalysis, DFS, BFS, RPO, PO, Peepholes, DCE, Inlining,
                               DBE, CheckElimination, LinearOrder>;

#endif