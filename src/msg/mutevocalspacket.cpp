#include "msg/mutevocalspacket.h"

#include "os/bitstream.h"

namespace {

constexpr int kSectionBits = 4;
constexpr int kTrackBits = 3;
constexpr int kPackedBytes = 1;

} // namespace

void MuteVocalsPacket::saveGuts(BinStream &stream) const {
    unsigned char packed[kPackedBytes];
    BitStream bits(packed, kPackedBytes);
    bits.Pack(mSection, kSectionBits);
    bits.Pack(mTrack, kTrackBits);
    bits.PackBool(mMute);
    stream.Write(packed, kPackedBytes);
}

void MuteVocalsPacket::restoreGuts(BinStream &stream) {
    unsigned char packed[kPackedBytes];
    stream.Read(packed, kPackedBytes);
    BitStream bits(packed, kPackedBytes);
    mSection = static_cast<int>(bits.Unpack(kSectionBits));
    mTrack = static_cast<int>(bits.Unpack(kTrackBits));
    mMute = bits.UnpackBool();
}

void MuteVocalsPacket::PrintExtra(PrnStream &stream) const {
    stream << "track: " << mTrack << "mute: " << mMute << "\n";
}
