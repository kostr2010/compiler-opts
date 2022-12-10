#include "loop.h"
#include "bb.h"

Loop::Loop(IdType id, BasicBlock* header, BasicBlock* back_edge) : id_(id), header_(header)
{
    AddBackEdge(back_edge);
    CalculateReducibility();
}

bool Loop::IsRoot() const
{
    return outer_loop_ == nullptr;
}

void Loop::CalculateReducibility()
{
    if (header_ == nullptr || back_edges_.empty()) {
        return;
    }

    is_reducible_ = true;
    for (const auto& bck : back_edges_) {
        is_reducible_ = is_reducible_ & header_->Dominates(bck);
    }
}

bool Loop::IsReducible() const
{
    return is_reducible_;
}

void Loop::AddInnerLoop(Loop* loop)
{
    inner_loops_.push_back(loop);
}

void Loop::AddBlock(BasicBlock* bb)
{
    blocks_.push_back(bb);
}

void Loop::AddBackEdge(BasicBlock* bb)
{
    back_edges_.push_back(bb);
    if (!header_->Dominates(bb)) {
        is_reducible_ = false;
    }
}

void Loop::ClearBackEdges()
{
    back_edges_.clear();
}

void Loop::Dump()
{
    std::cout << "loop #" << id_ << "\n";
    std::cout << "\treducible:   " << is_reducible_ << "\n";
    std::cout << "\touter loop:  " << ((outer_loop_ == nullptr) ? 0 : outer_loop_->id_) << "\n";
    std::cout << "\tinner loops: [ ";
    for (const auto& loop : inner_loops_) {
        std::cout << loop->id_ << " ";
    }
    std::cout << "]\n";
    std::cout << "\tpre-header:  " << ((pre_header_ == nullptr) ? 0 : pre_header_->GetId())
              << "\n";
    std::cout << "\theader:      " << ((header_ == nullptr) ? 0 : header_->GetId()) << "\n";
    std::cout << "\tback edges:   [ ";
    for (const auto& block : back_edges_) {
        std::cout << block->GetId() << " ";
    }
    std::cout << "]\n";
    std::cout << "\tblocks:      [ ";
    for (const auto& block : blocks_) {
        std::cout << block->GetId() << " ";
    }
    std::cout << "]\n";
}