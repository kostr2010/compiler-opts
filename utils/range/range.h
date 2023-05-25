#ifndef __UTILS_RANGE_H_INCLUDED__
#define __UTILS_RANGE_H_INCLUDED__

#include "utils/macros.h"

#include <cstddef>
#include <ostream>

class Range
{
  public:
    Range(unsigned from, unsigned to) : start_{ from }, end_{ to }
    {
        ASSERT(from <= to);
    }
    DEFAULT_COPY_SEMANTIC(Range);
    DEFAULT_MOVE_SEMANTIC(Range);
    DEFAULT_DTOR(Range);

    GETTER_SETTER(Start, unsigned, start_);
    GETTER_SETTER(End, unsigned, end_);

    bool Contains(unsigned pt);
    unsigned Length();
    bool IsEmpty();

    static bool IfIntersect(const Range& l, const Range& r);
    static Range Intersection(const Range& l, const Range& r);
    static Range Union(const Range& l, const Range& r);

    bool operator==(const Range& r) const
    {
        return GetStart() == r.GetStart() && GetEnd() == r.GetEnd();
    }

  private:
    unsigned start_;
    unsigned end_;
};

inline std::ostream& operator<<(std::ostream& os, const Range& r)
{
    os << '[' << r.GetStart() << ", " << r.GetEnd() << ')';
    return os;
}

#endif
