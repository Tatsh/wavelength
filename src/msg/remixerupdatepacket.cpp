#include "msg/remixerupdatepacket.h"

#include "os/bitstream.h"

namespace {

constexpr int kTrackBits = 3;
constexpr int kVersionBits = 28;

} // namespace

void RemixerUpdatePacket::saveGuts(BinStream &stream) const {
    int nPacked = 0;
    BitStream bits(&nPacked, sizeof(nPacked));
    bits.Pack(mTrack, kTrackBits);
    bits.PackBool(mRepeat);
    bits.Pack(mVersion, kVersionBits);
    int nWord = nPacked;
    stream.WriteEndian(&nWord, sizeof(nWord));
}

void RemixerUpdatePacket::restoreGuts(BinStream &stream) {
    int nPacked = 0;
    stream.ReadEndian(&nPacked, sizeof(nPacked));
    BitStream bits(&nPacked, sizeof(nPacked));
    mTrack = static_cast<int>(bits.Unpack(kTrackBits));
    mRepeat = bits.UnpackBool();
    mVersion = static_cast<int>(bits.Unpack(kVersionBits));
}

void RemixerUpdatePacket::PrintExtra(PrnStream &stream) const {
    stream << "track: " << mTrack << " " << "repeat: " << mRepeat << " "
           << "version: " << static_cast<unsigned int>(mVersion) << "\n";
}
