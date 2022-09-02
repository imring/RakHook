/*
        PROJECT:		mod_sa
        LICENSE:		See LICENSE in the top level directory
        COPYRIGHT:		Copyright we_sux, BlastHack

        mod_sa is available from https://github.com/BlastHackNet/mod_s0beit_sa/

        mod_sa is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        mod_sa is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with mod_sa.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RAKHOOK_INTERFACE_HPP
#define RAKHOOK_INTERFACE_HPP

#include "RakHook/rakhook.hpp"
#include "../RakNet/RakClientInterface.h"

class hooked_rakclient_interface {
public:
    hooked_rakclient_interface(RakClientInterface *rc){};
    virtual ~hooked_rakclient_interface() = default;

    virtual bool Connect(const char *host, unsigned short serverPort, unsigned short clientPort, unsigned int depreciated, int threadSleepTimer) {
        return rakhook::orig->Connect(host, serverPort, clientPort, depreciated, threadSleepTimer);
    }

    virtual void Disconnect(unsigned int blockDuration, unsigned char orderingChannel = 0) {
        rakhook::orig->Disconnect(blockDuration, orderingChannel);
    }

    virtual void InitializeSecurity(const char *privKeyP, const char *privKeyQ) {
        rakhook::orig->InitializeSecurity(privKeyP, privKeyQ);
    }

    virtual void SetPassword(const char *_password) {
        rakhook::orig->SetPassword(_password);
    }

    virtual bool HasPassword(void) const {
        return rakhook::orig->HasPassword();
    }

    virtual bool Send(const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel) {
        return rakhook::orig->Send(data, length, priority, reliability, orderingChannel);
    }

    virtual bool Send(RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel) {
        for (auto it = rakhook::on_send_packet.begin(); it != rakhook::on_send_packet.end();) {
            if (auto f = *it) {
                if (!f(bitStream, priority, reliability, orderingChannel))
                    return false;
                it++;
            } else {
                it = rakhook::on_send_packet.erase(it);
            }
        }
        return rakhook::orig->Send(bitStream, priority, reliability, orderingChannel);
    }

    virtual Packet *Receive(void) {
        Packet *packet = rakhook::orig->Receive();
        if (!packet)
            return nullptr;

        for (auto it = rakhook::on_receive_packet.begin(); it != rakhook::on_receive_packet.end();) {
            if (auto f = *it) {
                if (!f(packet)) {
                    rakhook::orig->DeallocatePacket(packet);
                    return nullptr;
                }
                it++;
            } else {
                it = rakhook::on_receive_packet.erase(it);
            }
        }
        return packet;
    }

    virtual void DeallocatePacket(Packet *packet) {
        rakhook::orig->DeallocatePacket(packet);
    }

    virtual void PingServer(void) {
        rakhook::orig->PingServer();
    }

    virtual void PingServer(const char *host, unsigned short serverPort, unsigned short clientPort, bool onlyReplyOnAcceptingConnections) {
        rakhook::orig->PingServer(host, serverPort, clientPort, onlyReplyOnAcceptingConnections);
    }

    virtual int GetAveragePing(void) {
        return rakhook::orig->GetAveragePing();
    }

    virtual int GetLastPing(void) const {
        return rakhook::orig->GetLastPing();
    }

    virtual int GetLowestPing(void) const {
        return rakhook::orig->GetLowestPing();
    }

    virtual int GetPlayerPing(const PlayerID playerId) {
        return rakhook::orig->GetPlayerPing(playerId);
    }

    virtual void StartOccasionalPing(void) {
        rakhook::orig->StartOccasionalPing();
    }

    virtual void StopOccasionalPing(void) {
        rakhook::orig->StopOccasionalPing();
    }

    virtual bool IsConnected(void) const {
        return rakhook::orig->IsConnected();
    }

    virtual unsigned int GetSynchronizedRandomInteger(void) const {
        return rakhook::orig->GetSynchronizedRandomInteger();
    }

    virtual bool GenerateCompressionLayer(unsigned int inputFrequencyTable[256], bool inputLayer) {
        return rakhook::orig->GenerateCompressionLayer(inputFrequencyTable, inputLayer);
    }

    virtual bool DeleteCompressionLayer(bool inputLayer) {
        return rakhook::orig->DeleteCompressionLayer(inputLayer);
    };

    virtual void RegisterAsRemoteProcedureCall(int *uniqueID, void (*functionPointer)(RPCParameters *rpcParms)) {
        rakhook::orig->RegisterAsRemoteProcedureCall(uniqueID, functionPointer);
    };

    virtual void RegisterClassMemberRPC(int *uniqueID, void *functionPointer) {
        rakhook::orig->RegisterClassMemberRPC(uniqueID, functionPointer);
    }

    virtual void UnregisterAsRemoteProcedureCall(int *uniqueID) {
        rakhook::orig->UnregisterAsRemoteProcedureCall(uniqueID);
    }

    virtual bool RPC(int *uniqueID, const char *data, unsigned int bitLength, PacketPriority priority, PacketReliability reliability, char orderingChannel,
                     bool shiftTimestamp) {
        return rakhook::orig->RPC(uniqueID, data, bitLength, priority, reliability, orderingChannel, shiftTimestamp);
    };

    virtual bool RPC(int *uniqueID, RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel,
                     bool shiftTimestamp) {
        if (!uniqueID)
            return rakhook::orig->RPC(uniqueID, bitStream, priority, reliability, orderingChannel, shiftTimestamp);

        int _uniqueID = *uniqueID;
        for (auto it = rakhook::on_send_rpc.begin(); it != rakhook::on_send_rpc.end();) {
            if (auto f = *it) {
                if (!f(_uniqueID, bitStream, priority, reliability, orderingChannel, shiftTimestamp))
                    return false;
                it++;
            } else {
                it = rakhook::on_send_rpc.erase(it);
            }
        }
        return rakhook::orig->RPC(&_uniqueID, bitStream, priority, reliability, orderingChannel, shiftTimestamp);
    }

    virtual bool RPC_(int *uniqueID, RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel,
                      bool shiftTimestamp, NetworkID networkID) {
        return rakhook::orig->RPC_(uniqueID, bitStream, priority, reliability, orderingChannel, shiftTimestamp, networkID);
    }

    virtual void SetTrackFrequencyTable(bool b) {
        return rakhook::orig->SetTrackFrequencyTable(b);
    }

    virtual bool GetSendFrequencyTable(unsigned int outputFrequencyTable[256]) {
        return rakhook::orig->GetSendFrequencyTable(outputFrequencyTable);
    }

    virtual float GetCompressionRatio(void) const {
        return rakhook::orig->GetCompressionRatio();
    }

    virtual float GetDecompressionRatio(void) const {
        return rakhook::orig->GetDecompressionRatio();
    }

    virtual void AttachPlugin(void *messageHandler) {
        return rakhook::orig->AttachPlugin(messageHandler);
    }

    virtual void DetachPlugin(void *messageHandler) {
        return rakhook::orig->DetachPlugin(messageHandler);
    }

    virtual RakNet::BitStream *GetStaticServerData(void) {
        return rakhook::orig->GetStaticServerData();
    }

    virtual void SetStaticServerData(const char *data, const int length) {
        return rakhook::orig->SetStaticServerData(data, length);
    }

    virtual RakNet::BitStream *GetStaticClientData(const PlayerID playerId) {
        return rakhook::orig->GetStaticClientData(playerId);
    }

    virtual void SetStaticClientData(const PlayerID playerId, const char *data, const int length) {
        return rakhook::orig->SetStaticClientData(playerId, data, length);
    }

    virtual void SendStaticClientDataToServer(void) {
        return rakhook::orig->SendStaticClientDataToServer();
    }

    virtual PlayerID GetServerID(void) const {
        return rakhook::orig->GetServerID();
    }

    virtual PlayerID GetPlayerID(void) const {
        return rakhook::orig->GetPlayerID();
    }

    virtual PlayerID GetInternalID(void) const {
        return rakhook::orig->GetInternalID();
    }

    virtual const char *PlayerIDToDottedIP(const PlayerID playerId) const {
        return rakhook::orig->PlayerIDToDottedIP(playerId);
    }

    virtual void PushBackPacket(Packet *packet, bool pushAtHead) {
        return rakhook::orig->PushBackPacket(packet, pushAtHead);
    }

    virtual void SetRouterInterface(void *routerInterface) {
        return rakhook::orig->SetRouterInterface(routerInterface);
    }

    virtual void RemoveRouterInterface(void *routerInterface) {
        return rakhook::orig->RemoveRouterInterface(routerInterface);
    }

    virtual void SetTimeoutTime(RakNetTime timeMS) {
        return rakhook::orig->SetTimeoutTime(timeMS);
    }

    virtual bool SetMTUSize(int size) {
        return rakhook::orig->SetMTUSize(size);
    }

    virtual int GetMTUSize(void) const {
        return rakhook::orig->GetMTUSize();
    }

    virtual void AllowConnectionResponseIPMigration(bool allow) {
        return rakhook::orig->AllowConnectionResponseIPMigration(allow);
    }

    virtual void AdvertiseSystem(const char *host, unsigned short remotePort, const char *data, int dataLength) {
        return rakhook::orig->AdvertiseSystem(host, remotePort, data, dataLength);
    }

    virtual void *const GetStatistics(void) {
        return rakhook::orig->GetStatistics();
    }

    virtual void ApplyNetworkSimulator(double maxSendBPS, unsigned short minExtraPing, unsigned short extraPingVariance) {
        return rakhook::orig->ApplyNetworkSimulator(maxSendBPS, minExtraPing, extraPingVariance);
    }

    virtual bool IsNetworkSimulatorActive(void) {
        return rakhook::orig->IsNetworkSimulatorActive();
    }

    virtual PlayerIndex GetPlayerIndex(void) {
        return rakhook::orig->GetPlayerIndex();
    }
};

#endif // RAKHOOK_INTERFACE_HPP