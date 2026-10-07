#include "msg/sectionchangepacket.h"

#include "os/bitstream.h"

namespace {

// The payload word and the bits each field takes in it.
constexpr int kPayloadBytes = 4;
constexpr int kSectionBits = 4;
constexpr int kTickBits = 28;

} // namespace

void SectionChangePacket::saveGuts(OBStream &stream) const {
    unsigned int nPayload;
    BitStream bits(&nPayload, kPayloadBytes);
    bits.Pack(mSection, kSectionBits);
    bits.Pack(mTick, kTickBits);
    const unsigned int nWord = nPayload;
    stream.WriteLE(&nWord, kPayloadBytes);
}

void SectionChangePacket::restoreGuts(IBStream &stream) {
    unsigned int nPayload;
    stream.ReadLE(&nPayload, kPayloadBytes);
    BitStream bits(&nPayload, kPayloadBytes);
    mSection = bits.Unpack(kSectionBits);
    mTick = bits.Unpack(kTickBits);
}
