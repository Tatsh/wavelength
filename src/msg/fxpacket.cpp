#include "msg/fxpacket.h"

#include "os/binstream.h"
#include "os/bitstream.h"
#include "os/prnstream.h"

namespace {

constexpr int kSectionBits = 4;
constexpr int kTrackBits = 3;
constexpr int kTypeBits = 2;

} // namespace

void FXPacket::PrintExtra(PrnStream &stream) const {
    stream << "section: " << mSection << "track: " << mTrack << "type: " << mType << "on: " << mOn
           << "\n";
}

void FXPacket::saveGuts(BinStream &stream) const {
    unsigned short nPacked;
    BitStream bits(&nPacked, sizeof(nPacked));
    bits.Pack(mSection, kSectionBits);
    bits.Pack(mTrack, kTrackBits);
    bits.Pack(mType, kTypeBits);
    bits.PackBool(mOn);
    const unsigned short nWord = nPacked;
    stream.WriteEndian(&nWord, sizeof(nWord));
}

void FXPacket::restoreGuts(BinStream &stream) {
    unsigned short nPacked;
    stream.ReadEndian(&nPacked, sizeof(nPacked));
    BitStream bits(&nPacked, sizeof(nPacked));
    mSection = static_cast<int>(bits.Unpack(kSectionBits));
    mTrack = static_cast<int>(bits.Unpack(kTrackBits));
    mType = static_cast<int>(bits.Unpack(kTypeBits));
    mOn = bits.UnpackBool();
}
