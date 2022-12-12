#ifndef __PASS_LOOP_ANALYSIS_INCLUDED__
#define __PASS_LOOP_ANALYSIS_INCLUDED__

#include <memory>
#include <unordered_map>
#include <vector>

#include "loop.h"
#include "pass.h"

class BasicBlock;
class Loop;

class LoopAnalysis : public Pass
{
  public:
    enum Marks
    {
        GREY = 0,
        BLACK,
        GREEN,
        N_MARKS,
    };

    static constexpr size_t GetNumMarks()
    {
        return Marks::N_MARKS;
    }

    LoopAnalysis(Graph* graph) : Pass(graph)
    {
        InitStartLoop();
    }
    DEFAULT_DTOR(LoopAnalysis);

    bool RunPass() override;

    Loop* GetRootLoop()
    {
        return loops_.at(ROOT_LOOP_ID).get();
    }

  private:
    void CollectBackEdges(BasicBlock* bb);
    void ClearLoopMarks(Loop* loop);
    void PopulateLoops();
    void PopulateLoop(Loop* loop);
    void RunLoopSearch(Loop* cur_loop, BasicBlock* cur_bb);
    void SplitBackEdges();
    void SplitBackEdge(Loop* loop);
    void AddPreHeaders();
    void AddPreHeader(Loop* loop);
    void PropagatePhis(BasicBlock* bb, BasicBlock* pred);
    void PopulateRootLoop();
    void BuildLoopTree();
    void ResetStructs();
    void ClearMarks();
    void InitStartLoop();
    void RecalculateLoopsReducibility();

    std::unordered_map<IdType, size_t> id_to_dfs_idx_;

    static constexpr size_t ROOT_LOOP_ID = 0;
    std::vector<std::unique_ptr<Loop> > loops_{};
};

#endif
