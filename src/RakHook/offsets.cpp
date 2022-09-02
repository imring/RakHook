#include "RakHook/samp.hpp"
#include "RakHook/detail.hpp"
#include "RakHook/offsets.hpp"

constexpr std::uintptr_t samp_info[]           = {0x21a0f8, 0x26e8dc, 0x26ea0c, 0x2aca24};
constexpr std::uintptr_t rakclient_interface[] = {0x3c9, 0x2c, 0x2c, 0x2c};
constexpr std::uintptr_t destroy_interface[]   = {0x342d0, 0x37680, 0x37d70, 0x37880};
constexpr std::uintptr_t handle_rpc_packet[]   = {0x372f0, 0x3a6a0, 0x3ad90, 0x3a8a0};
constexpr std::uintptr_t alloc_packet[]        = {0x347e0, 0x37b90, 0x38280, 0x37d90};
constexpr std::uintptr_t offset_packets[]      = {0xdb6, 0xdb6, 0xdb6, 0xdb6};
constexpr std::uintptr_t write_lock[]          = {0x35b10, 0x38ec0, 0x395b0, 0x390c0};
constexpr std::uintptr_t write_unlock[]        = {0x35b50, 0x38f00, 0x395f0, 0x39100};

std::uintptr_t get_offset(const std::uintptr_t addr[], bool base) {
    const rakhook::samp_ver v = rakhook::samp_version();
    if (v == rakhook::samp_ver::unknown)
        return 0;
    std::uintptr_t res = addr[rakhook::detail::to_underlying(v)];
    if (base)
        res += rakhook::samp_addr();
    return res;
}

#define new_offsets(name) \
    uintptr_t name(bool base) { return get_offset(::name, base); }

namespace rakhook::offsets {
new_offsets(samp_info);
new_offsets(rakclient_interface);
new_offsets(destroy_interface);
new_offsets(handle_rpc_packet);
new_offsets(alloc_packet);
new_offsets(offset_packets);
new_offsets(write_lock);
new_offsets(write_unlock);
} // namespace rakhook::offsets

#undef new_offsets