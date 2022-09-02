#ifndef RHEXAMPLE_EMUL_HPP
#define RHEXAMPLE_EMUL_HPP

#include "RakHook/rakhook.hpp"

#include "detail.hpp"

inline void change_name() {
    RakNet::BitStream rpc;
    rpc.Write<unsigned short>(0);                      // playerId
    write_with_size<unsigned char>(&rpc, "test_name"); // name
    rpc.Write<unsigned char>(1);                       // success

    rakhook::emul_rpc(11, rpc); // SETPLAYERNAME
}

inline void emul_player_sync() {
    RakNet::BitStream bs;
    float             vec[3]  = {0};
    float             quat[4] = {0};

    bs.Write<unsigned char>(ID_PLAYER_SYNC);
    bs.Write<unsigned short>(0);  // playerId
    bs.Write0();                  // w/o leftRightKeys
    bs.Write0();                  // w/o upDownKeys
    bs.Write<unsigned short>(0);  // keysData
    bs.Write(vec);                // position
    bs.Write(quat);               // quaternion
    bs.Write<unsigned char>(255); // health & armor
    bs.Write<unsigned char>(0);   // weapon
    bs.Write<unsigned char>(0);   // specialAction
    bs.Write<float>(0);           // moveSpeed
    bs.Write0();                  // w/o surfingVehicleId & surfingOffsets
    bs.Write0();                  // w/o animationId & animationFlags

    rakhook::emul_packet(bs);
}

#endif // RHEXAMPLE_EMUL_HPP
