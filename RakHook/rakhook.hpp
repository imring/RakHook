#pragma once
#include <vector>
#include <functional>

#include "RakNet/RakClientInterface.h"

namespace rakhook {
	enum samp_version {
		SAMP_UNKNOWN = -1,

		SAMP_037_R1 = 0,
		SAMP_037_R3_1,
		SAMP_037_R4,
		SAMP_03DL_R1
	};

	uintptr_t samp_addr(uintptr_t offset = 0);
	samp_version get_samp_version();

	template<typename T>
	struct on_event: public std::vector<std::function<T>> {
		on_event &operator+=(std::function<T> func) {
			this->push_back(func);
			return *this;
		}
	};

	extern bool initialized;
	extern RakClientInterface *orig;

	using send_t = bool(RakNet::BitStream *&bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel);
	extern on_event<send_t> on_send_packet;

	using receive_t = bool(Packet *&packet);
	extern on_event<receive_t> on_receive_packet;

	using send_rpc_t = bool(int &id, RakNet::BitStream *&bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel, bool &sh_timestamp);
	extern on_event<send_rpc_t> on_send_rpc;

	using receive_rpc_t = bool(unsigned char &id, RakNet::BitStream *&&bs);
	extern on_event<receive_rpc_t> on_receive_rpc;

	bool initialize();
	void destroy();

	bool send(RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel);
	bool send_rpc(int id, RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel, bool sh_timestamp);

	bool emul_rpc(unsigned char id, RakNet::BitStream &rpc_bs);
	bool emul_packet(RakNet::BitStream &pbs);
};