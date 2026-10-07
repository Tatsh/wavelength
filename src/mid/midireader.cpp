#include "mid/midireader.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "os/bufstream.h"
#include "os/debug.h"
#include "os/filestream.h"
#include "os/system.h"

namespace {

constexpr short kTargetDivision = 480;
constexpr int kNoTick = -1;

// The status bytes and their parts.
constexpr unsigned char kStatusBit = 0x80;
constexpr unsigned char kTypeMask = 0xf0;
constexpr unsigned char kChannelMask = 0x0f;
constexpr unsigned char kVarLenMore = 0x80;
constexpr unsigned char kVarLenMask = 0x7f;
constexpr int kVarLenBits = 7;
constexpr unsigned char kNoteOff = 0x80;
constexpr unsigned char kNoteOn = 0x90;
constexpr unsigned char kKeyPressure = 0xa0;
constexpr unsigned char kControlChange = 0xb0;
constexpr unsigned char kProgramChange = 0xc0;
constexpr unsigned char kChannelPressure = 0xd0;
constexpr unsigned char kPitchBend = 0xe0;
constexpr unsigned char kSystem = 0xf0;
constexpr unsigned char kSysex = 0xf0;
constexpr unsigned char kSysexContinue = 0xf7;
constexpr unsigned char kMeta = 0xff;

// The meta types the reader reports or skips.
enum MetaType {
    kMetaFirstText = 0x01,
    kMetaLastText = 0x07,
    kMetaChannelPrefix = 0x20,
    kMetaPortPrefix = 0x21,
    kMetaEndOfTrack = 0x2f,
    kMetaTempo = 0x51,
    kMetaSmpteOffset = 0x54,
    kMetaTimeSignature = 0x58,
    kMetaKeySignature = 0x59,
};

// The rank CompareMidi() gives each kind of channel event.
enum MidiRank {
    kRankNoteOff = 1,
    kRankControlChange = 2,
    kRankProgramChange = 3,
    kRankChannelPressure = 4,
    kRankPitchBend = 5,
    kRankKeyPressure = 6,
    kRankNoteOn = 7,
    kRankOther = 8,
};

constexpr int kTempoHighShift = 16;
constexpr int kTempoMiddleShift = 8;
constexpr float kTimeSignatureBase = 2.0f;
constexpr int kTimeSignatureSkipBytes = 2;
constexpr int kChunkLengthBytes = 4;
constexpr int kHeaderWordBytes = 2;

const char *const kHeaderChunk = "MThd";
const char *const kTrackChunk = "MTrk";

int Rank(unsigned char nStatus) {
    switch (nStatus & kTypeMask) {
    case kNoteOff:
        return kRankNoteOff;
    case kControlChange:
        return kRankControlChange;
    case kProgramChange:
        return kRankProgramChange;
    case kChannelPressure:
        return kRankChannelPressure;
    case kPitchBend:
        return kRankPitchBend;
    case kKeyPressure:
        return kRankKeyPressure;
    case kNoteOn:
        return kRankNoteOn;
    default:
        return kRankOther;
    }
}

} // namespace

BinStream &MidiReader::ChunkName::Read(BinStream &stream) {
    stream.Read(mName, kLength);
    return stream;
}

MidiReader::VarLen::VarLen(BinStream &stream) {
    Read(stream);
}

BinStream &MidiReader::VarLen::Read(BinStream &stream) {
    mValue = 0;
    unsigned char nByte;
    do {
        nByte = 0;
        stream.Read(&nByte, sizeof(nByte));
        mValue = (mValue << kVarLenBits) + (nByte & kVarLenMask);
    } while (nByte & kVarLenMore);
    return stream;
}

bool MidiReader::CompareMidi(const Midi &left, const Midi &right) {
    return Rank(left.mStatus) < Rank(right.mStatus);
}

MidiReader::MidiReader(const char *pszFile, MidiReceiver *pReceiver)
    : mStream(new FileStream(pszFile, false, false, 0)), mReceiver(pReceiver), mState(kStateHeader),
      mNumTracks(0), mDivision(0), mTargetDivision(kTargetDivision), mTrackIndex(0), mTrackTick(0),
      mRunningStatus(0), mPending(), mPendingTick(0), mCompare(&MidiReader::CompareMidi) {
}

