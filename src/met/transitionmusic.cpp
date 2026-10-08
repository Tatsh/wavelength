#include "met/transitionmusic.h"

#include <vector>

#include "gs/muse.h"
#include "gs/musebuilder.h"
#include "met/mixtrack.h"
#include "mid/midireader.h"
#include "mid/midireceiver.h"
#include "os/ptr.h"
#include "os/ramp.h"
#include "os/scheduler.h"
#include "os/string.h"
#include "os/system.h"
#include "script/dataarray.h"
#include "synth/synth.h"

namespace {

constexpr int kTicksPerBar = 1920;
constexpr int kNoChannel = -1;
constexpr int kNoBars = -1;
constexpr unsigned char kMidiChannelMask = 0xf;
constexpr unsigned char kMetaTrackName = 3;

constexpr char kTransition1[] = "TRANSITION 1";
constexpr char kTransition3[] = "TRANSITION 3";
constexpr char kTransition4[] = "TRANSITION 4";

constexpr int kFadeTicks = 1440;
constexpr int kLagMoveTicks = 3000;
constexpr int kLagStepTicks = 20;
constexpr float kFadedInLag = 100.0f;

// Ramp of the lag of the sound output.
class LagRamp : public Ramp {
public:
    LagRamp() : Ramp(&TheMetaScheduler, 0.0f) {
    }

    // NTSC-U/C: 0x0035e6d8, PAL: 0x003cc7b8
    ~LagRamp() override {
    }

    // NTSC-U/C: 0x0035e748, PAL: 0x003cc828
    void Apply(float fValue, [[maybe_unused]] int nTick) override {
        TheSynth->SetLag(fValue);
    }
};

// Receiver of the shared MIDI file that makes a MixTrack of each transition track.
class TransitionBuilder : public MidiReceiver {
public:
    // NTSC-U/C: 0x00196be8, PAL: 0x0019e108
    explicit TransitionBuilder(const char *pszFile);

    // NTSC-U/C: 0x00196c40, PAL: 0x0019e160
    ~TransitionBuilder() override;

    // NTSC-U/C: 0x00196cc0, PAL: 0x0019e1e0
    void Read();

    // NTSC-U/C: 0x0035e668, PAL: 0x003cc748
    void OnNewTrack([[maybe_unused]] unsigned char nTrack) override {
    }

    // NTSC-U/C: 0x00196d38, PAL: 0x0019e258
    void OnEndTrack() override;

    // NTSC-U/C: 0x0035e670, PAL: 0x003cc750
    void OnAllDone() override {
    }

    // NTSC-U/C: 0x00197010, PAL: 0x0019e530
    void
    OnMidi(int nTick, unsigned char nStatus, unsigned char nData1, unsigned char nData2) override;

    // NTSC-U/C: 0x0035e678, PAL: 0x003cc758
    void OnTempo([[maybe_unused]] int nTick, [[maybe_unused]] int nMicrosecondsPerBeat) override {
    }

    // NTSC-U/C: 0x00196f58, PAL: 0x0019e478
    void OnText(int nTick, const char *pszText, unsigned char nType) override;

    // NTSC-U/C: 0x0035e680, PAL: 0x003cc760
    void OnTimeSignature([[maybe_unused]] int nTick,
                         [[maybe_unused]] int nNumerator,
                         [[maybe_unused]] int nDenominator) override {
    }

private:
    const char *mFile;     // The shared MIDI file.
    MuseBuilder *mBuilder; // The builder of the current transition track, or null.
    Ptr<Muse> mMuse;       // The piece the builder stores.
    int mChannel;          // The MIDI channel of the current transition track, or kNoChannel.
    int mBars;             // `music_bars`, the bars of a loop.
    int mTrack;            // The index of the track the reader is in.
};

// NTSC-U/C: 0x003af8b0
LagRamp *sLagRamp;

// NTSC-U/C: 0x004368b8
std::vector<MixTrack *> sTracks;

TransitionBuilder::TransitionBuilder(const char *pszFile) : mFile(pszFile), mBuilder(nullptr) {
    mBars = kNoBars;
    mChannel = kNoChannel;
    mTrack = 0;
}

TransitionBuilder::~TransitionBuilder() {
    delete mBuilder;
}

void TransitionBuilder::Read() {
    SystemConfig()->FindArray("metagame", false)->FindInt("music_bars", &mBars, true);
    MidiReader reader(mFile, this);
    reader.ReadAll();
}

void TransitionBuilder::OnEndTrack() {
    if (mBuilder != nullptr) {
        mBuilder->OnEndTrack();
        MixTrack *pTrack =
            new MixTrack(mMuse.Get(), mBars * kTicksPerBar, static_cast<unsigned char>(mChannel));
        sTracks.push_back(pTrack);
        delete mBuilder;
    }
    mBuilder = nullptr;
    ++mTrack;
    mMuse = Ptr<Muse>();
    mChannel = kNoChannel;
}

void TransitionBuilder::OnMidi(int nTick,
                               unsigned char nStatus,
                               unsigned char nData1,
                               unsigned char nData2) {
    if (mBuilder == nullptr) {
        return;
    }
    if (mChannel == kNoChannel) {
        mChannel = nStatus & kMidiChannelMask;
    }
    mBuilder->OnMidi(nTick, nStatus, nData1, nData2);
}

void TransitionBuilder::OnText([[maybe_unused]] int nTick,
                               const char *pszText,
                               unsigned char nType) {
    if (mTrack == 0 || nType != kMetaTrackName) {
        return;
    }
    String name(pszText);
    if (name == kTransition1 || name == kTransition3 || name == kTransition4) {
        mBuilder = new MuseBuilder(0, false, nullptr, &mMuse);
    }
}

} // namespace

void LoadSharedMusic() {
    sLagRamp = new LagRamp;
    const char *pszFile = nullptr;
    SystemConfig()
        ->FindArray("metagame", true)
        ->FindSymbol("music_shared_midi_file", &pszFile, true);
    TransitionBuilder builder(pszFile);
    builder.Read();
}

void UnloadSharedMusic() {
    delete sLagRamp;
    sLagRamp = nullptr;
    for (unsigned int i = 0; i < sTracks.size(); ++i) {
        delete sTracks[i];
    }
    sTracks.clear();
}

void FadeInSharedMusic() {
    sLagRamp->Move(kLagMoveTicks, kLagStepTicks, TheSynth->GetLag(), kFadedInLag);
    for (unsigned int i = 0; i < sTracks.size(); ++i) {
        sTracks[i]->FadeIn(kFadeTicks, 0);
    }
}

void FadeOutSharedMusic() {
    float fLag = 0.0f;
    SystemConfig()->FindArray("synth", true)->FindFloat("lag_ms", &fLag, true);
    sLagRamp->Move(kLagMoveTicks, kLagStepTicks, TheSynth->GetLag(), fLag);
    for (unsigned int i = 0; i < sTracks.size(); ++i) {
        sTracks[i]->FadeOut(kFadeTicks);
    }
}

bool IsSharedMusicFading() {
    for (unsigned int i = 0; i < sTracks.size(); ++i) {
        if (sTracks[i]->IsPlaying()) {
            return true;
        }
    }
    return false;
}
