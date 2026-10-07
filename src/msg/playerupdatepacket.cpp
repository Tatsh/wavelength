#include "msg/playerupdatepacket.h"

#include "os/bitstream.h"

namespace {

constexpr int kTrackBits = 3;
constexpr int kPointsBits = 14;
constexpr int kVersionBits = 14;

} // namespace

void PlayerUpdatePacket::saveGuts(BinStream &stream) const {
    int nPacked = 0;
    BitStream bits(&nPacked, sizeof(nPacked));
    bits.Pack(mTrack, kTrackBits);
    bits.Pack(mPoints, kPointsBits);
    bits.PackBool(mCatching);
    bits.Pack(mVersion, kVersionBits);
    int nWord = nPacked;
    stream.WriteEndian(&nWord, sizeof(nWord));
}

void PlayerUpdatePacket::restoreGuts(BinStream &stream) {
    int nPacked = 0;
    stream.ReadEndian(&nPacked, sizeof(nPacked));
    BitStream bits(&nPacked, sizeof(nPacked));
    mTrack = static_cast<int>(bits.Unpack(kTrackBits));
    mPoints = static_cast<int>(bits.Unpack(kPointsBits));
    mCatching = bits.UnpackBool();
    mVersion = static_cast<int>(bits.Unpack(kVersionBits));
}

void PlayerUpdatePacket::PrintExtra(PrnStream &stream) const {
    stream << "track: " << mTrack << " " << "points: " << mPoints << " "
           << "catching: " << mCatching << " " << "version: " << static_cast<unsigned int>(mVersion)
           << "\n";
}