MidiReader::MidiReader(const char *pBuffer, int nSize, MidiReceiver *pReceiver)
    : mStream(new BufStream(pBuffer, nSize, false)), mReceiver(pReceiver), mState(kStateHeader),
      mNumTracks(0), mDivision(0), mTargetDivision(kTargetDivision), mTrackIndex(0), mTrackTick(0),
      mRunningStatus(0), mPending(), mPendingTick(0), mCompare(&MidiReader::CompareMidi) {
}

MidiReader::~MidiReader() {
    delete mStream;
}

void MidiReader::ReadAll() {
    while (ReadTrack()) {
    }
}

bool MidiReader::ReadTrack() {
    do {
        Step();
    } while (mState != kStateDone && mState != kStateTrackStart);
    return mState == kStateTrackStart;
}

bool MidiReader::ReadSteps(int nSteps) {
    for (int i = 0; i < nSteps; ++i) {
        if (mState == kStateDone) {
            return false;
        }
        Step();
    }
    return true;
}

bool MidiReader::ReadFor(float fMs) {
    const float fEnd = SystemMs() + fMs;
    while (SystemMs() < fEnd) {
        if (mState == kStateDone) {
            return false;
        }
        Step();
    }
    return true;
}

void MidiReader::Step() {
    switch (mState) {
    case kStateHeader:
        ReadHeader(*mStream);
        break;
    case kStateTrackStart:
        BeginTrack(*mStream);
        break;
    case kStateEvent:
        ReadEvent(*mStream);
        break;
    default:
        break;
    }
}

void MidiReader::ReadHeader(BinStream &stream) {
    ChunkName name;
    name.Read(stream);
    int nLength = 0;
    stream.ReadEndian(&nLength, kChunkLengthBytes);
    (void)strncmp(name.mName, kHeaderChunk, ChunkName::kLength); // The binary discards the result.
    short nFormat;
    stream.ReadEndian(&nFormat, kHeaderWordBytes);
    stream.ReadEndian(&mNumTracks, kHeaderWordBytes);
    stream.ReadEndian(&mDivision, kHeaderWordBytes);
    if (mNumTracks == 0) {
        mReceiver->OnAllDone();
        mState = kStateDone;
    } else {
        mState = kStateTrackStart;
    }
}

void MidiReader::BeginTrack(BinStream &stream) {
    ChunkName name;
    name.Read(stream);
    int nLength = 0;
    stream.ReadEndian(&nLength, kChunkLengthBytes);
    (void)strncmp(name.mName, kTrackChunk, ChunkName::kLength); // The binary discards the result.
    mReceiver->OnNewTrack(static_cast<unsigned char>(mTrackIndex));
    mPendingTick = kNoTick;
    mState = kStateEvent;
    ++mTrackIndex;
    mRunningStatus = 0;
    mTrackTick = 0;
}

void MidiReader::ReadEvent(BinStream &stream) {
    const VarLen delta(stream);
    mTrackTick += delta.mValue;
    const int nTick = mTrackTick * mTargetDivision / mDivision;
    if (nTick != mPendingTick) {
        Flush();
        mPendingTick = nTick;
    }
    unsigned char bytes[2];
    stream.Read(&bytes[0], 1);
    bool bRunning = false;
    if (bytes[0] & kStatusBit) {
        if ((bytes[0] & kTypeMask) != kSystem) {
            mRunningStatus = bytes[0];
        }
    } else {
        bRunning = true;
        bytes[1] = bytes[0];
        bytes[0] = mRunningStatus;
    }
    const unsigned char nStatus = bytes[0];
    if ((nStatus & kTypeMask) == kSystem) {
        ReadSystemEvent(nTick, nStatus, stream);
        return;
    }
    if (!bRunning) {
        stream.Read(&bytes[1], 1);
    }
    ReadChannelEvent(nTick, bytes[0], bytes[1], stream);
}

