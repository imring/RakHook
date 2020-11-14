#include "RakHook/rakhook.hpp"
#include "RakHook/hook_shared.hpp"

#include "RakNet/StringCompressor.h"
#include "RakNet/PacketEnumerations.h"

#include <iostream>

hook_shared_t<void(*)()> game_loop_hook;

using wndproc_t = LRESULT(CALLBACK *)(HWND, UINT, WPARAM, LPARAM);
hook_shared_t<wndproc_t> wndproc_hook;

template<typename T>
bool read_with_size(RakNet::BitStream *bs, std::string &str) {
	T size;
	if (!bs->Read(size)) return false;
	str.resize(size);
	return bs->Read(str.data(), size);
}

template<typename T>
void write_with_size(RakNet::BitStream *bs, std::string &str) {
	T size = T(str.size());
	bs->Write(size);
	bs->Write(str.data(), str.size());
}

void game_loop() {
	game_loop_hook->call_original();

	static bool initialized = false;
	if (initialized || !rakhook::initialize()) return;

	StringCompressor::AddReference();

	rakhook::on_send_rpc += [](int &id, RakNet::BitStream *&bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel, bool &sh_timestamp) -> bool {
		std::cout << "send rpc: " << id << ' ' << bs << ' ' << priority << ' ' << reliability << ' ' << +ord_channel << ' ' << std::boolalpha << sh_timestamp << std::noboolalpha << '\n';
		return true;
	};

	rakhook::on_receive_packet += [](Packet *&p) -> bool {
		std::cout << "receive packet: " << +(*p->data) << ' ' << static_cast<void*>(p->data) << '\n';

		if (*p->data == ID_PLAYER_SYNC) return false;
		return true;
	};

	rakhook::on_send_packet += [](RakNet::BitStream *&bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel) -> bool {
		std::cout << "send packet: " << +(*bs->GetData()) << ' ' << bs << ' ' << priority << ' ' << reliability << ' ' << +ord_channel << '\n';
		return true;
	};

	rakhook::on_receive_rpc += [](unsigned char &id, RakNet::BitStream *&&bs) -> bool {
		std::cout << "receive rpc: " << +id << ' ' << bs << '\n';

		if (id == 61) { // RPC_ShowDialog
			unsigned short id;
			unsigned char style;
			std::string title;
			std::string but1;
			std::string but2;
			std::string text(4096, 0);

			// read
			bs->Read(id);
			bs->Read(style);
			read_with_size<unsigned char>(bs, title);
			read_with_size<unsigned char>(bs, but1);
			read_with_size<unsigned char>(bs, but2);
			StringCompressor::Instance()->DecodeString(text.data(), 4096, bs);

			title = std::to_string(id) + " | " + title;
			text = "[HOOKED] " + text;
			size_t pos = text.find('\0');
			if (pos != std::string::npos)
				text.insert(pos, " [HOOKED]");
			text.resize(4096);

			// write
			bs->Reset();
			bs->Write(id);
			bs->Write(style);
			write_with_size<unsigned char>(bs, title);
			write_with_size<unsigned char>(bs, but1);
			write_with_size<unsigned char>(bs, but2);

			StringCompressor::Instance()->EncodeString(text.data(), 4096, bs);
		} else if (id == 93) { // RPC_ClientMessage
			unsigned long color;
			std::string msg;

			// read
			bs->Read(color);
			read_with_size<unsigned int>(bs, msg);

			msg = "[HOOKED] " + msg;

			// write
			bs->Reset();
			bs->Write(color);
			write_with_size<unsigned int>(bs, msg);
		}

		return true;
	};

	initialized = true;
}

LRESULT wndproc_hooked(HWND &hwnd, UINT &Message, WPARAM &wparam, LPARAM &lparam) {
	if (Message == WM_KEYUP) {
		if (wparam == VK_NUMPAD4) {
			RakNet::BitStream rpc;

			rpc.Write<unsigned short>(999);
			std::string name = "test_name";
			write_with_size<unsigned char>(&rpc, name);
			rpc.Write<unsigned char>(1);

			rakhook::emul_rpc(11, rpc);
		}
	} else if (Message == WM_KEYDOWN) {
		if (wparam == VK_NUMPAD5) {
			RakNet::BitStream bs;
			float vec[3] = { 0 };
			float quat[4] = { 0 };

			bs.Write<unsigned char>(ID_PLAYER_SYNC);
			bs.Write<unsigned short>(999);
			bs.Write0(); bs.Write0();
			bs.Write<unsigned char>(0);
			bs.Write(vec);
			bs.Write(quat);
			bs.Write<unsigned char>(255);
			bs.Write<unsigned char>(0);
			bs.Write<unsigned char>(0);
			bs.Write<float>(0);
			bs.Write0();
			bs.Write0();

			rakhook::emul_packet(bs);
		}
	}
	return wndproc_hook->call_original(hwnd, Message, wparam, lparam);
}

class rakhook_example {
public:
	rakhook_example() {
		game_loop_hook = std::make_shared<hook_t<void(*)()>>(reinterpret_cast<void(*)()>(0x53BEE0), game_loop);
		wndproc_hook = std::make_shared<hook_t<wndproc_t>>(reinterpret_cast<wndproc_t>(0x747EB0), wndproc_hooked);
	}
} _rakhook_example;