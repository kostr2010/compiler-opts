#ifndef MARKER_H_INCLUDED
#define MARKER_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "utils/macros.h"

namespace marker {

// RAII class for marker. acquired using MarkerFactory and released upon scope end
class Marker
{
    friend class MarkerFactory;

  public:
    using MarkerGenT = uint16_t;
    using GenUnset = std::integral_constant<MarkerGenT, 0>;

    GETTER(Gen, gen_);
    GETTER(Slot, slot_);

    ~Marker();

    bool IsUnset() const;

  private:
    Marker(size_t slot, MarkerGenT gen);

    void Unset();

    NO_COPY_SEMANTIC(Marker);
    NO_MOVE_SEMANTIC(Marker);

    size_t slot_;
    MarkerGenT gen_;
};

template <size_t N>
using Markers = Marker[N];

};

#endif