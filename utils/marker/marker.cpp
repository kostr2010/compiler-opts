#include "marker.h"
#include "marker_factory.h"

namespace marker {

Marker::Marker(size_t slot, MarkerGenT gen) : slot_{ slot }, gen_{ gen }
{
}

Marker::~Marker()
{
    MarkerFactory::DisposeMarker(this);
}

bool Marker::IsUnset() const
{
    return gen_ == GenUnset();
}

void Marker::Unset()
{
    gen_ = GenUnset();
}

};
