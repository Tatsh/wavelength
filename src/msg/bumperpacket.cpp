#include "msg/bumperpacket.h"

#include "os/bitstream.h"

namespace {

constexpr int kCountBits = 2;
constexpr int kNetOrderBits = 3;
constexpr int kTrackBits = 3;
constexpr int kBitsPerByte = 8;
constexpr int kMaxVictims = 3;

constexpr int kVictimBits = kNetOrderBits + kTrackBits;
constexpr int kHeaderBytes = 1;
constexpr int kBufferBytes =
    (kCountBits + kMaxVictims * kVictimBits + kBitsPerByte - 1) / kBitsPerByte;

} // namespace

void BumperPacket::saveGuts(BinStream &stream) const {
    unsigned char buffer[kBufferBytes];
    BitStream bits(buffer, kBufferBytes);
    bits.Pack(mVictims.size(), kCountBits);
    for (const auto &victim : mVictims) {
        bits.Pack(victim.mNetOrder, kNetOrderBits);
        bits.Pack(victim.mTrack, kTrackBits);
    }
    stream.Write(buffer,
                 static_cast<int>(static_cast<unsigned int>(bits.mBitPos - 1) / kBitsPerByte) + 1);
}

void BumperPacket::restoreGuts(BinStream &stream) {
    mVictims.clear();
    unsigned char buffer[kBufferBytes];
    stream.Read(buffer, kHeaderBytes);
    BitStream bits(buffer, kBufferBytes);
    const int nVictims = static_cast<int>(bits.Unpack(kCountBits));
    stream.Read(&buffer[kHeaderBytes], (kCountBits + nVictims * kVictimBits - 1) / kBitsPerByte);
    for (int i = 0; i < nVictims; ++i) {
        VictimData victim;
        victim.mNetOrder = static_cast<int>(bits.Unpack(kNetOrderBits));
        victim.mTrack = static_cast<int>(bits.Unpack(kTrackBits));
        mVictims.push_back(victim);
    }
}

void BumperPacket::PrintExtra(PrnStream &stream) const {
    for (const auto &victim : mVictims) {
        stream << "player: " << victim.mNetOrder << " " << "track: " << victim.mTrack << "\n";
    }
}
