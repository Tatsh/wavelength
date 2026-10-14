#include "game/remixinfo.h"

#include <string.h>

#include "math/rand.h"

namespace {

// The version byte the stream form writes.
constexpr unsigned char kVersion = 5;

// The random half of a key is drawn below this.
constexpr int kKeyRandomLimit = 0x7fffffff;
constexpr int kKeyClockShift = 32;

} // namespace

void RemixInfo::Reset() {
    memset(static_cast<void *>(this), 0, sizeof(*this));
}

int RemixInfo::WireSize() {
    return sizeof(kVersion) + sizeof(RemixInfo);
}

unsigned long long RemixInfo::MakeKey() {
    DateTime now;
    memset(static_cast<void *>(&now), 0, sizeof(now));
    (void)now.ReadClock(); // Yes, the binary discards the result.
    const unsigned long long nClock = static_cast<unsigned long long>(now.ToSeconds())
                                      << kKeyClockShift;
    return nClock | static_cast<unsigned int>(RandomInt(0, kKeyRandomLimit));
}

PrnStream &operator<<(PrnStream &stream, const RemixInfo &info) {
    stream << " name:" << info.mName << " key:" << info.mKey << " song:" << info.mSong
           << " sz:" << info.mDataSize << " " << info.mDate << " skill:" << info.mSkillLevel
           << " playable:" << (info.mPlayable != 0) << " ronly:" << (info.mReadOnly != 0)
           << " bpm:" << info.mTempo;
    switch (info.mSource) {
    case RemixInfo::kSourceNone:
        stream << " none";
        break;
    case RemixInfo::kSourceNet:
        stream << " net";
        break;
    case RemixInfo::kSourceCd:
        stream << " cd";
        break;
    case RemixInfo::kSourceMemcard:
        stream << " memcard";
        break;
    default:
        break;
    }
    return stream;
}

BinStream &operator<<(BinStream &stream, const RemixInfo &info) {
    const unsigned char nVersion = kVersion;
    stream.Write(&nVersion, sizeof(nVersion));
    stream.Write(&info, sizeof(info));
    return stream;
}

BinStream &operator>>(BinStream &stream, RemixInfo &info) {
    unsigned char nVersion;
    stream.Read(&nVersion, sizeof(nVersion));
    stream.Read(&info, sizeof(info));
    return stream;
}
