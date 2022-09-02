#include <iostream>

#include "RakHook/rakhook.hpp"
#include "RakNet/StringCompressor.h"

#include "events.hpp"
#include "emul.hpp"

using game_loop_t = void (*)();
using wndproc_t   = LRESULT(CALLBACK *)(HWND, UINT, WPARAM, LPARAM);

void game_loop(game_loop_t orig) {
    orig();

    static bool initialized = false;
    if (initialized || !rakhook::initialize())
        return;
    StringCompressor::AddReference();

    // print incoming/outgoing packets/rpc
    rakhook::on_send_rpc +=
        [](int &id, RakNet::BitStream *bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel, bool &sh_timestamp) -> bool {
        std::cout << "send rpc: " << id << ' ' << bs << ' ' << priority << ' ' << reliability << ' ' << +ord_channel << ' ' << std::boolalpha << sh_timestamp
                  << std::noboolalpha << '\n';
        return true;
    };

    rakhook::on_receive_packet += [](Packet *p) -> bool {
        std::cout << "receive packet: " << +(*p->data) << ' ' << static_cast<void *>(p->data) << '\n';
        return true;
    };

    rakhook::on_send_packet += [](RakNet::BitStream *bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel) -> bool {
        std::cout << "send packet: " << +(*bs->GetData()) << ' ' << bs << ' ' << priority << ' ' << reliability << ' ' << +ord_channel << '\n';
        return true;
    };

    rakhook::on_receive_rpc += [](unsigned char &id, RakNet::BitStream *bs) -> bool {
        std::cout << "receive rpc: " << +id << ' ' << bs << '\n';
        return true;
    };

    // modify some packets/rpc
    rakhook::on_receive_rpc += on_show_dialog;
    rakhook::on_receive_rpc += on_client_msg;
    rakhook::on_receive_packet += nop_player_sync;

    initialized = true;
}

LRESULT wndproc_hooked(wndproc_t orig, HWND hwnd, UINT Message, WPARAM wparam, LPARAM lparam) {
    if (Message == WM_KEYUP) {
        if (wparam == VK_NUMPAD4) {
            change_name();
        }
    } else if (Message == WM_KEYDOWN) {
        if (wparam == VK_NUMPAD5) {
            emul_player_sync();
        }
    }
    return orig(hwnd, Message, wparam, lparam);
}

std::unique_ptr<cyanide::polyhook_x86<game_loop_t, decltype(&game_loop)>>    game_loop_hook;
std::unique_ptr<cyanide::polyhook_x86<wndproc_t, decltype(&wndproc_hooked)>> wndproc_hook;

class rakhook_example {
public:
    rakhook_example() {
        if (!rakhook::samp_addr() || rakhook::samp_version() == rakhook::samp_ver::unknown)
            return;

        game_loop_hook = std::make_unique<typename decltype(game_loop_hook)::element_type>(std::bit_cast<game_loop_t>(0x53BEE0), std::move(&game_loop));
        wndproc_hook   = std::make_unique<typename decltype(wndproc_hook)::element_type>(std::bit_cast<wndproc_t>(0x747EB0), std::move(&wndproc_hooked));

        game_loop_hook->install();
        wndproc_hook->install();
    }
} rakhook_example_;