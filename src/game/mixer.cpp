#include "game/mixer.h"

#include <algorithm>

#include "game/gamedb.h"
#include "os/debug.h"
#include "os/memfun1command.h"
#include "os/scheduler.h"
#include "os/system.h"
#include "script/scriptfunction.h"
#include "synth/synth.h"

namespace {

constexpr char kGameSection[] = "game";
constexpr char kMixerSection[] = "mixer";
constexpr char kMuteCommandName[] = "track_solo";

constexpr unsigned char kFullVolume = 127;
constexpr float kRampStartVolume = 100.0f;
constexpr unsigned char kDeactivatedVolume = 100;

// The ticks between the steps of a volume move.
constexpr int kVolumeStepTicks = 50;
constexpr int kFadeStepTicks = 20;

constexpr int kVictoryLapTicks = 1920;

constexpr unsigned char kMidiControlChange = 0xb0;
constexpr unsigned char kMidiVolumeController = 17;

} // namespace

Mixer *TheMixer = Mixer::shared();

void Mixer::ReadVolumes(const DataArray *pConfig,
                        const char *pszKey,
                        std::vector<unsigned char> &volumes) {
    const DataArray *pList = pConfig->FindArray(pszKey, true);
    volumes.resize(pList->mSize - 1, 0);
    for (int i = 1; i < pList->mSize; ++i) {
        volumes[i - 1] = static_cast<unsigned char>(pList->Int(i));
    }
}

int Mixer::ReadInt(const char *pszKey, const DataArray *pConfig) {
    int nValue = 0;
    pConfig->FindInt(pszKey, &nValue, true);
    return nValue;
}

unsigned char Mixer::ReadByte(const char *pszKey, const DataArray *pConfig) {
    int nValue = 0;
    pConfig->FindInt(pszKey, &nValue, true);
    return static_cast<unsigned char>(nValue);
}

void Mixer::VolumeRamp::Apply(float fValue, [[maybe_unused]] int nTick) {
    TheSynth->SendMessage(kMidiControlChange | mChannel,
                          kMidiVolumeController,
                          static_cast<unsigned char>(static_cast<int>(fValue)),
                          0);
}

Mixer::Mixer() {
    mRampTicks = -1;
    mHighVolume = kFullVolume;
    mVictoryLapVolume = kFullVolume;
    mHeldCount = 0;
    mMode = kModeOff;
    mHasInstrumentTrack = false;
    mMuteUnoccupied = false;
    for (int i = 0; i < kChannelCount; ++i) {
        mRamps[i] = new VolumeRamp(&TheSongScheduler, kRampStartVolume, i);
    }
}

Mixer::~Mixer() {
    for (VolumeRamp *pRamp : mRamps) {
        delete pRamp;
    }
}

void Mixer::Init(int nTracks, int nInstrument) {
    mHasInstrumentTrack = nInstrument != kInstrumentNone;
    mHeldCount = 0;
    mMode = kModeOff;

    mTracks.clear();
    mTracks.resize(nTracks);
    for (int i = 0; i < nTracks; ++i) {
        mTracks[i].mReleaseCommand = Ptr<Command>(NewMemFun1Command(this, &Mixer::ReleaseTrack, i));
    }

    const DataArray *pConfig =
        SystemConfig()->FindArray(kGameSection, false)->FindArray(kMixerSection, false);
    pConfig->FindInt("ramp_ticks", &mRampTicks, true);
    mHighVolume = static_cast<unsigned char>(ReadInt("high_volume", pConfig));

    std::vector<unsigned char> lowVolumes;
    ReadVolumes(pConfig, "low_volumes", lowVolumes);
    if (nInstrument == kInstrumentAxe) {
        ReadVolumes(pConfig, "low_volumes_axe", mInstrumentVolumes);
        mVictoryLapVolume = ReadByte("victorylap_volume_axe", pConfig);
    } else {
        ReadVolumes(pConfig, "low_volumes_scratch", mInstrumentVolumes);
        mVictoryLapVolume = ReadByte("victorylap_volume_scratch", pConfig);
    }
    for (int i = 0; i < kFxChannel; ++i) {
        mVolumes[i] = lowVolumes;
    }
    ReadVolumes(pConfig, "fx_volumes", mVolumes[kFxChannel]);

    for (int i = 0; i < kChannelCount; ++i) {
        mRamps[i]->Stop();
        SetVolume(i, mVolumes[i][0], 0);
    }
}

bool Mixer::IsInstrumentTrack(int nTrack) const {
    if (!mHasInstrumentTrack) {
        return false;
    }
    return nTrack == static_cast<int>(mTracks.size()) - 1;
}

void Mixer::SetVolumes(int nChannel, const std::vector<unsigned char> &volumes) {
    mVolumes[nChannel] = volumes;
}

void Mixer::SetInstrumentVolumes(const std::vector<unsigned char> &volumes) {
    mInstrumentVolumes = volumes;
}

Mixer *Mixer::shared() {
    static Mixer instance;
    return &instance;
}

void Mixer::AddPlayer(int nTrack) {
    ++mTracks[nTrack].mPlayers;
    if (mMode != kModePlaying) {
        return;
    }
    if (IsInstrumentTrack(nTrack)) {
        RefreshAll(mRampTicks);
    } else {
        RefreshTrack(nTrack, 0);
    }
}

