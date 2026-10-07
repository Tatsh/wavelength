#include "msg/freestylebumppacket.h"

#include "os/binstream.h"
#include "os/bitstream.h"
#include "os/prnstream.h"

namespace {

constexpr int kNetOrderBits = 3;
constexpr int kTrackBits = 3;

} // namespace

void FreestyleBumpPacket::saveGuts(BinStream &stream) const {
    unsigned char nPacked;
    BitStream bits(&nPacked, sizeof(nPacked));
    bits.Pack(mNetOrder, kNetOrderBits);
    bits.Pack(mTrack, kTrackBits);
    const unsigned char nByte = nPacked;
    stream.Write(&nByte, sizeof(nByte));
}

void FreestyleBumpPacket::restoreGuts(BinStream &stream) {
    unsigned char nPacked;
    stream.Read(&nPacked, sizeof(nPacked));
    BitStream bits(&nPacked, sizeof(nPacked));
    mNetOrder = static_cast<int>(bits.Unpack(kNetOrderBits));
    mTrack = static_cast<int>(bits.Unpack(kTrackBits));
}

void FreestyleBumpPacket::PrintExtra(PrnStream &stream) const {
    stream << "player: " << mNetOrder << " " << "track: " << mTrack << "\n";
}
