# RakHook

## Russian
RakHook - библиотека, которая добавляет события RakNet'a (входящие/исходящие пакеты и RPC), эмуляцию и отправку пакетов и RPC.  
Есть одновременная поддержка версий 0.3.7-R1, 0.3.7-R3-1, 0.3.7-R4 и 0.3DL-R1.

Функции:
```cpp
uintptr_t samp_addr(uintptr_t offset = 0); // получить адрес samp.dll со смещением
samp_version get_samp_version(); // получить версию SA:MP.

bool rakhook::initialize(); // инициализация ракхука.
void rakhook::destroy(); // завершение работы библиотеки.

bool rakhook::send(RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel); // отправка пакета.
bool rakhook::send_rpc(int id, RakNet::BitStream *bs, PacketPriority priority, PacketReliability reliability, char ord_channel, bool sh_timestamp); // отправка RPC.

bool rakhook::emul_rpc(unsigned char id, RakNet::BitStream &rpc_bs); // эмуляция RPC.
bool rakhook::emul_packet(RakNet::BitStream &pbs); // эмуляция пакета.
```

События:
```cpp
// исходящий RPC.
rakhook::on_send_rpc += [](int &id, RakNet::BitStream *&bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel, bool &sh_timestamp) -> bool {
  return true;
};

// исходящий пакет.
rakhook::on_send_packet += [](RakNet::BitStream *&bs, PacketPriority &priority, PacketReliability &reliability, char &ord_channel) -> bool {
  return true;
};

// входящий RPC.
rakhook::on_receive_rpc += [](unsigned char &id, RakNet::BitStream *&&bs) -> bool {
  return true;
};

// входящий пакет.
rakhook::on_receive_packet += [](Packet *&p) -> bool {
  return true;
};
```