#ifndef MARKER_H_INCLUDED
#define MARKER_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "utils/macros.h"

namespace marker {

// RAII class for marker. acquired using default constructor and released upon scope end
class Marker
{
    friend class MarkerFactory;

  public:
    using MarkerGenT = uint16_t;
    static constexpr MarkerGenT GEN_UNSET = 0;

    Marker();
    ~Marker();
    NO_COPY_SEMANTIC(Marker);
    NO_MOVE_SEMANTIC(Marker);

    GETTER(Gen, gen_);
    GETTER(Slot, slot_);

    bool IsUnset() const;

  private:
    size_t slot_{};
    MarkerGenT gen_{};
};

template <size_t N>
using Markers = Marker[N];

}; // namespace marker

#endif