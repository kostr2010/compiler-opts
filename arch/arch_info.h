#ifndef __ARCH_INFO_H_INCLUDED__
#define __ARCH_INFO_H_INCLUDED__

#include <cstddef>

namespace arch {

enum class Arch
{
    UNSET,
    X86_64,
    ARM,
    MIPS
};

template <Arch ARCH>
struct ArchInfo;

template <>
struct ArchInfo<Arch::X86_64>
{
    static constexpr unsigned NUM_REGISTERS = 16;
};

}; // namespace arch
#endif
