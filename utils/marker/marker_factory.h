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
    friend class Marker;

  public:
    NO_COPY_SEMANTIC(MarkerFactory);
    NO_MOVE_SEMANTIC(MarkerFactory);

    static Marker AcquireMarker();

  private:
    DEFAULT_CTOR(MarkerFactory);
    DEFAULT_DTOR(MarkerFactory);

    static MarkerFactory* Get();
    static Marker::MarkerGenT NewGen();
    static void DisposeMarker(Marker* marker);

    using MarkerSlotsTracker = std::bitset<NumConcurrentMarkers::value>;
    MarkerSlotsTracker slot_tracker_;
};

};

#endif