#include "markable.h"

namespace marking {

Markable::Markable()
{
    markers_.fill(Marker::GenUnset());
}

bool Markable::ProbeMark(const Marker* marker)
{
    return markers_[marker->GetSlot()] == marker->GetGen();
}

// true if mark was set, false otherwise
bool Markable::SetMark(const Marker* marker)
{
    bool was_set = ProbeMark(marker);
    markers_[marker->GetSlot()] = marker->GetGen();
    return was_set;
}

// true if mark was set, false otherwise
bool Markable::ClearMark(const Marker* marker)
{
    bool was_set = ProbeMark(marker);
    markers_[marker->GetSlot()] = Marker::GenUnset();
    return was_set;
}

};