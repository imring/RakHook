# RakHook
RakHook is a library that adds RakNet events (incoming/outgoing Packets & RPC), emulation and sending Packets & RPC.  
There is support for versions 0.3.7-R1, 0.3.7-R3-1, 0.3.7-R4 and 0.3DL-R1.

The RakHook part is header-only and does not depend on any hooking library directly:
all hooks are installed through a compile-time hook implementation (wrapper) of your choice.

## Functions

### Common

- `rakhook::version_manager` - Detects the `samp.dll` base address and the entry point and selects
  the matching offsets for the supported versions.
- `rakhook::signal<Func>` - Event container used by the hooker (callbacks are called in order,
  `bool` callbacks can stop the event by returning `false`).

### Wrappers

`rakhook::hooker<HookImpl>` is a class template with static members, where `HookImpl` is a hook implementation.  
RakHook has 3 ready-to-use wrappers:
- `rakhook::wrappers::cyanide` for [cyanide](https://github.com/imring/cyanide/tree/959fd05d56a779b584e7020b7243ff600799fc46);
- `rakhook::wrappers::minhook` for [minhook](https://github.com/TsudaKageyu/minhook/tree/v1.3.4);
- `rakhook::wrappers::kthook` for [kthook](https://github.com/kin4stat/kthook/tree/4331456543f408c42122ced22144cfb698031266).

### Events

- `bool rakhook::hooker<HookImpl>::initialize()` - Initialize RakHook.
- `void rakhook::hooker<HookImpl>::destroy()` - Destroy RakHook.
- `signal<send_t> rakhook::hooker<HookImpl>::on_send_packet` - Outgoing the packet.
- `signal<receive_t> rakhook::hooker<HookImpl>::on_receive_packet` - Incoming the packet.
- `signal<send_rpc_t> rakhook::hooker<HookImpl>::on_send_rpc` - Outgoing RPC.
- `signal<receive_rpc_t> rakhook::hooker<HookImpl>::on_receive_rpc` - Incoming RPC.
- `bool rakhook::hooker<HookImpl>::send(RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel)` - Send the packet.
- `bool rakhook::hooker<HookImpl>::send_rpc(int id, RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel, bool sh_timestamp)` - Send RPC.
- `bool rakhook::hooker<HookImpl>::emul_packet(RakNet::BitStream &pbs)` - Emulate the packet.
- `bool rakhook::hooker<HookImpl>::emul_rpc(unsigned char id, RakNet::BitStream &rpc_bs)` - Emulate RPC.

## Example
You can learn the example [here](./example/).

## Custom SA:MP version

If the loaded `samp.dll` version is not supported by RakHook, you can add it manually
with `rakhook::version_manager::add_version` before `rakhook::hooker<HookImpl>::initialize()` is called:

```cpp
using rh = rakhook::hooker<rakhook::wrappers::cyanide>;

// 0.3.7-R1
rh::versions.add_version(0x31DF13, {
    .samp_info           = 0x21a0f8, // CNetGame singleton
    .rakclient_interface = 0x3c9,    // CNetGame::m_pRakClient offset
    .destroy_interface   = 0x342d0,  // function to destroy CNetGame::m_pRakClient
    .handle_rpc_packet   = 0x372f0,  // RakPeer::HandleRPCPacket function
    .alloc_packet        = 0x347e0,  // AllocPacket function (see RakPeer.cpp)
    .offset_packets      = 0xdb6,    // RakPeer::packetSingleProducerConsumer offset
    .write_lock          = 0x35b10,  // DataStructures::SingleProducerConsumer::WriteLock function
    .write_unlock        = 0x35b50,  // DataStructures::SingleProducerConsumer::WriteUnlock function
});

rh::initialize(); // returns false if the loaded samp.dll is still unknown 
```

The first argument is the `AddressOfEntryPoint` from the PE header of the target `samp.dll`
and all `version_offsets` fields are offsets relative to the module base, except `offset_packets`,
which is a struct offset.

## Custom hook
A custom hook implementation must provide:

```cpp
struct my_hook {
    static bool initialize();
    static void uninitialize();

    template <typename Fn, auto Detour>
    struct hook {
        hook(Fn target);
        bool install();
        bool uninstall();
        Fn get_original();
    };
};
```

where `Detour` is a function pointer of the form `Ret (*)(Fn original, Args...)`.

## CMake options
- `RAKHOOK_WRAPPER_CYANIDE` - build the library with cyanide (`rakhook::cyanide`).
- `RAKHOOK_WRAPPER_MINHOOK` - build the library with MinHook (`rakhook::minhook`).
- `RAKHOOK_WRAPPER_KTHOOK` - build the library with kthook (`rakhook::kthook`).
- `RAKHOOK_TESTS` - build unit tests for the hook wrappers.
- `RAKHOOK_EXAMPLE` - build the example.
- `RAKHOOK_EXAMPLE_WRAPPER` - hook wrapper used by the example: `auto` (default, the first enabled one),
  `cyanide`, `minhook` or `kthook`.
- `RAKHOOK_INSTALL` - install targets and headers.

cyanide and kthook cannot be enabled at the same time because both vendor `xbyak`.

## Use in projects
Recommended way to link the library - FetchContent, but you can use others (submodule, install, etc).
