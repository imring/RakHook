#ifndef RHEXAMPLE_EVENTS_HPP
#define RHEXAMPLE_EVENTS_HPP

#include <string>

#include "RakNet/BitStream.h"
#include "RakNet/StringCompressor.h"
#include "RakNet/PacketEnumerations.h"

#include "detail.hpp"

inline bool on_show_dialog(unsigned char &id, RakNet::BitStream *bs) {
    if (id != 61) // RPC_ShowDialog
        return true;
    unsigned short did;
    unsigned char  style;
    std::string    title, but1, but2, text(4096, 0);

    // read
    bs->Read(did);
    bs->Read(style);
    title = read_with_size<unsigned char>(bs);
    but1  = read_with_size<unsigned char>(bs);
    but2  = read_with_size<unsigned char>(bs);
    StringCompressor::Instance()->DecodeString(text.data(), 4096, bs);

    title      = std::to_string(id) + " | " + title;
    text       = "[HOOKED] " + text;
    size_t pos = text.find('\0');
    if (pos != std::string::npos)
        text.insert(pos, " [HOOKED]");
    text.resize(4096);

    // write
    bs->Reset();
    bs->Write(did);
    bs->Write(style);
    write_with_size<unsigned char>(bs, title);
    write_with_size<unsigned char>(bs, but1);
    write_with_size<unsigned char>(bs, but2);
    StringCompressor::Instance()->EncodeString(text.data(), 4096, bs);
    return true;
}

inline bool on_client_msg(unsigned char &id, RakNet::BitStream *bs) {
    if (id != 93) // RPC_ClientMessage
        return true;
    unsigned long color;
    std::string   msg;

    // read
    bs->Read(color);
    msg = read_with_size<unsigned int>(bs);

    msg = "[HOOKED] " + msg;

    // write
    bs->Reset();
    bs->Write(color);
    write_with_size<unsigned int>(bs, msg);
    return true;
}

inline bool nop_player_sync(Packet *p) {
    return *p->data != ID_PLAYER_SYNC;
}

#endif // RHEXAMPLE_EVENTS_HPP
