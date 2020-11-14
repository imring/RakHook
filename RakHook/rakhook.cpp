#include "consts.hpp"
#include "rakhook.hpp"
#include "hook_shared.hpp"
#include "hooked_rakclient_interface.hpp"

#include "RakNet/PacketEnumerations.h"

#include <Windows.h>

#ifndef MAX_ALLOCA_STACK_ALLOCATION
#define MAX_ALLOCA_STACK_ALLOCATION 1048576
#endif

hooked_rakclient_interface *hooked_interface = nullptr;
void *rakpeer = nullptr;
PlayerID gplayerid;

bool rakhook::initialized = false;
RakClientInterface *rakhook::orig = nullptr;

rakhook::on_event<rakhook::send_t>        rakhook::on_send_packet;
rakhook::on_event<rakhook::receive_t>     rakhook::on_receive_packet;
rakhook::on_event<rakhook::send_rpc_t>    rakhook::on_send_rpc;
rakhook::on_event<rakhook::receive_rpc_t> rakhook::on_receive_rpc;

using handle_rpc_packet_t = bool(__thiscall *)(void *rakpeer, const char *data, int length, PlayerID playerid);

hook_shared_t<void(*)(void*)>      destroy_ri_hook;
hook_shared_t<handle_rpc_packet_t> handle_rpc_hook;

namespace offs = rakhook::offsets::v037r3;

/// This template functions should be used when we perform cast from one pointer type to another
/// It's safer than using reiterpret_cast
///
/// It doesn't allow to do such things like:
/// int i = 10;
/// A *a = pointer_cast<A*>(i);
/// Only pointer could be used in this function.

template<typename result, typename source>
result pointer_cast(source *v)
{
	return static_cast<result>(static_cast<void*>(v));
}

template<typename result, typename source>
result pointer_cast(const source *v)
{
	return static_cast<result>(static_cast<const void*>(v));
}

// callbacks
void destroy_rakclient_interface(void *&rakclient_interface) {
	if (rakclient_interface == hooked_interface) {
		rakclient_interface = rakhook::orig;
		delete hooked_interface;
	}
	return destroy_ri_hook->call_original(rakclient_interface);
}

bool handle_rpc_packet(void *&rp, const char *&data, int &length, PlayerID &playerid) {
	rakpeer = rp;
	gplayerid = playerid;

	RakNet::BitStream incoming(pointer_cast<unsigned char*>(const_cast<char*>(data)), length, true);
	unsigned char id = 0;
	unsigned char* input = nullptr;
	unsigned int bits_data = 0;
	std::shared_ptr<RakNet::BitStream> callback_bs = std::make_shared<RakNet::BitStream>();

	incoming.IgnoreBits(8);
	if (data[0] == ID_TIMESTAMP)
		incoming.IgnoreBits(8 * (sizeof(RakNetTime) + sizeof(unsigned char)));

	int offset = incoming.GetReadOffset();
	incoming.Read(id);

	if (!incoming.ReadCompressed(bits_data))
		return false;

	if (bits_data) {
		bool used_alloca = false;
		if (BITS_TO_BYTES(incoming.GetNumberOfUnreadBits()) < MAX_ALLOCA_STACK_ALLOCATION) {
			input = pointer_cast<unsigned char*>(alloca(BITS_TO_BYTES(incoming.GetNumberOfUnreadBits())));
			used_alloca = true;
		} else input = new unsigned char[BITS_TO_BYTES(incoming.GetNumberOfUnreadBits())];

		if (!incoming.ReadBits(input, bits_data, false))
			return false;

		callback_bs = std::make_shared<RakNet::BitStream>(input, (bits_data / 8) + 1, true);

		if (!used_alloca)
			delete[] input;
	}

	for (auto &func: rakhook::on_receive_rpc)
		if (!func(id, callback_bs.get()))
			return false;

	incoming.SetWriteOffset(offset);
	incoming.Write(id);
	bits_data = callback_bs->GetNumberOfBytesUsed() * 8;
	incoming.WriteCompressed(bits_data);
	if (bits_data)
		incoming.WriteBits(callback_bs->GetData(), bits_data, false);

	return handle_rpc_hook->call_original(rp, pointer_cast<char*>(incoming.GetData()), incoming.GetNumberOfBytesUsed(), playerid);
}

