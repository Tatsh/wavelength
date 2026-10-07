#include "met/mixtrack.h"

#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "os/system.h"
#include "script/dataarray.h"
#include "synth/synth.h"

namespace {

// The ticks between two steps of a volume ramp.
constexpr int kRampStepTicks = 50;

constexpr float kFullVolume = 1.0f;
constexpr float kMidiMaxValue = 127.0f;
constexpr unsigned char kMidiControlChange = 0xb0;
constexpr unsigned char kMidiControlExpression = 0x11;

} // namespace

MixTrack::VolumeRamp::VolumeRamp(MixTrack *pOwner) : Ramp(&TheMetaScheduler, 0.0f), mOwner(pOwner) {
}

MixTrack::VolumeRamp::~VolumeRamp() {
}

void MixTrack::VolumeRamp::Apply(float fValue, [[maybe_unused]] int nTick) {
    mOwner->SetVolume(fValue);
}

MixTrack::MixTrack(Muse *pMuse, int nLength, unsigned char nChannel) : MuseLooper(pMuse, nLength) {
    VolumeRamp *pRamp = new VolumeRamp(this);
    mChannel = nChannel;
    mRamp = pRamp;
    mMaxVolume = kFullVolume;
    mStopCmd = Ptr<Command>(NewMemFunCommand(static_cast<MuseLooper *>(this), &MuseLooper::Stop));
    SystemConfig()->FindArray("metagame", true)->FindFloat("music_max_volume", &mMaxVolume, true);
}

MixTrack::~MixTrack() {
    StopNow();
    delete mRamp;
}

void MixTrack::FadeIn(int nFadeTicks, int nPosition) {
    TheMetaScheduler.Cancel(mStopCmd.Get());
    RampTo(mMaxVolume, nFadeTicks);
    Play(&TheMetaScheduler, nPosition);
}

void MixTrack::FadeOut(int nFadeTicks) {
    if (MuseLooper::IsPlaying()) {
        RampTo(0.0f, nFadeTicks);
        TheMetaScheduler.PostIn(mStopCmd.Get(), nFadeTicks, false);
    }
}

void MixTrack::StopNow() {
    Stop();
    mRamp->Stop();
    TheMetaScheduler.Cancel(mStopCmd.Get());
    SetVolume(kFullVolume);
}

bool MixTrack::IsPlaying() const {
    return MuseLooper::IsPlaying();
}

void MixTrack::SetVolume(float fVolume) {
    if (kFullVolume < fVolume) {
        fVolume = kFullVolume;
    } else if (fVolume < 0.0f) {
        fVolume = 0.0f;
    }
    TheSynth->SendMessage(kMidiControlChange | mChannel,
                          kMidiControlExpression,
                          static_cast<unsigned char>(static_cast<int>(fVolume * kMidiMaxValue)),
                          0);
}

void MixTrack::RampTo(float fTarget, int nTicks) {
    mRamp->MoveTo(nTicks, kRampStepTicks, fTarget);
}
