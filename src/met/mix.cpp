#include "met/mix.h"

#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "os/system.h"
#include "script/dataarray.h"
#include "synth/synth.h"

namespace {

constexpr int kTicksPerBar = 1920;

// The tube sound is two notes on the last MIDI channel.
constexpr unsigned char kTubeNoteOn = 0x9f;
constexpr unsigned char kTubeNoteOff = 0x8f;
constexpr unsigned char kTubeNoteLow = 0x12;
constexpr unsigned char kTubeNoteHigh = 0x13;
constexpr unsigned char kTubeVelocity = 0x40;

constexpr float kFullLevel = 1.0f;

// Pack a message the way Synth::SendPackedMessage() takes it, the status byte lowest.
inline unsigned int PackMessage(unsigned char nStatus, unsigned char nData1, unsigned char nData2) {
    constexpr int kData1Shift = 8;
    constexpr int kData2Shift = 16;
    return nStatus | (static_cast<unsigned int>(nData1) << kData1Shift) |
           (static_cast<unsigned int>(nData2) << kData2Shift);
}

} // namespace

Mix::Mix() {
    mStartTick = -1;
    mLoopTicks = -1;
    mFadeTicks = -1;
    mMixes.assign(kNumMixes, std::vector<String>());
    mMix = kNoMix;
    mStopCmd = Ptr<Command>(NewMemFunCommand(this, &Mix::StopTracks));
    DataArray *pMetagame = SystemConfig()->FindArray("metagame", false);
    int nBars = 0;
    pMetagame->FindInt("music_bars", &nBars, true);
    mLoopTicks = nBars * kTicksPerBar;
    pMetagame->FindInt("music_fade_ticks", &mFadeTicks, true);
}

Mix::~Mix() {
    StopTracks();
    for (const auto &entry : mTracks) {
        delete entry.second;
    }
}

void Mix::AddTrack(Muse *pMuse, unsigned char nChannel, const String &name) {
    MixTrack *pTrack = new MixTrack(pMuse, mLoopTicks, nChannel);
    mTracks[name] = pTrack;
}

void Mix::SetMixTracks(int nMix, const std::vector<String> &names) {
    mMixes[nMix] = names;
}

void Mix::ChangeMix(int nMix) {
    SwitchMix(nMix, mFadeTicks);
}

void Mix::SwitchMix(int nMix, int nFadeTicks) {
    if (nMix == mMix) {
        return;
    }
    if (mMix != kNoMix) {
        FadeOutMix(mMix, nFadeTicks);
    }
    FadeInMix(nMix, nFadeTicks);
    mMix = nMix;
}

void Mix::EnterTube() {
    TheSynth->SendPackedMessage(PackMessage(kTubeNoteOn, kTubeNoteLow, kTubeVelocity));
    TheSynth->SendPackedMessage(PackMessage(kTubeNoteOn, kTubeNoteHigh, kTubeVelocity));
}

void Mix::ExitTube([[maybe_unused]] bool bSpeedingUp) {
    TheSynth->SendPackedMessage(PackMessage(kTubeNoteOff, kTubeNoteLow, kTubeVelocity));
    TheSynth->SendPackedMessage(PackMessage(kTubeNoteOff, kTubeNoteHigh, kTubeVelocity));
}

void Mix::Start(int nMix) {
    const int nTick = TheMetaScheduler.mTick;
    mStartTick = nTick - nTick % kTicksPerBar;
    TheSynth->SetSoftFxSweep(kFullLevel);
    TheSynth->SetOutputLevel(kFullLevel);
    ChangeMix(nMix);
}

void Mix::StopTracks() {
    TheMetaScheduler.Cancel(mStopCmd.Get());
    for (const auto &entry : mTracks) {
        if (entry.second != nullptr) {
            entry.second->StopNow();
        }
    }
}

void Mix::FadeOutMix(int nMix, int nFadeTicks) {
    const std::vector<String> &names = mMixes[nMix];
    for (unsigned int i = 0; i < names.size(); ++i) {
        mTracks[names[i]]->FadeOut(nFadeTicks);
    }
}

void Mix::FadeInMix(int nMix, int nFadeTicks) {
    const std::vector<String> &names = mMixes[nMix];
    for (unsigned int i = 0; i < names.size(); ++i) {
        MixTrack *pTrack = mTracks[names[i]];
        pTrack->FadeIn(nFadeTicks, (TheMetaScheduler.mTick - mStartTick) % mLoopTicks);
    }
}

void Mix::Stop() {
    for (const auto &entry : mTracks) {
        if (entry.second != nullptr) {
            entry.second->FadeOut(kTicksPerBar);
        }
    }
    TheMetaScheduler.PostIn(mStopCmd.Get(), kTicksPerBar, false);
}
