#include "game/axetrack.h"

#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "game/player.h"
#include "game/stats.h"
#include "gfx/gfxmanager.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "script/dataarray.h"

namespace {

constexpr char kAxeSoftFxVolume[] = "axe_softfx_volume";
constexpr int kFirstTick = 0;
constexpr int kNoColumn = 0;

} // namespace

AxeTrack::AxeTrack(AxeTrackData *pData,
                   SectionBoundaries *pSections,
                   PlayMap *pPlayMap,
                   const float *pfMsPerTick,
                   int nIndex,
                   [[maybe_unused]] int nIntroBars,
                   int nNumBars,
                   int nTicksPerBar)
    : FreestyleTrack(pSections, pPlayMap, pfMsPerTick, nIndex, nNumBars, nTicksPerBar),
      mTicksPerBar(nTicksPerBar), mNumBars(nNumBars), mData(pData), mAxers(kNumAxers, nullptr),
      mActiveButton(kNoButton), mX(0.0f), mY(0.0f), mHarmony(nullptr),
      mHarmonyCommand(NewMemFunCommand(this, &AxeTrack::UpdateHarmony)),
      mContourCommand(NewMemFunCommand(this, &AxeTrack::UpdateContours)) {
    (void)mData->GetContours(kFirstTick, &mContours[0], &mContours[1], &mContours[2]);
    (void)mData->GetHarmony(kFirstTick, &mHarmony);

    float fVolume = TheGameConfig->mAxeSoftFxVolume;
    TheGameDb->FindSong(TheGameDb->mSong.c_str())->FindFloat(kAxeSoftFxVolume, &fVolume, false);
    for (int i = 0; i < kNumAxers; ++i) {
        mAxers[i] = new Axer(mContours[i], mHarmony, mData->mReserved, fVolume);
    }
}

AxeTrack::~AxeTrack() {
    AxeTrack::Stop();
    for (auto *pAxer : mAxers) {
        delete pAxer;
    }
}

void AxeTrack::Start() {
    UpdateHarmony();
    UpdateContours();
}

void AxeTrack::Stop() {
    FreestyleTrack::Stop();
    TheSongScheduler.Cancel(mHarmonyCommand.Get());
    TheSongScheduler.Cancel(mContourCommand.Get());
    for (auto *pAxer : mAxers) {
        pAxer->Stop();
    }
}

void AxeTrack::SetPlayer(Player *pPlayer) {
    EndNote();
    FreestyleTrack::SetPlayer(pPlayer);
}

void AxeTrack::HandleInput(Player *pPlayer, const PlayNoteEvent &event) {
    const int nTick = TheSongScheduler.mTick;
    (void)(nTick / mTicksPerBar); // Yes, the binary computes this quotient and discards it.
    if (nTick < 0) {
        return;
    }

    if (pPlayer == mPlayer) {
        mX = event.mX;
        mY = event.mY;
        if (event.mState != 0) {
            StartNote(event.mButton, mX, mY);
        } else if (mActiveButton == event.mButton) {
            EndNote();
        }
    }
    FreestyleTrack::HandleInput(pPlayer, event);
}

void AxeTrack::HandleInput(Player *pPlayer, const StickEvent<2> &event) {
    FreestyleTrack::HandleInput(pPlayer, event);
    if (pPlayer != mPlayer || mActiveButton == kNoButton) {
        return;
    }

    mX = event.mX;
    mY = event.mY;
    Axer *pAxer = mAxers[mActiveButton];
    (void)pAxer->IsPlaying(); // Yes, the binary discards this call's result.
    pAxer->SetPosition(mX);
    pAxer->SetPitch(mY);
}

void AxeTrack::StartNote(int nButton, float fX, float fY) {
    if (mActiveButton != kNoButton || nButton != kNoButton) {
        EndNote();
    }

    TheGfxManager.SetFreestyle(mPlayer->GetIndex(), true, kNoColumn);
    mActiveButton = nButton;
    Axer *pAxer = mAxers[nButton];
    const int nTick = TheSongScheduler.mTick;
    (void)(nTick / mTicksPerBar); // Yes, the binary computes this quotient and discards it.
    const int nOffset = nTick % pAxer->GetLength();
    pAxer->SetPosition(fX);
    pAxer->SetPitch(fY);
    pAxer->Start(nOffset);
    TheStats->AxeBegin(mIndex, nTick);
}

void AxeTrack::EndNote() {
    if (mActiveButton == kNoButton) {
        return;
    }

    mAxers[mActiveButton]->Stop();
    mActiveButton = kNoButton;
    if (mPlayer != nullptr) {
        TheStats->AxeEnd(mIndex, TheSongScheduler.mTick);
        TheGfxManager.SetFreestyle(mPlayer->GetIndex(), false, kNoColumn);
    }
}

void AxeTrack::UpdateHarmony() {
    const int nTick = TheSongScheduler.mTick;
    const int nWrapped = WrapTick(nTick);
    const int nSetTick = (nWrapped >= 0) ? nWrapped : 0;

    const AxeHarmony *pHarmony = nullptr;
    const int nNextTick = mData->GetHarmony(nSetTick, &pHarmony);
    if (pHarmony != mHarmony) {
        for (int i = 0; i < kNumAxers; ++i) {
            mAxers[i]->SetHarmony(pHarmony);
        }
        mHarmony = pHarmony;
    }
    TheSongScheduler.PostAt(mHarmonyCommand.Get(), SpanEnd(nTick, nSetTick, nNextTick), false);
}

void AxeTrack::UpdateContours() {
    const int nTick = TheSongScheduler.mTick;
    const int nWrapped = WrapTick(nTick);
    const int nSetTick = (nWrapped >= 0) ? nWrapped : 0;

    AxeContour *apContours[kNumAxers] = {};
    const int nNextTick =
        mData->GetContours(nSetTick, &apContours[0], &apContours[1], &apContours[2]);
    for (int i = 0; i < kNumAxers; ++i) {
        if (mContours[i] != apContours[i]) {
            mAxers[i]->SetContour(apContours[i]);
            mContours[i] = apContours[i];
        }
    }
    TheSongScheduler.PostAt(mContourCommand.Get(), SpanEnd(nTick, nSetTick, nNextTick), false);
}

void AxeTrack::Refresh() {
    TheSongScheduler.Cancel(mHarmonyCommand.Get());
    TheSongScheduler.Cancel(mContourCommand.Get());
    UpdateHarmony();
    UpdateContours();
}
