#include "msg/cripplerpacket.h"

#include "os/binstream.h"

void CripplerPacket::saveGuts(BinStream &stream) const {
    const unsigned char nVictim = static_cast<unsigned char>(mVictim);
    stream.Write(&nVictim, sizeof(nVictim));
}

void CripplerPacket::restoreGuts(BinStream &stream) {
    unsigned char nVictim;
    stream.Read(&nVictim, sizeof(nVictim));
    mVictim = nVictim;
}
