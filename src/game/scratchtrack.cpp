#include "game/scratchtrack.h"

#include <algorithm>

#include "game/gameconfig.h"
#include "game/player.h"
#include "game/stats.h"
#include "gfx/gfxmanager.h"
#include "gs/multimuse.h"
#include "gs/notemuse.h"
#include "gs/stdmidimuse.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"

namespace {

// The program of the first release sound. Each later button uses the next program.
constexpr int kFirstReleaseProgram = 100;

// The note every release sound plays.
constexpr unsigned char kReleaseNote = 25;
constexpr unsigned char kReleaseVelocity = 127;
constexpr int kReleaseDurationTicks = 2000;

// How long a released scratch waits before Retrigger() may restart it.
constexpr int kRetriggerDelayTicks = 480;

} // namespace

ScratchTrack::ScratchTrack(ScratchTrackData *pData,
                           const SectionBoundaries *pSections,
                           PlayMap *pPlayMap,
                           const float *pfMsPerTick,
                           int nIndex,
                           [[maybe_unused]] int nIntroBars,
                           int nNumBars,
                           int nTicksPerBar)
    : FreestyleTrack(pSections, pPlayMap, pfMsPerTick, nIndex, nNumBars, nTicksPerBar),
      mTicksPerBar(nTicksPerBar), mNumBars(nNumBars), mData(pData), mActiveButton(kNoButton),
      mLastButton(0), mUpdateCommand(NewMemFunCommand(this, &ScratchTrack::UpdateScratchers)),
      mRetriggerCommand(NewMemFunCommand(this, &ScratchTrack::Retrigger)) {
    std::fill(mScratchers, mScratchers + kNumScratchers, nullptr);

    const unsigned char nChannel = static_cast<unsigned char>(nIndex);
    for (int i = 0; i < kNumScratchers; ++i) {
        MultiMuse *pSound = new MultiMuse(0);
        pSound->Add(StdMidiMuse::NewProgramChange(
                        nChannel, static_cast<unsigned char>(kFirstReleaseProgram + i)),
                    0);
        pSound->Add(new NoteMuse(kReleaseNote, kReleaseVelocity, kReleaseDurationTicks, nChannel),
                    0);
        mReleaseSounds[i] = Ptr<Muse>(pSound);
    }
}

ScratchTrack::~ScratchTrack() {
    ScratchTrack::Stop();
}

void ScratchTrack::Start() {
    UpdateScratchers();
}

void ScratchTrack::Stop() {
    FreestyleTrack::Stop();
    TheSongScheduler.Cancel(mUpdateCommand.Get());
    TheSongScheduler.Cancel(mRetriggerCommand.Get());
    for (Scratcher *pScratcher : mScratchers) {
        if (pScratcher != nullptr) {
            pScratcher->Stop();
        }
    }
}

void ScratchTrack::SetPlayer(Player *pPlayer) {
    EndScratch();
    FreestyleTrack::SetPlayer(pPlayer);
}

void ScratchTrack::HandleInput(Player *pPlayer, const PlayNoteEvent &event) {
    const int nTick = TheSongScheduler.mTick;
    (void)(nTick / mTicksPerBar); // Yes, the binary computes this quotient and discards it.
    if (nTick < 0) {
        return;
    }

    if (pPlayer == mPlayer) {
        mX = event.mX;
        mY = event.mY;
        if (event.mState != 0) {
            StartScratch(event.mButton, event.mX, event.mY);
        } else if (mActiveButton == event.mButton) {
            EndScratch();
        }
    }
    FreestyleTrack::HandleInput(pPlayer, event);
}

void ScratchTrack::HandleInput(Player *pPlayer, const StickEvent<2> &event) {
    FreestyleTrack::HandleInput(pPlayer, event);
    if (pPlayer != mPlayer) {
        return;
    }

    mX = event.mX;
    mY = event.mY;
    if (mActiveButton != kNoButton) {
        mScratchers[mActiveButton]->SetPosition(event.mX);
        mScratchers[mActiveButton]->SetPitch(event.mY);
    }
}

