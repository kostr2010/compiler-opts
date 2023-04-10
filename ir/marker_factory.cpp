#include "marker_factory.h"

namespace marking {

Marker MarkerFactory::AcquireMarker()
{
    auto slots = Get()->slot_tracker_;
    for (size_t i = 0; i < slots.size(); ++i) {
        if (!slots[i]) {
            (Get()->slot_tracker_)[i] = true;
            auto gen = NewGen();
            auto slot = i;
            return Marker(slot, gen);
        }
    }

    UNREACHABLE("couldn't find empty marker slot");
}

void MarkerFactory::DisposeMarker(Marker* marker)
{
    Get()->slot_tracker_[marker->GetSlot()] = false;
}

MarkerFactory* MarkerFactory::Get()
{
    static MarkerFactory factory{};
    return &factory;
}

Marker::MarkerGenT MarkerFactory::NewGen()
{
    static auto current_gen = Marker::GenUnset::value;
    // this will make so that GenUnset will never be returned
    ++current_gen;
    current_gen = current_gen + 1 * (current_gen == Marker::GenUnset::value);
    return current_gen;
}

};