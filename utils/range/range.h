#ifndef __UTILS_RANGE_H_INCLUDED__
#define __UTILS_RANGE_H_INCLUDED__

#include "utils/macros.h"

#include <cstddef>
#include <ostream>

class Range
{
  public:
    Range(size_t from, size_t to) : start_{ from }, end_{ to }
    {
        ASSERT(from <= to);
    }
    DEFAULT_COPY_SEMANTIC(Range);
    DEFAULT_MOVE_SEMANTIC(Range);
    DEFAULT_DTOR(Range);

    GETTER_SETTER(Start, size_t, start_);
    GETTER_SETTER(End, size_t, end_);

    bool Contains(size_t pt);
    size_t Length();
    bool IsEmpty();

    static bool IfIntersect(const Range& l, const Range& r);
    static Range Intersection(const Range& l, const Range& r);
    static Range Union(const Range& l, const Range& r);

    bool operator==(const Range& r) const
    {
        return GetStart() == r.GetStart() && GetEnd() == r.GetEnd();
    }

  private:
    size_t start_;
    size_t end_;
};

inline std::ostream& operator<<(std::ostream& os, const Range& r)
{
    os << '[' << r.GetStart() << ", " << r.GetEnd() << ')';
    return os;
}

#endif
