#ifndef RAKHOOK_OFFSETS_HPP
#define RAKHOOK_OFFSETS_HPP

#include <cstdint>

namespace rakhook::offsets {
std::uintptr_t samp_info(bool base = false);
std::uintptr_t rakclient_interface(bool base = false);

std::uintptr_t destroy_interface(bool base = false);
std::uintptr_t handle_rpc_packet(bool base = false);

std::uintptr_t alloc_packet(bool base = false);
std::uintptr_t offset_packets(bool base = false);
std::uintptr_t write_lock(bool base = false);
std::uintptr_t write_unlock(bool base = false);
} // namespace rakhook::offsets

#endif // RAKHOOK_OFFSETS_HPP