void Mixer::RemovePlayer(int nTrack) {
    --mTracks[nTrack].mPlayers;
    if (mMode != kModePlaying) {
        return;
    }
    if (IsInstrumentTrack(nTrack)) {
        RefreshAll(mRampTicks);
    } else if (mTracks[nTrack].mPlayers == 0) {
        RefreshTrack(nTrack, mRampTicks);
    }
}

void Mixer::HoldTrack(int nTrack, int nUntilTick) {
    if (TheSongScheduler.mTick >= nUntilTick) {
        return;
    }
    (void)IsInstrumentTrack(nTrack); // Yes, the binary discards this call's result.
    ++mHeldCount;
    mTracks[nTrack].mHeld = true;
    TheSongScheduler.PostAt(mTracks[nTrack].mReleaseCommand.Get(), nUntilTick - 1, false);
    if (mMode == kModePlaying) {
        for (int i = 0; i < kChannelCount; ++i) {
            if (i != nTrack) {
                RefreshTrack(i, mRampTicks);
            }
        }
    }
}

void Mixer::ReleaseTrack(int nTrack) {
    (void)IsInstrumentTrack(nTrack); // Yes, the binary discards this call's result.
    --mHeldCount;
    mTracks[nTrack].mHeld = false;
    if (mMode == kModePlaying) {
        RefreshAll(mRampTicks);
    }
}

void Mixer::PlayFullMix() {
    mMode = kModeFullMix;
    for (int i = 0; i < kFxChannel; ++i) {
        SetVolume(i, mHighVolume, 0);
    }
}

void Mixer::StartVictoryLap() {
    mMode = kModeVictoryLap;
    for (int i = 0; i < kChannelCount; ++i) {
        if (mHasInstrumentTrack && i == static_cast<int>(mTracks.size()) - 1) {
            SetVolume(i, mHighVolume, kVictoryLapTicks);
        } else {
            SetVolume(i, mVictoryLapVolume, kVictoryLapTicks);
        }
    }
}

void Mixer::Activate() {
    if (mMode != kModeOff) {
        return;
    }
    ScriptFunction::Register(ToggleMuteUnoccupied, kMuteCommandName, nullptr);
    mMode = kModePlaying;
    RefreshAll(0);
}

void Mixer::Deactivate() {
    if (mMode == kModeOff) {
        return;
    }
    mMode = kModeOff;
    ScriptFunction::Unregister(ToggleMuteUnoccupied);
    for (int i = 0; i < kFxChannel; ++i) {
        mRamps[i]->Stop();
        SetVolume(i, kDeactivatedVolume, 0);
    }
}

void Mixer::FadeOut(int nTicks) {
    for (int i = 0; i < kFxChannel; ++i) {
        mRamps[i]->Move(nTicks, kFadeStepTicks, mRamps[i]->GetValue(), 0.0f);
    }
}

void Mixer::ApplyVolume(int nChannel, const std::vector<unsigned char> &volumes, int nTicks) {
    const int nLast = static_cast<int>(volumes.size()) - 1;
    unsigned char nVolume = volumes[std::min(mHeldCount, nLast)];
    if (mMuteUnoccupied) {
        nVolume = 0;
    }
    if (TheGameDb->GetNumPads() >= 2) {
        nVolume = mHighVolume;
    }
    SetVolume(nChannel, nVolume, nTicks);
}

void Mixer::SetVolume(int nChannel, unsigned char nVolume, int nTicks) {
    mRamps[nChannel]->MoveTo(nTicks, kVolumeStepTicks, static_cast<float>(nVolume));
}

unsigned char Mixer::GetVolume(int nChannel) {
    return static_cast<unsigned char>(static_cast<int>(mRamps[nChannel]->GetValue()));
}

void Mixer::RefreshTrack(int nChannel, int nTicks) {
    if (mHasInstrumentTrack && mTracks.back().mPlayers > 0) {
        if (nChannel == static_cast<int>(mTracks.size()) - 1) {
            SetVolume(nChannel, mHighVolume, nTicks);
        } else if (nChannel == kFxChannel) {
            ApplyVolume(kFxChannel, mVolumes[kFxChannel], nTicks);
        } else {
            ApplyVolume(nChannel, mInstrumentVolumes, nTicks);
        }
        return;
    }

    if (static_cast<unsigned>(nChannel) < mTracks.size()) {
        const TrackData &track = mTracks[nChannel];
        const bool bFollowsPlayers = !track.mHeld || mMuteUnoccupied;
        if (bFollowsPlayers && track.mPlayers > 0) {
            SetVolume(nChannel, mHighVolume, 0); // Yes, an occupied track moves at once.
            return;
        }
    }
    ApplyVolume(nChannel, mVolumes[nChannel], nTicks);
}

void Mixer::RefreshAll(int nTicks) {
    for (int i = 0; i < kChannelCount; ++i) {
        RefreshTrack(i, nTicks);
    }
}

void Mixer::ToggleMuteUnoccupied([[maybe_unused]] DataArray *pCommand,
                                 [[maybe_unused]] void *pUserData) {
    TheMixer->mMuteUnoccupied = !TheMixer->mMuteUnoccupied;
    DebugPrint("CHEAT: %s muting unoccupied tracks\n", TheMixer->mMuteUnoccupied ? "" : "not");
    TheMixer->RefreshAll(0);
}
