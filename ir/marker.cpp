#include "marker.h"

namespace marking {

MarkHolder* Markable::GetMarkHolder()
{
    return &bits;
}

};