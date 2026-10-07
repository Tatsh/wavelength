#include "game/stutter.h"

#include <algorithm>

#include "game/gamedb.h"
#include "game/mixer.h"
#include "game/songentry.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "synth/synth.h"

namespace {

constexpr int kUnsetPeriod = -1;
constexpr float kFastBpm = 160.0f;
constexpr int kSlowPeriodTicks = 120;
constexpr int kFastPeriodTicks = 240;
constexpr int kChosen = 1;

// The control change and the controller the gate sets.
constexpr unsigned int kStatusControlChange = 0xb0;
constexpr unsigned char kControllerGate = 17;

} // namespace

Stutter::Stutter()
    : mRunning(0), mPeriodTicks(kUnsetPeriod), mOpenCmd(NewMemFunCommand(this, &Stutter::Open)),
      mCloseCmd(NewMemFunCommand(this, &Stutter::Close)) {
    const SongEntry entry{TheGameDb->FindSong(TheGameDb->mSong.c_str())};
    mPeriodTicks = kFastBpm <= entry.GetBpm() ? kFastPeriodTicks : kSlowPeriodTicks;
    std::fill(mChannels, mChannels + kNumChannels, 0);
}

Stutter::~Stutter() {
    Stop();
}

void Stutter::SetChannel(int nChannel, int nOn) {
    mChannels[nChannel] = nOn;
    if (std::find(mChannels, mChannels + kNumChannels, kChosen) != mChannels + kNumChannels) {
        Start();
    } else {
        Stop();
    }
    if (nOn == 0) {
        SetVolume(nChannel, true);
    }
}

void Stutter::Start() {
    if (mRunning) {
        return;
    }
    mRunning = 1;
    int nTick = std::max(TheSongScheduler.mTick, 0);
    const int nRemainder = nTick % mPeriodTicks;
    if (nRemainder != 0) {
        nTick += mPeriodTicks - nRemainder;
    }
    TheSongScheduler.PostAt(mOpenCmd.Get(), nTick, false);
    TheSongScheduler.PostAt(mCloseCmd.Get(), nTick + mPeriodTicks / 2, false);
}

void Stutter::Stop() {
    if (!mRunning) {
        return;
    }
    mRunning = 0;
    SetVolumes(true);
    TheSongScheduler.Cancel(mOpenCmd.Get());
    TheSongScheduler.Cancel(mCloseCmd.Get());
}

void Stutter::Open() {
    SetVolumes(true);
    TheSongScheduler.PostIn(mOpenCmd.Get(), mPeriodTicks, false);
}

void Stutter::Close() {
    SetVolumes(false);
    TheSongScheduler.PostIn(mCloseCmd.Get(), mPeriodTicks, false);
}

void Stutter::SetVolumes(bool bOpen) {
    for (int i = 0; i < kNumChannels; ++i) {
        if (mChannels[i] != 0) {
            SetVolume(i, bOpen);
        }
    }
}

void Stutter::SetVolume(int nChannel, bool bOpen) {
    const unsigned char nVolume = bOpen ? TheMixer->GetVolume(nChannel) : 0;
    TheSynth->SendMessage(
        static_cast<unsigned char>(nChannel) | kStatusControlChange, kControllerGate, nVolume, 0);
}
