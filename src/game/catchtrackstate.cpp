#include "game/catchtrackstate.h"

CatchTrackState::CatchTrackState(CatchTrackData *pData,
                                 int nNumBars,
                                 int nTicksPerBar,
                                 PlayMap *pPlayMap)
    : mPlayMap(pPlayMap), mData(pData), mNumBars(nNumBars), mTicksPerBar(nTicksPerBar),
      mBars(nNumBars, BarState{nullptr, 0, 0, 0}) {
    MarkGems();
}

CatchTrackState::~CatchTrackState() {
}

void CatchTrackState::SetWrittenCapturedBy(int nBar, Player *pPlayer) {
    mBars[nBar].mCapturedBy = pPlayer;
}

void CatchTrackState::SetCapturedBy(int nStartBar, int nEndBar, Player *pPlayer) {
    for (int nBar = nStartBar; nBar < nEndBar; ++nBar) {
        mBars[mPlayMap->MapBar(nBar)].mCapturedBy = pPlayer;
    }
}

Player *CatchTrackState::GetCapturedBy(int nBar) {
    return mBars[mPlayMap->MapBar(nBar)].mCapturedBy;
}

void CatchTrackState::SetEnabled(int nStartBar, int nEndBar, bool bEnabled) {
    for (int nBar = nStartBar; nBar < nEndBar; ++nBar) {
        mBars[mPlayMap->MapBar(nBar)].mEnabled = bEnabled;
    }
}

bool CatchTrackState::IsEnabled(int nBar) {
    return mBars[mPlayMap->MapBar(nBar)].mEnabled;
}

bool CatchTrackState::IsBarEmpty(int nBar) {
    return !mBars[mPlayMap->MapBar(nBar)].mHasGems;
}

bool CatchTrackState::IsWrittenBarEmpty(int nBar) {
    return !mBars[nBar].mHasGems;
}

void CatchTrackState::SetWrittenEnabled(int nBar, bool bEnabled) {
    mBars[nBar].mEnabled = bEnabled;
}

int CatchTrackState::GetPowerup(int nBar) {
    return mBars[mPlayMap->MapBar(nBar)].mPowerup;
}

void CatchTrackState::SetPowerup(int nBar, int nPowerup) {
    mBars[mPlayMap->MapBar(nBar)].mPowerup = nPowerup;
}

void CatchTrackState::SetWrittenPowerup(int nBar, int nPowerup) {
    mBars[nBar].mPowerup = nPowerup;
}

GemCursor CatchTrackState::GetCursor() {
    return GetCursor(0);
}

GemCursor CatchTrackState::GetCursor(int nLane, int nTick) {
    return FindCursor(nLane, nTick);
}

GemCursor CatchTrackState::GetCursor(int nTick) {
    return FindCursor(kAnyLane, nTick);
}

void CatchTrackState::MarkGems() {
    const int nNumGems = mData->GetNumGems();
    for (int i = 0; i < nNumGems; ++i) {
        const int nBar = mData->GetGem(i)->mTick / mTicksPerBar;
        if (nBar >= mNumBars) {
            break;
        }
        if (nBar >= 0) {
            mBars[nBar].mHasGems = 1;
        }
    }
}

void CatchTrackState::ClearBars(int nStartBar, int nEndBar) {
    SetEnabled(nStartBar, nEndBar, false);
    mData->EraseSpan(mPlayMap, nStartBar * mTicksPerBar, nEndBar * mTicksPerBar);
}
