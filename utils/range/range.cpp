#include "range.h"

bool Range::IfIntersect(const Range& l, const Range& r)
{
    bool is_l_before_r = l.end_ <= r.start_;
    bool is_r_before_l = r.end_ <= l.start_;

    return !is_l_before_r && !is_r_before_l;
}

bool Range::Contains(size_t pt)
{
    return start_ <= pt && pt < end_;
}

bool Range::IsEmpty()
{
    return start_ == end_;
}

Range Range::Intersection(const Range& l, const Range& r)
{
    ASSERT(IfIntersect(l, r));

    bool end_rel_pos = l.end_ < r.end_;
    size_t end = end_rel_pos * l.end_ + !end_rel_pos * r.end_;

    bool start_rel_pos = l.start_ > r.start_;
    size_t start = start_rel_pos * l.start_ + !start_rel_pos * r.start_;

    return Range(start, end);
}

Range Range::Union(const Range& l, const Range& r)
{
    bool end_rel_pos = l.end_ > r.end_;
    size_t end = end_rel_pos * l.end_ + !end_rel_pos * r.end_;

    bool start_rel_pos = l.start_ < r.start_;
    size_t start = start_rel_pos * l.start_ + !start_rel_pos * r.start_;

    return Range(start, end);
}

size_t Range::Length()
{
    return end_ - start_;
}