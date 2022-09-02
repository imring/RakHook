# RakHook
RakHook is a library that adds RakNet events (incoming/outgoing Packets & RPC), emulation and sending Packets & RPC.  
There is support for versions 0.3.7-R1, 0.3.7-R3-1, 0.3.7-R4 and 0.3DL-R1.

## Functions

### SA:MP
- `std::uintptr_t rakhook::samp_addr(std::uintptr_t offset = 0)` - Get SA:MP module address with an offset.
- `samp_ver rakhook::samp_version()` - Get SA:MP version supported by RakHook.

### Events
- `bool rakhook::initialize()` - Initialize RakHook.
- `void rakhook::destroy()` - Destroy RakHook.
- `on_event<send_t> rakhook::on_send_packet` - Outgoing the packet.
- `on_event<receive_t> rakhook::on_receive_packet` - Incoming the packet.
- `on_event<send_rpc_t> rakhook::on_send_rpc` - Outgoing RPC.
- `on_event<receive_rpc_t> rakhook::on_receive_rpc` - Incoming RPC.

### Send/Emulate
- `bool rakhook::send(RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel)` - Send the packet.
- `bool rakhook::send_rpc(int id, RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel, bool sh_timestamp)` - Send RPC.
- `bool rakhook::emul_packet(RakNet::BitStream &pbs)` - Emulate the packet.
- `bool rakhook::emul_rpc(unsigned char id, RakNet::BitStream &rpc_bs)` - Emulate RPC.

## Example
You can learn the example [here](./example/).

## Use in projects
Recommended way to link the library - FetchContent, but you can use others (submodule, install, etc).