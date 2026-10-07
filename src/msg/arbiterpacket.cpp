#include "msg/arbiterpacket.h"

#include "os/bitstream.h"

namespace {

constexpr int kVersionBits = 14;
constexpr int kCountBits = 2;
constexpr int kNetOrderBits = 3;
constexpr int kPlayerVersionBits = 14;
constexpr int kTrackBits = 3;
constexpr int kSlotBits = 2;
constexpr int kBitsPerByte = 8;
constexpr int kMaxPlayers = 4;

constexpr int kHeaderBits = kVersionBits + kCountBits;
constexpr int kPlayerBits = kNetOrderBits + kPlayerVersionBits + kTrackBits + kSlotBits;
constexpr int kHeaderBytes = kHeaderBits / kBitsPerByte;
constexpr int kBufferBytes = (kHeaderBits + kMaxPlayers * kPlayerBits) / kBitsPerByte;

} // namespace

void ArbiterPacket::saveGuts(BinStream &stream) const {
    unsigned char buffer[kBufferBytes];
    BitStream bits(buffer, kBufferBytes);
    bits.Pack(mVersion, kVersionBits);
    bits.Pack(mPlayers.size() - 1, kCountBits);
    for (const auto &player : mPlayers) {
        bits.Pack(player.mNetOrder, kNetOrderBits);
        bits.Pack(player.mVersion, kPlayerVersionBits);
        bits.Pack(player.mTrack, kTrackBits);
        bits.Pack(player.mSlot, kSlotBits);
    }
    stream.Write(buffer,
                 static_cast<int>(static_cast<unsigned int>(bits.mBitPos - 1) / kBitsPerByte) + 1);
}

void ArbiterPacket::restoreGuts(BinStream &stream) {
    mPlayers.clear();
    unsigned char buffer[kBufferBytes];
    stream.Read(buffer, kHeaderBytes);
    BitStream bits(buffer, kBufferBytes);
    mVersion = static_cast<int>(bits.Unpack(kVersionBits));
    const int nPlayers = static_cast<int>(bits.Unpack(kCountBits)) + 1;
    stream.Read(&buffer[kHeaderBytes], (nPlayers * kPlayerBits - 1) / kBitsPerByte + 1);
    for (int i = 0; i < nPlayers; ++i) {
        PlayerData player;
        player.mNetOrder = static_cast<int>(bits.Unpack(kNetOrderBits));
        player.mVersion = static_cast<int>(bits.Unpack(kPlayerVersionBits));
        player.mTrack = static_cast<int>(bits.Unpack(kTrackBits));
        player.mSlot = static_cast<int>(bits.Unpack(kSlotBits));
        mPlayers.push_back(player);
    }
}

void ArbiterPacket::PrintExtra(PrnStream &stream) const {
    stream << "vers: " << mVersion << "\n";
    for (const auto &player : mPlayers) {
        stream << "player id " << player.mNetOrder << ": track " << player.mTrack << ", slot "
               << player.mSlot << "\n";
    }
}
