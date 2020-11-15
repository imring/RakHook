#include "offsets.hpp"
#include "rakhook.hpp"

#define new_offsets(name, v037r1, v037r3, v037r4, v03dlr1) \
uintptr_t rakhook::offsets::##name() { \
    static uintptr_t addr[] = { v037r1, v037r3, v037r4, v03dlr1 }; \
    \
    samp_version v = get_samp_version(); \
    if (v == SAMP_UNKNOWN) return 0; \
    return addr[v]; \
}

new_offsets(samp_info,           0x21a0f8, 0x26e8dc, 0x26ea0c, 0x2aca24)
new_offsets(rakclient_interface, 0x3c9,    0x2c,     0x2c,     0x2c)
new_offsets(destroy_interface,   0x342d0,  0x37680,  0x37d70,  0x37880)
new_offsets(handle_rpc_packet,   0x372f0,  0x3a6a0,  0x3ad90,  0x3a8a0)
new_offsets(alloc_packet,        0x347e0,  0x37b90,  0x38280,  0x37d90)
new_offsets(offset_packets,      0xdb6,    0xdb6,    0xdb6,    0xdb6)
new_offsets(write_lock,          0x35b10,  0x38ec0,  0x395b0,  0x390c0)
new_offsets(write_unlock,        0x35b50,  0x38f00,  0x395f0,  0x39100)

#undef new_offsets