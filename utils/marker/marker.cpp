#include "marker.h"
#include "marker_factory.h"

namespace marker {

Marker::Marker()
{
    MarkerFactory::InitMarker(this);
}

Marker::~Marker()
{
    MarkerFactory::DisposeMarker(this);
}

bool Marker::IsUnset() const
{
    return gen_ == GEN_UNSET;
}

}; // namespace marker
