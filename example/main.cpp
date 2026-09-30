#include <iostream>

#include <Windows.h>

#include "RakHook/rakhook.hpp"
#include "RakNet/StringCompressor.h"

#include "emul.hpp"
#include "events.hpp"

#if defined(RAKHOOK_EXAMPLE_WRAPPER_CYANIDE)
#include "RakHook/wrappers/cyanide.hpp"
using example_wrapper = rakhook::wrappers::cyanide;
#elif defined(RAKHOOK_EXAMPLE_WRAPPER_MINHOOK)
#include "RakHook/wrappers/minhook.hpp"
using example_wrapper = rakhook::wrappers::minhook;
#elif defined(RAKHOOK_EXAMPLE_WRAPPER_KTHOOK)
#include "RakHook/wrappers/kthook.hpp"
using example_wrapper = rakhook::wrappers::kthook;
#else
#error "No RakHook hook wrapper selected for the example"
#endif

using game_loop_t = void (*)();
using wndproc_t   = LRESULT(CALLBACK *)(HWND, UINT, WPARAM, LPARAM);

using rh = rakhook::hooker<example_wrapper>;

void game_loop(game_loop_t orig) {
    orig();

    static bool initialized = false;
    if (initialized || !rh::initialize()) {
        return;
    }
    StringCompressor::AddReference();

    // print incoming/outgoing packets/rpc
    rh::on_send_rpc +=
        [](int &id, RakNet::BitStream *bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel, bool &sh_timestamp) -> bool {
        std::cout << "send rpc: " << id << ' ' << bs << ' ' << priority << ' ' << reliability << ' ' << +ord_channel << ' ' << std::boolalpha << sh_timestamp
                  << std::noboolalpha << '\n';
        return true;
    };

    rh::on_receive_packet += [](Packet *packet) -> bool {
        std::cout << "receive packet: " << +(*packet->data) << ' ' << static_cast<void *>(packet->data) << '\n';
        return true;
    };

    rh::on_send_packet += [](RakNet::BitStream *bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel) -> bool {
        std::cout << "send packet: " << +(*bs->GetData()) << ' ' << bs << ' ' << priority << ' ' << reliability << ' ' << +ord_channel << '\n';
        return true;
    };

    rh::on_receive_rpc += [](unsigned char &id, RakNet::BitStream *bs) -> bool {
        std::cout << "receive rpc: " << +id << ' ' << bs << '\n';
        return true;
    };

    // modify some packets/rpc
    rh::on_receive_rpc += on_show_dialog;
    rh::on_receive_rpc += on_client_msg;
    rh::on_receive_packet += nop_player_sync;

    initialized = true;
}

LRESULT wndproc_hooked(wndproc_t orig, HWND hwnd, UINT Message, WPARAM wparam, LPARAM lparam) {
    if (Message == WM_KEYUP) {
        if (wparam == VK_HOME) {
            change_name<rh>();
        }
    } else if (Message == WM_KEYDOWN) {
        if (wparam == VK_END) {
            emul_player_sync<rh>();
        }
    }
    return orig(hwnd, Message, wparam, lparam);
}

using game_loop_hook_t = example_wrapper::hook<game_loop_t, &game_loop>;
using wndproc_hook_t   = example_wrapper::hook<wndproc_t, &wndproc_hooked>;

std::unique_ptr<game_loop_hook_t> game_loop_hook;
std::unique_ptr<wndproc_hook_t>   wndproc_hook;

class rakhook_example {
public:
    rakhook_example() {
        if (!rh::versions.valid() || !example_wrapper::initialize()) {
            return;
        }

        game_loop_hook = std::make_unique<game_loop_hook_t>(std::bit_cast<game_loop_t>(0x53BEE0));
        wndproc_hook   = std::make_unique<wndproc_hook_t>(std::bit_cast<wndproc_t>(0x747EB0));

        game_loop_hook->install();
        wndproc_hook->install();
    }

    ~rakhook_example() {
        example_wrapper::uninitialize();
    }
} rakhook_example_;
