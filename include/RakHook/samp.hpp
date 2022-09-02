#ifndef RAKHOOK_SAMP_HPP
#define RAKHOOK_SAMP_HPP

#include <cstdint>

namespace rakhook {
enum class samp_ver {
    unknown = -1,

    v037r1 = 0,
    v037r31,
    v037r4,
    v03dlr1
};

std::uintptr_t samp_addr(std::uintptr_t offset = 0);
samp_ver       samp_version();
} // namespace rakhook

#endif // RAKHOOK_SAMP_HPP