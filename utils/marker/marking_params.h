#ifndef MARKING_PARAMS_H_INCLUDED
#define MARKING_PARAMS_H_INCLUDED

#include <cstddef>
#include <type_traits>

namespace marker {
using NumConcurrentMarkers = std::integral_constant<size_t, 4>;
};

#endif
