#include "game/player.h"

#include "game/gamecallback.h"
#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "game/gamelogic.h"
#include "game/stats.h"
#include "gfx/gfxmanager.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"

namespace {

// The multiplier value while no multiplier power-up is active.
constexpr int kNoMultiplier = 1;

// The streak multiplier SetScore() passes with a score set outright.
constexpr int kBaseStreakMultiplier = 1;

constexpr int kNoReserved = -1;

} // namespace

Player::Player(int nIndex, int nTicksPerBar)
    : mIndex(nIndex), mTicksPerBar(nTicksPerBar), mTrack(nullptr), mScore(0), mPendingPoints(0),
      mMultiplierValue(kNoMultiplier), mStreak(0), mCatching(false), mAborted(false), mRepeat(true),
      mReserved(kNoReserved), mMultiplierEndCommand(NewMemFunCommand(this, &Player::EndMultiplier)),
      mPowerup(GameLogic::kPowerupNone) {
}

Player::~Player() {
    CancelMultiplier();
}

void Player::SetTrack(Track *pTrack) {
    mTrack = pTrack;
    TheStats->ChangeTrack(mIndex, pTrack->mIndex, TheSongScheduler.mTick);
}

Track *Player::GetTrack() const {
    return mTrack;
}

void Player::HidePowerup() {
    TheGfxManager.ShowPowerup(mIndex, GameLogic::kPowerupNone);
}

void Player::CancelMultiplier() {
    TheSongScheduler.Cancel(mMultiplierEndCommand.Get());
}

void Player::HandleInput(const PlayNoteEvent &event) {
    mTrack->HandleInput(this, event);
}

void Player::HandleInput(const BtnEvent<10> &event) {
    mTrack->HandleInput(this, event);
}

void Player::HandleInput(const BtnEvent<8> &event) {
    mTrack->HandleInput(this, event);
}

void Player::HandleInput(const StickEvent<2> &event) {
    mTrack->HandleInput(this, event);
}

void Player::HandleInput(const StickEvent<6> &event) {
    mTrack->HandleInput(this, event);
}

void Player::Abort() {
    mAborted = true;
}

void Player::SetScore(int nScore) {
    mScore = nScore;
    TheGfxManager.SetScore(mIndex, nScore, kBaseStreakMultiplier, 0);
}

void Player::AddScore(int nPoints) {
    mScore += nPoints;
    TheGfxManager.SetScore(mIndex, mScore, kBaseStreakMultiplier, nPoints);
}

void Player::SetPendingPoints(int nPoints, bool bHide) {
    mPendingPoints = nPoints;
    if (nPoints != 0) {
        TheGfxManager.ShowPendingPoints(mIndex, nPoints * mMultiplierValue);
        return;
    }
    TheGfxManager.SetPendingPointsResult(mIndex, GfxManager::kPendingPointsCleared);
    if (bHide) {
        TheGfxManager.HidePendingPoints(mIndex);
    }
}

void Player::CommitPendingPoints(bool bHide) {
    const int nStreakMultiplier = StreakMultiplier(mStreak);
    const int nPoints = mPendingPoints * mMultiplierValue * nStreakMultiplier;
    mScore += nPoints;
    if (nPoints > 0) {
        TheGfxManager.SetScore(mIndex, mScore, nStreakMultiplier, nPoints);
        TheGfxManager.SetPendingPointsResult(mIndex, GfxManager::kPendingPointsCaptured);
        if (bHide) {
            TheGfxManager.HidePendingPoints(mIndex);
        }
    } else {
        TheGfxManager.SetPendingPointsResult(mIndex, GfxManager::kPendingPointsCleared);
    }
    mPendingPoints = 0;
}

void Player::LosePendingPoints() {
    TheGfxManager.SetPendingPointsResult(mIndex, GfxManager::kPendingPointsLost);
    mPendingPoints = 0;
}

void Player::ActivateMultiplier() {
    mMultiplierValue = TheGameConfig->mMultiplierValue;
    TheGfxManager.SetStreakMultiplier(
        mIndex, StreakMultiplier(mStreak), StreakMultiplier(mStreak + 1), true);
    SetPendingPoints(mPendingPoints, true);
    const int nEndTick =
        TheSongScheduler.mTick + TheGameConfig->mMultiplierDurationBars * mTicksPerBar;
    TheGfxManager.SetMultiplierEndTick(mIndex, static_cast<float>(nEndTick));
    TheSongScheduler.Cancel(mMultiplierEndCommand.Get());
    TheSongScheduler.PostAt(mMultiplierEndCommand.Get(), nEndTick, false);
}

void Player::EndMultiplier() {
    mMultiplierValue = kNoMultiplier;
    TheGfxManager.SetStreakMultiplier(
        mIndex, StreakMultiplier(mStreak), StreakMultiplier(mStreak + 1), false);
    SetPendingPoints(mPendingPoints, true);
}

void Player::SetStreak(int nStreak) {
    mStreak = nStreak;
    TheGfxManager.SetStreakMultiplier(mIndex,
                                      StreakMultiplier(nStreak),
                                      StreakMultiplier(mStreak + 1),
                                      mMultiplierValue > kNoMultiplier);
}

void Player::IncrementStreak() {
    SetStreak(mStreak + 1);
}

void Player::ResetStreak() {
    SetStreak(0);
}

int Player::GetStreak() const {
    return mStreak;
}

void Player::SetCatching(bool bCatching) {
    mCatching = bCatching;
}

bool Player::GetCatching() const {
    return mCatching;
}

int Player::GetPowerup() const {
    return mPowerup;
}

void Player::SetPowerup(int nPowerup) {
    mPowerup = nPowerup;
    TheGfxManager.ShowPowerup(mIndex, nPowerup);
    if (TheGameCallback != nullptr && nPowerup != GameLogic::kPowerupNone) {
        TheGameCallback->OnPowerup();
    }
}

bool Player::GetRepeat() const {
    return mRepeat;
}

void Player::SetRepeat(bool bRepeat) {
    mRepeat = bRepeat;
}

int Player::StreakMultiplier(int nStreak) const {
    const int nMultiplier = nStreak + 1;
    const int nMaximum = TheGameDb->mCommunity == GameDb::kCommunitySolo ?
                             TheGameConfig->mStreakMultiplierMaxSolo :
                             TheGameConfig->mStreakMultiplierMaxMulti;
    return nMaximum < nMultiplier ? nMaximum : nMultiplier;
}
