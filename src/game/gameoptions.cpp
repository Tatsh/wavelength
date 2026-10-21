#include "game/gameoptions.h"

#include <string.h>

#include "os/debug.h"
#include "synth/synth.h"

namespace {

// The version byte Save() writes.
constexpr unsigned char kVersion = 3;

// The first version with the output mode as a word, and the first with the date.
constexpr unsigned char kVersionOutputModeWord = 3;
constexpr unsigned char kVersionDate = 2;

#ifdef VIDEO_STANDARD_PAL
// The bytes Save() writes, the version byte, the output mode, the two flags, the track size, and
// the six bytes of the date.
constexpr int kSavedSize = 17;
#endif

} // namespace

GameOptions::GameOptions() {
    mFreqSize = kFreqSizeLarge;
    mForceFeedback = 1;
    mOutputMode = kOutputStereo;
    mHelpText = 1;
    memset(static_cast<void *>(&mDate), 0, sizeof(mDate));
    mModified = 0;
}

void GameOptions::SetFreqSize(int nFreqSize) {
    mFreqSize = nFreqSize;
    mModified = 1;
}

void GameOptions::SetOutputMode(int nOutputMode) {
    mOutputMode = nOutputMode;
    mModified = 1;
}

void GameOptions::SetHelpText(int nHelpText) {
    mHelpText = nHelpText;
    mModified = 1;
}

void GameOptions::SetForceFeedback(int nForceFeedback) {
    mForceFeedback = nForceFeedback;
    mModified = 1;
}

void GameOptions::RecordUpload() {
    (void)mDate.ReadClock(); // Yes, the binary discards the result.
    mModified = 1;
}

bool GameOptions::UploadedToday() const {
    DateTime now;
    memset(static_cast<void *>(&now), 0, sizeof(now));
    (void)now.ReadClock(); // Yes, the binary discards the result.
    return (mDate.GetDay() == now.GetDay()) && (mDate.GetMonth() == now.GetMonth()) &&
           (mDate.GetYear() == now.GetYear());
}

void GameOptions::ApplyOutputMode() {
    switch (mOutputMode) {
    case kOutputMono:
        TheSynth->SetMono(true);
        TheSynth->SetSurround(false);
        break;
    case kOutputStereo:
        TheSynth->SetMono(false);
        TheSynth->SetSurround(false);
        break;
    case kOutputSurround:
        TheSynth->SetMono(false);
        TheSynth->SetSurround(true);
        break;
    default:
        DebugWarn(" Bad audio setting!!!");
        break;
    }
}

void GameOptions::Save(BinStream &stream) const {
    const unsigned char nVersion = kVersion;
    stream.Write(&nVersion, sizeof(nVersion));
    stream.WriteEndian(&mOutputMode, sizeof(mOutputMode));
    unsigned char nByte = static_cast<unsigned char>(mHelpText);
    stream.Write(&nByte, sizeof(nByte));
    nByte = static_cast<unsigned char>(mForceFeedback);
    stream.Write(&nByte, sizeof(nByte));
    stream.WriteEndian(&mFreqSize, sizeof(mFreqSize));
    stream << mDate;
}

#ifdef VIDEO_STANDARD_PAL
int GameOptions::SavedSize() {
    return kSavedSize;
}
#endif

void GameOptions::Load(BinStream &stream) {
    unsigned char nVersion;
    stream.Read(&nVersion, sizeof(nVersion));
    unsigned char nByte;
    if (nVersion < kVersionOutputModeWord) {
        stream.Read(&nByte, sizeof(nByte));
        mOutputMode = kOutputStereo; // Yes, the binary ignores the byte it read.
    } else {
        int nOutputMode;
        stream.ReadEndian(&nOutputMode, sizeof(nOutputMode));
        mOutputMode = nOutputMode;
    }
    if (nVersion != 0) {
        stream.Read(&nByte, sizeof(nByte));
        mHelpText = (nByte != 0);
        stream.Read(&nByte, sizeof(nByte));
        mForceFeedback = (nByte != 0);
        int nFreqSize;
        stream.ReadEndian(&nFreqSize, sizeof(nFreqSize));
        mFreqSize = nFreqSize;
    }
    if (nVersion >= kVersionDate) {
        stream >> mDate;
    }
}

BinStream &operator<<(BinStream &stream, const GameOptions &options) {
    options.Save(stream);
    return stream;
}

BinStream &operator>>(BinStream &stream, GameOptions &options) {
    options.Load(stream);
    return stream;
}
