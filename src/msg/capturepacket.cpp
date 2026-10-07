#include "msg/capturepacket.h"

#include "os/bitstream.h"

namespace {

constexpr int kTrackBits = 3;
constexpr int kBarBits = 8;

} // namespace

void CapturePacket::saveGuts(BinStream &stream) const {
    unsigned short nPacked = 0;
    BitStream bits(&nPacked, sizeof(nPacked));
    bits.Pack(mTrack, kTrackBits);
    bits.Pack(mBar, kBarBits);
    bits.PackBool(mAutocatch);
    bits.PackBool(mValid);
    unsigned short nHalfWord = nPacked;
    stream.WriteEndian(&nHalfWord, sizeof(nHalfWord));
}

void CapturePacket::restoreGuts(BinStream &stream) {
    unsigned short nPacked = 0;
    stream.ReadEndian(&nPacked, sizeof(nPacked));
    BitStream bits(&nPacked, sizeof(nPacked));
    mTrack = static_cast<int>(bits.Unpack(kTrackBits));
    mBar = static_cast<int>(bits.Unpack(kBarBits));
    mAutocatch = bits.UnpackBool();
    mValid = bits.UnpackBool();
}
