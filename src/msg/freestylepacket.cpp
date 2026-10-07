#include "msg/freestylepacket.h"

#include "os/binstream.h"
#include "os/bitstream.h"
#include "os/prnstream.h"

namespace {

constexpr int kButtonBits = 2;
constexpr int kXBits = 7;
constexpr int kYBits = 6;

constexpr float kRoundingBias = 0.5f;

} // namespace

int FreestylePacket::Quantize(float fValue, int nBits) {
    const int nMax = (1 << (nBits - 1)) - 1;
    return static_cast<int>((fValue * static_cast<float>(nMax)) + kRoundingBias);
}

float FreestylePacket::Dequantize(int nValue, int nBits) {
    const int nMax = (1 << (nBits - 1)) - 1;
    return static_cast<float>(nValue) / static_cast<float>(nMax);
}

void FreestylePacket::saveGuts(BinStream &stream) const {
    unsigned short nPacked;
    BitStream bits(&nPacked, sizeof(nPacked));
    bits.PackBool(mActive);
    bits.Pack(mButton, kButtonBits);
    const int nX = Quantize(mX, kXBits);
    const int nY = Quantize(mY, kYBits);
    bits.PackSigned(nX, kXBits);
    bits.PackSigned(nY, kYBits);
    unsigned short nHalfWord = nPacked;
    stream.WriteEndian(&nHalfWord, sizeof(nHalfWord));
}

void FreestylePacket::restoreGuts(BinStream &stream) {
    unsigned short nPacked;
    stream.ReadEndian(&nPacked, sizeof(nPacked));
    BitStream bits(&nPacked, sizeof(nPacked));
    mActive = bits.UnpackBool();
    mButton = static_cast<int>(bits.Unpack(kButtonBits));
    const int nX = bits.UnpackSigned(kXBits);
    const int nY = bits.UnpackSigned(kYBits);
    mX = Dequantize(nX, kXBits);
    mY = Dequantize(nY, kYBits);
}

void FreestylePacket::PrintExtra(PrnStream &stream) const {
    stream << "active: " << mActive << " " << "button: " << mButton << " " << "x: " << mX << " "
           << "y: " << mY << "\n";
}