// functions
uintptr_t rakhook::samp_addr(uintptr_t offset) {
	static uintptr_t samp_module = reinterpret_cast<uintptr_t>(GetModuleHandle("samp.dll"));
	return samp_module + offset;
}

bool rakhook::initialize() {
	if (initialized) return true;

	uintptr_t samp_info = *reinterpret_cast<uintptr_t*>(samp_addr(offs::samp_info));
	if (!samp_info) return false;

	RakClientInterface **rakclient_interface = reinterpret_cast<RakClientInterface **>(samp_info + offs::rakclient_interface);
	if (!(orig = *rakclient_interface)) return false;

	hooked_interface = new hooked_rakclient_interface(orig);
	*rakclient_interface = pointer_cast<RakClientInterface *>(hooked_interface);

	destroy_ri_hook = std::make_shared<hook_t<void(*)(void*)>>(reinterpret_cast<void(*)(void*)>(samp_addr(offs::destroy_interface)), destroy_rakclient_interface);
	handle_rpc_hook = std::make_shared<hook_t<handle_rpc_packet_t>>(reinterpret_cast<handle_rpc_packet_t>(samp_addr(offs::handle_rpc_packet)), handle_rpc_packet);

	return (initialized = true);
}

void rakhook::destroy() {
	if (!initialized) return;

	uintptr_t samp_info = *reinterpret_cast<uintptr_t*>(samp_addr(offs::samp_info));
	if (!samp_info) return;

	RakClientInterface **rakclient_interface = reinterpret_cast<RakClientInterface **>(samp_info + offs::rakclient_interface);
	*rakclient_interface = orig;

	destroy_ri_hook.reset();
	handle_rpc_hook.reset();
	delete hooked_interface;
}

bool rakhook::send(RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel) {
	if (!initialized) return false;
	return orig->Send(bs, priority, reliability, ord_channel);
}

bool rakhook::send_rpc(int id, RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel, bool sh_timestamp) {
	if (!initialized) return false;
	return orig->RPC(&id, bs, priority, reliability, ord_channel, sh_timestamp);
}

bool rakhook::emul_rpc(unsigned char id, RakNet::BitStream &rpc_bs) {
	if (!initialized || !rakpeer) return false;
	RakNet::BitStream bs;
	bs.Write<unsigned char>(ID_RPC);
	bs.Write(id);
	bs.WriteCompressed<unsigned int>(rpc_bs.GetNumberOfBytesUsed() * 8);
	bs.WriteBits(rpc_bs.GetData(), rpc_bs.GetNumberOfBytesUsed() * 8, false);
	return handle_rpc_hook->call_original(rakpeer, pointer_cast<char*>(bs.GetData()), bs.GetNumberOfBytesUsed(), gplayerid);
}

bool rakhook::emul_packet(RakNet::BitStream &pbs) {
	if (!initialized || !rakpeer) return false;
	Packet *send_packet = reinterpret_cast<Packet * (*)(size_t)>(samp_addr(offs::alloc_packet))(pbs.GetNumberOfBytesUsed());
	memcpy(send_packet->data, pbs.GetData(), send_packet->length);

	// RakPeer::AddPacketToProducer
	char *packets = static_cast<char *>(rakpeer) + offs::offset_packets;
	auto write_lock = reinterpret_cast<Packet **(__thiscall *)(void*)>(samp_addr(offs::write_lock));
	auto write_unlock = reinterpret_cast<void(__thiscall *)(void*)>(samp_addr(offs::write_unlock));

	*write_lock(packets) = send_packet;
	write_unlock(packets);

	return true;
}