void ScratchTrack::HandleInput(Player *pPlayer, [[maybe_unused]] const BtnEvent<10> &event) {
    if (pPlayer != mPlayer) {
        return;
    }

    PlayReleaseSound(mLastButton);
    if (mActiveButton != kNoButton) {
        ReleaseScratch();
    }
    TheGfxManager.ResetFreestyle(pPlayer->GetIndex());
}

void ScratchTrack::Retrigger() {
    if (mActiveButton != kNoButton) {
        StartScratch(mActiveButton, mX, mY);
    }
}

void ScratchTrack::ReleaseScratch() {
    TheSongScheduler.Cancel(mRetriggerCommand.Get());
    TheSongScheduler.PostIn(mRetriggerCommand.Get(), kRetriggerDelayTicks, false);
    mScratchers[mActiveButton]->Stop();
}

void ScratchTrack::StartScratch(int nButton, float fX, float fY) {
    if ((mActiveButton != kNoButton) || (nButton != kNoButton)) {
        EndScratch();
    }

    const int nTick = TheSongScheduler.mTick;
    (void)(nTick / mTicksPerBar); // Yes, the binary computes this quotient and discards it.
    const int nPlayer = mPlayer->GetIndex();
    TheGfxManager.SetFreestyle(nPlayer, true, 0);
    mActiveButton = nButton;
    mLastButton = nButton;

    Scratcher *pScratcher = mScratchers[nButton];
    for (Ptr<Muse> &sound : mReleaseSounds) {
        sound->Stop();
    }
    pScratcher->SetPosition(fX);
    pScratcher->SetPitch(fY);
    pScratcher->Start(nPlayer, mTickOffset);
    TheStats->ScratchBegin(mIndex, nTick);
    TheSongScheduler.Cancel(mRetriggerCommand.Get());
}

void ScratchTrack::EndScratch() {
    TheSongScheduler.Cancel(mRetriggerCommand.Get());
    if (mActiveButton == kNoButton) {
        return;
    }

    mScratchers[mActiveButton]->Stop();
    mActiveButton = kNoButton;
    if (mPlayer != nullptr) {
        TheStats->ScratchEnd(mIndex, TheSongScheduler.mTick);
        TheGfxManager.SetFreestyle(mPlayer->GetIndex(), false, 0);
    }
}

void ScratchTrack::UpdateScratchers() {
    const int nTick = TheSongScheduler.mTick;
    const int nWrapped = WrapTick(nTick);
    const int nSetTick = (nWrapped >= 0) ? nWrapped : 0;

    Scratcher *apSet[kNumScratchers] = {};
    const int nNextSetTick = mData->GetScratchers(nSetTick, &apSet[0], &apSet[1], &apSet[2]);
    for (int i = 0; i < kNumScratchers; ++i) {
        if (mScratchers[i] == apSet[i]) {
            continue;
        }
        if (mScratchers[i] != nullptr) {
            mScratchers[i]->Stop();
        }
        mScratchers[i] = apSet[i];
    }

    if (mActiveButton != kNoButton) {
        mScratchers[mActiveButton]->Start(mPlayer->GetIndex(), mTickOffset);
    }
    TheSongScheduler.PostAt(mUpdateCommand.Get(), SpanEnd(nTick, nSetTick, nNextSetTick), false);
}

void ScratchTrack::PlayReleaseSound(int nButton) {
    const int nQuantum = TheGameConfig->mScratcherSampleQuantizationTicks;
    const int nRemainder = TheSongScheduler.mTick % nQuantum;
    const int nDelay = (nRemainder == 0) ? 0 : (nQuantum - nRemainder);
    mReleaseSounds[nButton]->PlayFrom(&TheSongScheduler, -nDelay);
}

void ScratchTrack::Refresh() {
    TheSongScheduler.Cancel(mUpdateCommand.Get());
    UpdateScratchers();
}
