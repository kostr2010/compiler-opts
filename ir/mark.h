#ifndef __MARK_H_INCLUDED__
#define __MARK_H_INCLUDED__

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>

#include "macros.h"

template <size_t START = 0, size_t LENGTH = 1>
class Mark
{
    static constexpr size_t BYTE_SZ = 8;
    static constexpr size_t MAX_LEN = sizeof(uint64_t) * BYTE_SZ;
    static constexpr size_t END_BIT = START + LENGTH;
    static_assert(START < MAX_LEN);
    static_assert(END_BIT <= MAX_LEN);

  public:
    template <size_t LEN>
    using Next = Mark<START + LENGTH, LEN>;

    static constexpr uint64_t MaxValue()
    {
        return (1ULL << LENGTH) - 1;
    }

    static constexpr uint64_t Mask()
    {
        return MaxValue() << START;
    }

    static constexpr void Set(uint64_t* dst, uint64_t value = 1)
    {
        assert(value <= MaxValue());
        *dst = (*dst & ~Mask()) | (value << START);
    }

    static constexpr uint64_t Get(uint64_t src)
    {
        return (src >> START) & MaxValue();
    }

  private:
    NO_DEFAULT_CTOR(Mark);
    NO_DEFAULT_DTOR(Mark);
};

#endif