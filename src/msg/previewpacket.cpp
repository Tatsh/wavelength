#include "msg/previewpacket.h"

#include "os/bitstream.h"

namespace {

constexpr int kSectionBits = 31;

} // namespace

void PreviewPacket::saveGuts(BinStream &stream) const {
    int nPacked = 0;
    BitStream bits(&nPacked, sizeof(nPacked));
    bits.Pack(mSection, kSectionBits);
    bits.PackBool(mPreviewing);
    int nWord = nPacked;
    stream.WriteEndian(&nWord, sizeof(nWord));
}

void PreviewPacket::restoreGuts(BinStream &stream) {
    int nPacked = 0;
    stream.ReadEndian(&nPacked, sizeof(nPacked));
    BitStream bits(&nPacked, sizeof(nPacked));
    mSection = static_cast<int>(bits.Unpack(kSectionBits));
    mPreviewing = bits.UnpackBool();
}