void MidiReader::ReadChannelEvent(int nTick,
                                  unsigned char nStatus,
                                  unsigned char nData1,
                                  BinStream &stream) {
    unsigned char nData2 = 0;
    switch (nStatus & kTypeMask) {
    case kNoteOn:
        stream.Read(&nData2, sizeof(nData2));
        if (nData2 == 0) {
            nStatus = (nStatus & kChannelMask) | kNoteOff;
        }
        break;
    case kNoteOff:
    case kKeyPressure:
    case kControlChange:
    case kPitchBend:
        stream.Read(&nData2, sizeof(nData2));
        break;
    case kProgramChange:
    case kChannelPressure:
        nData2 = 0;
        break;
    default:
        DebugWarn("don't know how to parse event %i", nStatus & kTypeMask);
        break;
    }
    AddMidi(nTick, nStatus, nData1, nData2);
}

void MidiReader::ReadSystemEvent(int nTick, unsigned char nStatus, BinStream &stream) {
    switch (nStatus) {
    case kSysex:
    case kSysexContinue: {
        // The binary reads the length and does not skip the data.
        const VarLen length(stream);
        break;
    }
    case kMeta: {
        unsigned char nType;
        stream.Read(&nType, sizeof(nType));
        ReadMeta(nTick, nType, stream);
        break;
    }
    default:
        DebugWarn("don't know how to parse system event %i", nStatus);
        break;
    }
}

void MidiReader::ReadMeta(int nTick, unsigned char nType, BinStream &stream) {
    const VarLen length(stream);
    const int nStart = stream.Tell();
    if (nType >= kMetaFirstText && nType <= kMetaLastText) {
        char *pszText = new char[length.mValue + 1];
        stream.Read(pszText, length.mValue);
        pszText[length.mValue] = '\0';
        mReceiver->OnText(nTick, pszText, nType);
        delete[] pszText;
    } else {
        switch (nType) {
        case kMetaTempo: {
            unsigned char tempo[3];
            stream.Read(&tempo[0], 1);
            stream.Read(&tempo[1], 1);
            stream.Read(&tempo[2], 1);
            mReceiver->OnTempo(nTick,
                               ((tempo[0] << kTempoHighShift) + (tempo[1] << kTempoMiddleShift)) |
                                   tempo[2]);
            break;
        }
        case kMetaEndOfTrack:
            Flush();
            mPendingTick = kNoTick;
            mReceiver->OnEndTrack();
            if (mTrackIndex == mNumTracks) {
                mReceiver->OnAllDone();
                mState = kStateDone;
            } else {
                mState = kStateTrackStart;
            }
            break;
        case kMetaTimeSignature: {
            unsigned char nNumerator;
            unsigned char nDenominator;
            stream.Read(&nNumerator, sizeof(nNumerator));
            stream.Read(&nDenominator, sizeof(nDenominator));
            mReceiver->OnTimeSignature(
                nTick,
                nNumerator,
                static_cast<int>(std::pow(kTimeSignatureBase, static_cast<float>(nDenominator))));
            stream.Seek(kTimeSignatureSkipBytes, BinStream::kSeekCurrent);
            break;
        }
        case kMetaChannelPrefix:
        case kMetaPortPrefix:
        case kMetaSmpteOffset:
        case kMetaKeySignature:
            break;
        default:
            DebugWarn("don't know how to parse meta event %i", nType);
            break;
        }
    }
    stream.Seek(nStart + length.mValue, BinStream::kSeekBegin);
}

void MidiReader::AddMidi(int nTick,
                         unsigned char nStatus,
                         unsigned char nData1,
                         unsigned char nData2) {
    if (mCompare == nullptr) {
        mReceiver->OnMidi(nTick, nStatus, nData1, nData2);
        return;
    }
    Midi midi;
    memset(&midi, 0, sizeof(midi));
    midi.mStatus = nStatus;
    midi.mData1 = nData1;
    midi.mData2 = nData2;
    mPending.push_back(midi);
}

void MidiReader::Flush() {
    if (mPending.empty()) {
        return;
    }
    std::sort(mPending.begin(), mPending.end(), mCompare);
    for (const Midi &midi : mPending) {
        mReceiver->OnMidi(mPendingTick, midi.mStatus, midi.mData1, midi.mData2);
    }
    mPending.clear();
}
