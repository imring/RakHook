#ifndef RAKHOOK_HPP
#define RAKHOOK_HPP

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>

#include "RakHook/signal.hpp"
#include "RakHook/version_manager.hpp"

#include "RakNet/PacketEnumerations.h"
#include "RakNet/RakClientInterface.h"

static_assert(sizeof(std::size_t) == 4, "Only 32-bit builds are supported");

#ifndef MAX_ALLOCA_STACK_ALLOCATION
#define MAX_ALLOCA_STACK_ALLOCATION 1048576
#endif

namespace rakhook {
using send_t        = bool(RakNet::BitStream *bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel);
using receive_t     = bool(Packet *packet);
using send_rpc_t    = bool(int &id, RakNet::BitStream *bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel, bool &sh_timestamp);
using receive_rpc_t = bool(unsigned char &id, RakNet::BitStream *bs);

/*
 * Compile-time hooker class. HookImpl is a hook implementation that satisfies
 * the following contract:
 *
 *     static bool HookImpl::initialize();
 *     static void HookImpl::uninitialize();
 *
 *     template <typename Fn, auto Detour>
 *     struct HookImpl::hook {
 *         hook(Fn target);
 *         bool install();
 *         bool uninstall();
 *         Fn get_original();
 *     };
 *
 * where Detour is a function pointer of the form `Ret (*)(Fn original, Args...)`.
 * Ready-to-use implementations live in `RakHook/wrappers`.
 *
 * The RakClientInterface methods are hooked directly: their addresses are
 * taken from the virtual table of the client instance created by SA:MP.
 */
template <typename HookImpl>
class hooker {
public:
    using hook_impl = HookImpl;

    static inline bool                initialized = false;
    static inline RakClientInterface *orig        = nullptr;

    static inline signal<send_t>        on_send_packet;
    static inline signal<receive_t>     on_receive_packet;
    static inline signal<send_rpc_t>    on_send_rpc;
    static inline signal<receive_rpc_t> on_receive_rpc;

    static inline version_manager versions;

    static bool initialize() {
        if (initialized) {
            return true;
        }

        versions.update();
        if (versions.base() == 0 || !versions.valid()) {
            return false;
        }

        const version_offsets &off       = versions.get();
        const std::uintptr_t   samp_info = *std::bit_cast<std::uintptr_t *>(versions.base() + off.samp_info);
        if (samp_info == 0) {
            return false;
        }

        auto **rakclient_interface = std::bit_cast<RakClientInterface **>(samp_info + off.rakclient_interface);
        if (*rakclient_interface == nullptr) {
            return false;
        }

        orig = *rakclient_interface;

        auto **vtable = *std::bit_cast<void ***>(orig);
        if (vtable == nullptr) {
            return false;
        }

        if (!HookImpl::initialize()) {
            return false;
        }

        send_hook       = std::make_unique<send_hook_t>(std::bit_cast<send_fn>(vtable[vtable_send]));
        receive_hook    = std::make_unique<receive_hook_t>(std::bit_cast<receive_fn>(vtable[vtable_receive]));
        rpc_hook        = std::make_unique<rpc_hook_t>(std::bit_cast<rpc_fn>(vtable[vtable_rpc]));
        handle_rpc_hook = std::make_unique<handle_rpc_hook_t>(std::bit_cast<handle_rpc_packet_t>(versions.base() + off.handle_rpc_packet));

        const bool installed = send_hook->install() && receive_hook->install() && rpc_hook->install() && handle_rpc_hook->install();
        initialized          = true;

        if (!installed) {
            destroy();
            return false;
        }

        return true;
    }

    static void destroy() {
        if (!initialized) {
            return;
        }

        send_hook.reset();
        receive_hook.reset();
        rpc_hook.reset();
        handle_rpc_hook.reset();

        HookImpl::uninitialize();

        initialized = false;
        orig        = nullptr;
        rakpeer     = nullptr;
    }

    static bool send(RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel) {
        if (!initialized || !send_hook || !send_hook->get_original()) {
            return false;
        }
        return send_hook->get_original()(orig, bs, priority, reliability, ord_channel);
    }

    static bool send_rpc(int id, RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel, bool sh_timestamp) {
        if (!initialized || !rpc_hook || !rpc_hook->get_original()) {
            return false;
        }
        return rpc_hook->get_original()(orig, &id, bs, priority, reliability, ord_channel, sh_timestamp);
    }

    static bool emul_rpc(unsigned char id, RakNet::BitStream &rpc_bs) {
        if (!initialized || rakpeer == nullptr) {
            return false;
        }

        RakNet::BitStream bs;
        bs.Write<unsigned char>(ID_RPC);
        bs.Write(id);
        bs.WriteCompressed<unsigned int>(BYTES_TO_BITS(rpc_bs.GetNumberOfBytesUsed()));
        bs.WriteBits(rpc_bs.GetData(), BYTES_TO_BITS(rpc_bs.GetNumberOfBytesUsed()), false);

        return handle_rpc_hook->get_original()(rakpeer, std::bit_cast<char *>(bs.GetData()), bs.GetNumberOfBytesUsed(), gplayerid);
    }

    static bool emul_packet(RakNet::BitStream &pbs) {
        if (!initialized || rakpeer == nullptr) {
            return false;
        }
        const version_offsets &off = versions.get();

        Packet *send_packet = std::bit_cast<Packet *(*)(size_t)>(versions.base() + off.alloc_packet)(pbs.GetNumberOfBytesUsed());
        memcpy(send_packet->data, pbs.GetData(), send_packet->length);

        // RakPeer::AddPacketToProducer
        char *packets      = static_cast<char *>(rakpeer) + off.offset_packets;
        auto  write_lock   = std::bit_cast<Packet **(__thiscall *)(void *)>(versions.base() + off.write_lock);
        auto  write_unlock = std::bit_cast<void(__thiscall *)(void *)>(versions.base() + off.write_unlock);

        *write_lock(packets) = send_packet;
        write_unlock(packets);

        return true;
    }

private:
    using handle_rpc_packet_t = bool(__thiscall *)(void *, const char *, int, PlayerID);

