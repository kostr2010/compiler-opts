#ifndef MARKER_FACTORY_H_INCLUDED
#define MARKER_FACTORY_H_INCLUDED

#include <bitset>
#include <cstdint>

#include "marker.h"
#include "marking_params.h"
#include "utils/macros.h"

namespace marker {

class MarkerFactory
{
  public:
    static void InitMarker(Marker* m);
    static void DisposeMarker(Marker* marker);

  private:
    DEFAULT_CTOR(MarkerFactory);
    DEFAULT_DTOR(MarkerFactory);
    NO_COPY_SEMANTIC(MarkerFactory);
    NO_MOVE_SEMANTIC(MarkerFactory);

    static MarkerFactory* Get();
    static Marker::MarkerGenT NewGen();

    using MarkerSlotsTracker = std::bitset<NumConcurrentMarkers::value>;
    MarkerSlotsTracker slot_tracker_;
};

}; // namespace marker

#endif