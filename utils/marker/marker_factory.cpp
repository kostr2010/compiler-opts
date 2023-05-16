#include "marker_factory.h"

namespace marker {

void MarkerFactory::InitMarker(Marker* m)
{
    ASSERT(m->GetGen() == Marker::GEN_UNSET);

    auto slots = Get()->slot_tracker_;
    for (size_t i = 0; i < slots.size(); ++i) {
        if (!slots[i]) {
            (Get()->slot_tracker_)[i] = true;
            auto gen = NewGen();
            auto slot = i;
            m->gen_ = gen;
            m->slot_ = slot;
            return;
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
    static auto current_gen = Marker::GEN_UNSET;
    // this will make so that GEN_UNSET will never be returned
    ++current_gen;
    current_gen = current_gen + 1 * (current_gen == Marker::GEN_UNSET);
    return current_gen;
}

};