    using send_fn    = bool(__thiscall *)(RakClientInterface *, RakNet::BitStream *, PacketPriority, PacketReliability, char);
    using receive_fn = Packet *(__thiscall *)(RakClientInterface *);
    using rpc_fn     = bool(__thiscall *)(RakClientInterface *, int *, RakNet::BitStream *, PacketPriority, PacketReliability, char, bool);

    static constexpr std::size_t vtable_send    = 6;  // bool Send(RakNet::BitStream *, PacketPriority, PacketReliability, char)
    static constexpr std::size_t vtable_receive = 8;  // Packet *Receive()
    static constexpr std::size_t vtable_rpc     = 25; // bool RPC(int *, RakNet::BitStream *, PacketPriority, PacketReliability, char, bool)

    // callbacks
    static bool __cdecl send_hooked(send_fn orig, RakClientInterface *self, RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability,
                                    char ord_channel) {
        if (!on_send_packet.call(bs, priority, reliability, ord_channel)) {
            return false;
        }
        return orig(self, bs, priority, reliability, ord_channel);
    }

    static Packet *__cdecl receive_hooked(receive_fn orig, RakClientInterface *self) {
        Packet *packet = orig(self);
        if (packet == nullptr) {
            return nullptr;
        }

        if (!on_receive_packet.call(packet)) {
            self->DeallocatePacket(packet);
            return nullptr;
        }
        return packet;
    }

    static bool __cdecl rpc_hooked(rpc_fn orig, RakClientInterface *self, int *uniqueID, RakNet::BitStream *bs, PacketPriority priority,
                                   PacketReliability reliability, char ord_channel, bool sh_timestamp) {
        if (uniqueID == nullptr) {
            return orig(self, uniqueID, bs, priority, reliability, ord_channel, sh_timestamp);
        }

        int id = *uniqueID;
        if (!on_send_rpc.call(id, bs, priority, reliability, ord_channel, sh_timestamp)) {
            return false;
        }
        return orig(self, &id, bs, priority, reliability, ord_channel, sh_timestamp);
    }

    static bool __cdecl handle_rpc_packet(handle_rpc_packet_t orig, void *rp, const char *data, int length, PlayerID playerid) {
        rakpeer   = rp;
        gplayerid = playerid;

        RakNet::BitStream                  incoming{std::bit_cast<unsigned char *>(const_cast<char *>(data)), static_cast<unsigned int>(length), true};
        unsigned char                      id        = 0;
        unsigned char                     *input     = nullptr;
        unsigned int                       bits_data = 0;
        std::shared_ptr<RakNet::BitStream> callback_bs{std::make_shared<RakNet::BitStream>()};

        incoming.IgnoreBits(8);
        if (data[0] == ID_TIMESTAMP) {
            incoming.IgnoreBits(8 * (sizeof(RakNetTime) + sizeof(unsigned char)));
        }

        int offset = incoming.GetReadOffset();
        incoming.Read(id);

        if (!incoming.ReadCompressed(bits_data)) {
            return false;
        }

        if (bits_data != 0) {
            bool used_alloca = false;
            if (BITS_TO_BYTES(incoming.GetNumberOfUnreadBits()) < MAX_ALLOCA_STACK_ALLOCATION) {
                input       = std::bit_cast<unsigned char *>(alloca(BITS_TO_BYTES(incoming.GetNumberOfUnreadBits())));
                used_alloca = true;
            } else {
                input = new unsigned char[BITS_TO_BYTES(incoming.GetNumberOfUnreadBits())];
            }

            if (!incoming.ReadBits(input, bits_data, false)) {
                if (!used_alloca) {
                    delete[] input;
                }
                return false;
            }

            callback_bs = std::make_shared<RakNet::BitStream>(input, BITS_TO_BYTES(bits_data), true);

            if (!used_alloca) {
                delete[] input;
            }
        }

        if (!on_receive_rpc.call(id, callback_bs.get())) {
            return false;
        }

        incoming.SetWriteOffset(offset);
        incoming.Write(id);
        bits_data = BYTES_TO_BITS(callback_bs->GetNumberOfBytesUsed());
        incoming.WriteCompressed(bits_data);
        if (bits_data != 0) {
            incoming.WriteBits(callback_bs->GetData(), bits_data, false);
        }

        return orig(rp, std::bit_cast<char *>(incoming.GetData()), incoming.GetNumberOfBytesUsed(), playerid);
    }

    using send_hook_t       = typename HookImpl::template hook<send_fn, &send_hooked>;
    using receive_hook_t    = typename HookImpl::template hook<receive_fn, &receive_hooked>;
    using rpc_hook_t        = typename HookImpl::template hook<rpc_fn, &rpc_hooked>;
    using handle_rpc_hook_t = typename HookImpl::template hook<handle_rpc_packet_t, &handle_rpc_packet>;

    static inline void                              *rakpeer = nullptr;
    static inline PlayerID                           gplayerid{};
    static inline std::unique_ptr<send_hook_t>       send_hook;
    static inline std::unique_ptr<receive_hook_t>    receive_hook;
    static inline std::unique_ptr<rpc_hook_t>        rpc_hook;
    static inline std::unique_ptr<handle_rpc_hook_t> handle_rpc_hook;
};
} // namespace rakhook

#endif // RAKHOOK_HPP
