#ifndef RAKHOOK_VERSION_MANAGER_HPP
#define RAKHOOK_VERSION_MANAGER_HPP

#include <bit>
#include <cstdint>
#include <map>

#include <Windows.h>

namespace rakhook {
struct version_offsets {
    std::uintptr_t samp_info;           // CNetGame singleton
    std::uintptr_t rakclient_interface; // CNetGame::m_pRakClient offset
    std::uintptr_t handle_rpc_packet;   // RakPeer::HandleRPCPacket function
    std::uintptr_t alloc_packet;        // AllocPacket function (see RakPeer.cpp)
    std::uintptr_t offset_packets;      // RakPeer::packetSingleProducerConsumer offset
    std::uintptr_t write_lock;          // DataStructures::SingleProducerConsumer::WriteLock function
    std::uintptr_t write_unlock;        // DataStructures::SingleProducerConsumer::WriteUnlock function
};

class version_manager {
public:
    version_manager();

    void add_version(const std::uintptr_t entry_point, const version_offsets &offsets) {
        versions_[entry_point] = offsets;
    }
    void update();

    [[nodiscard]] std::uintptr_t base() const {
        return base_;
    }
    [[nodiscard]] bool valid() const {
        return versions_.contains(current_entry_point_);
    }
    [[nodiscard]] const version_offsets &get() const {
        return current_offsets_;
    }

private:
    std::map<std::uintptr_t, version_offsets> versions_;
    std::uintptr_t                            base_                = 0;
    std::uintptr_t                            current_entry_point_ = 0;
    version_offsets                           current_offsets_     = {};
};

#ifdef UNICODE
inline constexpr auto module_name = L"samp.dll";
#else
inline constexpr auto module_name = "samp.dll";
#endif

inline version_manager::version_manager() {
    // 0.3.7 R1
    add_version(0x31DF13, {.samp_info           = 0x21a0f8,
                           .rakclient_interface = 0x3c9,
                           .handle_rpc_packet   = 0x372f0,
                           .alloc_packet        = 0x347e0,
                           .offset_packets      = 0xdb6,
                           .write_lock          = 0x35b10,
                           .write_unlock        = 0x35b50});

    // 0.3.7 R3-1
    add_version(0xCC4D0, {.samp_info           = 0x26e8dc,
                          .rakclient_interface = 0x2c,
                          .handle_rpc_packet   = 0x3a6a0,
                          .alloc_packet        = 0x37b90,
                          .offset_packets      = 0xdb6,
                          .write_lock          = 0x38ec0,
                          .write_unlock        = 0x38f00});

    // 0.3.7 R4
    add_version(0xCBCB0, {.samp_info           = 0x26ea0c,
                          .rakclient_interface = 0x2c,
                          .handle_rpc_packet   = 0x3ad90,
                          .alloc_packet        = 0x38280,
                          .offset_packets      = 0xdb6,
                          .write_lock          = 0x395b0,
                          .write_unlock        = 0x395f0});

    // 0.3.DL R1
    add_version(0xFDB60, {.samp_info           = 0x2aca24,
                          .rakclient_interface = 0x3c9,
                          .handle_rpc_packet   = 0x3a8a0,
                          .alloc_packet        = 0x37d90,
                          .offset_packets      = 0xdb6,
                          .write_lock          = 0x390c0,
                          .write_unlock        = 0x39100});

    update();
}

inline void version_manager::update() {
    base_                = std::bit_cast<std::uintptr_t>(GetModuleHandle(module_name));
    current_entry_point_ = 0;
    current_offsets_     = {};

    if (base_ == 0) {
        return;
    }

    const auto *ntheader = std::bit_cast<PIMAGE_NT_HEADERS>(base_ + std::bit_cast<PIMAGE_DOS_HEADER>(base_)->e_lfanew);
    current_entry_point_ = ntheader->OptionalHeader.AddressOfEntryPoint;

    if (const auto it = versions_.find(current_entry_point_); it != versions_.end()) {
        current_offsets_ = it->second;
    }
}
} // namespace rakhook

#endif // RAKHOOK_VERSION_MANAGER_HPP
