#ifndef __PASS_LOOP_ANALYSIS_INCLUDED__
#define __PASS_LOOP_ANALYSIS_INCLUDED__

#include <memory>
#include <unordered_map>
#include <vector>

#include "ir/loop.h"
#include "ir/typedefs.h"
#include "pass.h"
#include "utils/marker/marker.h"

class BasicBlock;

class LoopAnalysis : public Pass
{
  public:
    using is_cfg_sensitive = std::true_type;

    enum MarksBckEdges
    {
        GREY = 0,
        BLACK,
        N_MARKS_BACKEDGES,
    };
    using MarkersBckEdges = marker::Markers<MarksBckEdges::N_MARKS_BACKEDGES>;

    enum MarksPopulate
    {
        GREEN = 0,
        N_MARKS_POPULATE,
    };
    using MarkersPopulate = marker::Markers<MarksPopulate::N_MARKS_POPULATE>;

    LoopAnalysis(Graph* graph) : Pass(graph)
    {
        InitStartLoop();
    }

    bool Run() override;

    Loop* GetRootLoop()
    {
        return loops_.at(ROOT_LOOP_ID).get();
    }

  private:
    void CollectBackEdges(BasicBlock* bb, const MarkersBckEdges markers);
    void PopulateLoops();
    void PopulateLoop(Loop* loop);
    void RunLoopSearch(Loop* cur_loop, BasicBlock* cur_bb, const MarkersPopulate markers);
    void SplitBackEdges();
    void SplitBackEdge(Loop* loop);
    void AddPreHeaders();
    void AddPreHeader(Loop* loop);
    void PropagatePhis(BasicBlock* bb, BasicBlock* pred);
    void PopulateRootLoop();
    void BuildLoopTree();
    void ResetState();
    void InitStartLoop();
    void RecalculateLoopsReducibility();
    void Check();

    std::unordered_map<IdType, size_t> id_to_dfs_idx_{};

    static constexpr unsigned ROOT_LOOP_ID = 0;
    std::vector<std::unique_ptr<Loop> > loops_{};
};

#endif
