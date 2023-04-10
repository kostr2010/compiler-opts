#ifndef __MARKABLE_H_INCLUDED__
#define __MARKABLE_H_INCLUDED__

#include <array>
#include <cstdint>

#include "macros.h"
#include "marker.h"
#include "marking_params.h"

namespace marking {
class Markable
{
  public:
    Markable();

    bool ProbeMark(const Marker* marker);
    // true if mark was set, false otherwise
    bool SetMark(const Marker* marker);
    // true if mark was set, false otherwise
    bool ClearMark(const Marker* marker);

  private:
    using Markers = std::array<Marker::MarkerGenT, NumConcurrentMarkers::value>;
    Markers markers_;
};
};

#endif