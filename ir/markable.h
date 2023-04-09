#ifndef __MARKABLE_H_INCLUDED__
#define __MARKABLE_H_INCLUDED__

#include <cstdint>

#include "macros.h"

namespace marking {

using MarkHolder = uint64_t;

class Markable
{
  public:
    DEFAULT_CTOR(Markable);

    MarkHolder* GetMarkHolder();
    MarkHolder PeekMarkHolder() const;

  private:
    MarkHolder bits{};
};

};

#endif