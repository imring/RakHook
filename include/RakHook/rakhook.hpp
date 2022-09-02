#ifndef RAKHOOK_HPP
#define RAKHOOK_HPP

#include <functional>

#include <cyanide/hook_impl_polyhook.hpp>

#include "RakHook/samp.hpp"
#include "RakNet/RakClientInterface.h"

static_assert(sizeof(std::size_t) == 4, "Only 32-bit builds are supported");

namespace rakhook {
template <typename T>
struct on_event : public std::vector<std::function<T>> {
    on_event &operator+=(std::function<T> func) {
        this->push_back(func);
        return *this;
    }
};

using send_t        = bool(RakNet::BitStream *bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel);
using receive_t     = bool(Packet *packet);
using send_rpc_t    = bool(int &id, RakNet::BitStream *bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel, bool &sh_timestamp);
using receive_rpc_t = bool(unsigned char &id, RakNet::BitStream *bs);

inline bool                initialized = false;
inline RakClientInterface *orig        = nullptr;

inline on_event<send_t>        on_send_packet;
inline on_event<receive_t>     on_receive_packet;
inline on_event<send_rpc_t>    on_send_rpc;
inline on_event<receive_rpc_t> on_receive_rpc;

bool initialize();
void destroy();

bool send(RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel);
bool send_rpc(int id, RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel, bool sh_timestamp);

bool emul_rpc(unsigned char id, RakNet::BitStream &rpc_bs);
bool emul_packet(RakNet::BitStream &pbs);
} // namespace rakhook

#endif // RAKHOOK_HPP