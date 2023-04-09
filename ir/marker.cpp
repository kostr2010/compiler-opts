#include "marker.h"

namespace marking {

MarkHolder* Markable::GetMarkHolder()
{
    return &bits;
}

MarkHolder Markable::PeekMarkHolder() const
{
    return bits;
}

};