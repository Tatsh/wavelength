#include "msg/erasebarpacket.h"

#include "os/binstream.h"
#include "os/bitstream.h"

namespace {

constexpr int kTrackBits = 3;
constexpr int kBarBits = 29;

} // namespace

void EraseBarPacket::saveGuts(BinStream &stream) const {
    unsigned int nPacked;
    BitStream bits(&nPacked, sizeof(nPacked));
    bits.Pack(mTrack, kTrackBits);
    bits.Pack(mBar, kBarBits);
    unsigned int nWord = nPacked;
    stream.WriteEndian(&nWord, sizeof(nWord));
}

void EraseBarPacket::restoreGuts(BinStream &stream) {
    unsigned int nPacked;
    stream.ReadEndian(&nPacked, sizeof(nPacked));
    BitStream bits(&nPacked, sizeof(nPacked));
    mTrack = static_cast<int>(bits.Unpack(kTrackBits));
    mBar = static_cast<int>(bits.Unpack(kBarBits));
}
