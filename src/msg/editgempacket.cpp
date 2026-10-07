#include "msg/editgempacket.h"

#include "os/bitstream.h"

namespace {

constexpr int kPayloadBytes = 4;
constexpr int kTrackBits = 3;
constexpr int kSlotBits = 2;
constexpr int kTickBits = 26;

} // namespace

void EditGemPacket::saveGuts(BinStream &stream) const {
    unsigned int nPayload;
    BitStream bits(&nPayload, kPayloadBytes);
    bits.Pack(mTrack, kTrackBits);
    bits.Pack(mSlot, kSlotBits);
    bits.Pack(mTick, kTickBits);
    bits.PackBool(mRepeat);
    const unsigned int nWord = nPayload;
    stream.WriteEndian(&nWord, kPayloadBytes);
}

void EditGemPacket::restoreGuts(BinStream &stream) {
    unsigned int nPayload;
    stream.ReadEndian(&nPayload, kPayloadBytes);
    BitStream bits(&nPayload, kPayloadBytes);
    mTrack = static_cast<int>(bits.Unpack(kTrackBits));
    mSlot = static_cast<int>(bits.Unpack(kSlotBits));
    mTick = static_cast<int>(bits.Unpack(kTickBits));
    mRepeat = bits.UnpackBool();
}
