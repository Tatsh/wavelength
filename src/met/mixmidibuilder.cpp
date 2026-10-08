#include "met/mixmidibuilder.h"

#include "os/mem.h"
#include "os/system.h"

namespace {

constexpr char kMixTag[] = "mix";
constexpr char kMetagameTag[] = "metagame";
constexpr char kMidiFileTag[] = "music_shared_midi_file";
constexpr char kBufferTag[] = "MixMidiBuilder buf";
constexpr char kNoTrackName[] = "";

constexpr int kNoTrack = -1;
constexpr float kUnknownBeatLength = -1.0f;
constexpr int kUnknownBarLength = -1;

constexpr int kOpenRead = 1;
constexpr int kNoOpenFlags = 0;
constexpr int kDefaultAlignment = 0;

// The events MidiReader parses on each Poll().
constexpr int kStepsPerPoll = 1000;

constexpr unsigned char kTrackNameMetaEvent = 3;
constexpr unsigned char kChannelMask = 0xf;
constexpr float kMicrosecondsPerMillisecond = 1000.0f;
constexpr int kTicksPerBeat = 480;

} // namespace

MixMidiBuilder::MixMidiBuilder(DataArray *pConfig)
    : mTracks(), mBuffer(nullptr), mSize(0), mFile(nullptr), mReserved1c(), mReadDone(0), mDone(0),
      mReader(nullptr), mNames(), mReceiver(new BuilderReceiver(this)), mReserved50(0),
      mTrackIndex(kNoTrack), mTrackName() {
    mMuseBuilder = nullptr;
    mHasTrack = 0;
    mMillisecondsPerBeat = kUnknownBeatLength;
    mTicksPerBar = kUnknownBarLength;
    DataArray *pMixes = pConfig->FindArray(kMixTag, true);
    for (int i = 1; i < pMixes->Size(); ++i) {
        DataArray *pMix = pMixes->Array(i);
        for (int j = 1; j < pMix->Size(); ++j) {
            mNames.insert(String(pMix->Sym(j)));
        }
    }
}

MixMidiBuilder::~MixMidiBuilder() {
    PoolMemFree(mBuffer);
    delete mFile;
    delete mReceiver;
    delete mMuseBuilder;
    delete mReader;
}

void MixMidiBuilder::StartLoad() {
    const char *pszFile = nullptr;
    SystemConfig()->FindArray(kMetagameTag, false)->FindSymbol(kMidiFileTag, &pszFile, true);
    mFile = File::New(pszFile, kOpenRead, kNoOpenFlags);
    mSize = mFile->Size();
    mBuffer = static_cast<char *>(PoolMemAlloc(mSize, kBufferTag, kDefaultAlignment));
    mFile->ReadAsync(mBuffer, mSize);
    mReadDone = 0;
    mDone = 0;
    mTrackIndex = 0;
    mTrackName = kNoTrackName;
    mTicksPerBar = kUnknownBarLength;
}

void MixMidiBuilder::Poll() {
    if (!mReadDone) {
        int nBytes;
        mReadDone = mFile->ReadDone(&nBytes);
        return;
    }
    if (mDone) {
        return;
    }
    if (mReader == nullptr) {
        mReader = new MidiReader(mBuffer, mSize, mReceiver);
        return;
    }
    mDone = !mReader->ReadSteps(kStepsPerPoll);
    if (!mDone) {
        return;
    }
    delete mReader;
    mReader = nullptr;
    delete mFile;
    mFile = nullptr;
    if (mBuffer != nullptr) {
        PoolMemFree(mBuffer);
        mBuffer = nullptr;
    }
}

bool MixMidiBuilder::IsDone() const {
    return mDone != 0;
}

int MixMidiBuilder::NumTracks() const {
    return static_cast<int>(mTracks.size());
}

Muse *MixMidiBuilder::TrackMuse(int nTrack) const {
    return mTracks[nTrack].mMuse.Get();
}

unsigned char MixMidiBuilder::TrackChannel(int nTrack) const {
    return mTracks[nTrack].mChannel;
}

const String &MixMidiBuilder::TrackName(int nTrack) const {
    return mTracks[nTrack].mName;
}

void MixMidiBuilder::OnNewTrack([[maybe_unused]] unsigned char nTrack) {
    mTrackName = kNoTrackName;
}

void MixMidiBuilder::OnEndTrack() {
    if (mMuseBuilder != nullptr) {
        mMuseBuilder->OnEndTrack();
    }
    if (mHasTrack) {
        mTracks.back().mName = mTrackName;
    }
    ++mTrackIndex;
    delete mMuseBuilder;
    mMuseBuilder = nullptr;
}

void MixMidiBuilder::OnAllDone() {
}

void MixMidiBuilder::OnMidi(int nTick,
                            unsigned char nStatus,
                            unsigned char nData1,
                            unsigned char nData2) {
    if (mHasTrack) {
        mTracks.back().mChannel = nStatus & kChannelMask;
    }
    if (mMuseBuilder != nullptr) {
        mMuseBuilder->OnMidi(nTick, nStatus, nData1, nData2);
    }
}

void MixMidiBuilder::OnTempo(int nTick, int nMicrosecondsPerBeat) {
    mMillisecondsPerBeat = static_cast<float>(nMicrosecondsPerBeat) / kMicrosecondsPerMillisecond;
    if (mMuseBuilder != nullptr) {
        mMuseBuilder->OnTempo(nTick, nMicrosecondsPerBeat);
    }
}

void MixMidiBuilder::OnText(int nTick, const char *pszText, unsigned char nType) {
    if (nType == kTrackNameMetaEvent) {
        mTrackName = pszText;
        mHasTrack = 0;
        if (mTrackIndex > 0) {
            if (mNames.find(String(pszText)) != mNames.end()) {
                mTracks.push_back(TrackData());
                mMuseBuilder = new MuseBuilder(mTrackIndex, false, nullptr, &mTracks.back().mMuse);
                mHasTrack = 1;
            }
        }
    }
    if (mMuseBuilder != nullptr) {
        mMuseBuilder->OnText(nTick, pszText, nType);
    }
}

void MixMidiBuilder::OnTimeSignature(int nTick, int nNumerator, int nDenominator) {
    mTicksPerBar = nNumerator * kTicksPerBeat;
    if (mMuseBuilder != nullptr) {
        mMuseBuilder->OnTimeSignature(nTick, nNumerator, nDenominator);
    }
}
