#include "app/hudcommon.h"

#include <memory>

#include "app/overlay.h"
#include "game/gamedb.h"
#include "os/string.h"
#include "rnd/rndmanager.h"

namespace {

// The layout letters of the remix displays.
constexpr char kHudSolo = 's';
constexpr char kHudLocal = 'm';
constexpr char kHudOnline = 'n';

// The player of the leader avatar, and its number.
constexpr int kLeaderPlayer = -1;
constexpr int kLeaderIndex = 0;

// The player SetPlaying() finds when not exactly one plays.
constexpr int kNoLeader = -1;

} // namespace

HudCommon::HudCommon(RndView *pHudView, DataArray *pConfig, DataArray *pDefaults)
    : mMessage("HUD genmsg.txt", pHudView), mMessage2("HUD genmsg2.txt", pHudView),
      mTextMessage(pHudView), mLetterbox(pHudView) {
    mPlaying = 0;
    if (TheGameDb->mRuleSet == GameDb::kRuleSetGame ||
        TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
        mSongPos = std::make_unique<OvySongPos>(pHudView);
    }
    for (int i = 0; i < kNumLanes; ++i) {
        mButtons[i] = std::make_unique<HudFlyingButton>(i, pHudView);
    }
    if (TheGameDb->mCommunity != GameDb::kCommunitySolo &&
        TheGameDb->mRuleSet != GameDb::kRuleSetRemix &&
        TheGameDb->mRuleSet != GameDb::kRuleSetDuel) {
        mLeaderAvatar = std::make_unique<OvyAvatar>(kLeaderPlayer, kLeaderIndex, pHudView);
    }
    if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
        char chHud = kHudSolo;
        if (TheGameDb->mCommunity != GameDb::kCommunitySolo) {
            chHud = TheGameDb->mCommunity == GameDb::kCommunityOnline ? kHudOnline : kHudLocal;
        }
        pHudView->AddAnim(dynamic_cast<RndAnimatable *>(
            TheManager.Find(FormatString("HUD%cr selected font.mnm", chHud))));
        auto *pLetterbox = dynamic_cast<RndView *>(
            TheManager.Find(FormatString("%s letterbox scale all.view", Overlay::sHudPrefix)));
        mSections = std::make_unique<OvySectionPanel>(
            chHud, String(FormatString("HUD%cr", chHud)), pLetterbox);
        mStick = std::make_unique<HudStick>(chHud, pHudView, pLetterbox);
        if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
            mChat = std::make_unique<OvyChat>(pConfig, pDefaults, pHudView);
        }
    }
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        mLetterExit = std::make_unique<HudLetterExit>();
    }
    if (TheGameDb->mCommunity != GameDb::kCommunityLocal &&
        TheGameDb->mRuleSet != GameDb::kRuleSetDuel) {
        mTrackLabel = std::make_unique<OvyTrackLabel>();
    }
    mMaterialAnim = dynamic_cast<RndAnimatable *>(TheManager.Find("mat_2d_always.anim"));
}

HudCommon::~HudCommon() = default;

void HudCommon::Poll(float fSongDelta, float fRealDelta, float fTickDelta) {
    const float fSongTime = TheGameDb->mSongTime;
    const float fTick = TheGameDb->mSongTick;
    if (mMaterialAnim != nullptr) {
        mMaterialAnim->SetFrame(fSongTime);
    }
    mJuice.Poll(fRealDelta);
    mHilite.Update(fTick);
    if (mSongPos != nullptr) {
        mSongPos->Poll();
    }
    mTextMessage.Poll();
    mLetterbox.Poll(fSongDelta, fTickDelta);
    for (const auto &pButton : mButtons) {
        pButton->Poll(fRealDelta);
    }
    if (mLeaderAvatar != nullptr) {
        mLeaderAvatar->Poll(fRealDelta);
    }
    if (mController != nullptr) {
        mController->HideablePanel::Poll();
    }
    if (mDialog != nullptr) {
        mDialog->HideablePanel::Poll();
    }
    if (mSections != nullptr) {
        mSections->Poll();
    }
    if (mChat != nullptr) {
        mChat->Poll();
    }
    if (mLetterExit != nullptr) {
        mLetterExit->Poll();
    }
    if (mTrackLabel != nullptr) {
        mTrackLabel->HideablePanel::Poll();
    }
}

void HudCommon::Draw() {
    if (mSongPos != nullptr) {
        mSongPos->Draw();
    }
    for (const auto &pButton : mButtons) {
        pButton->Draw();
    }
    if (mSections != nullptr) {
        mSections->Draw();
    }
    if (mStick != nullptr) {
        mStick->mMesh->Draw();
    }
}

void HudCommon::DrawText() {
    if (mChat != nullptr) {
        mChat->mDrawable->Draw();
    }
    mTextMessage.Draw();
    static_cast<RndDrawable *>(mLetterbox.mView)->Draw();
    mMessage.Draw();
    mMessage2.Draw();
}

void HudCommon::DrawAvatar() {
    if (mLeaderAvatar != nullptr) {
        mLeaderAvatar->Draw();
    }
}

void HudCommon::SetPlaying(int nPlayer, bool bPlaying) {
    if (nPlayer < 0) {
        return;
    }
    const unsigned char bit = static_cast<unsigned char>(1 << nPlayer);
    if (bPlaying) {
        mPlaying |= bit;
    } else {
        mPlaying &= static_cast<unsigned char>(~bit);
    }
    if (mLeaderAvatar == nullptr) {
        return;
    }
    int nLeader = kNoLeader;
    const signed char mask = static_cast<signed char>(mPlaying);
    for (signed char i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
        if (((mask >> i) & 1) == 0) {
            continue;
        }
        if (nLeader >= 0) {
            nLeader = kNoLeader;
            break;
        }
        nLeader = i;
    }
    mLeaderAvatar->SetPlayer(nLeader);
}

void HudCommon::Reset() {
    mMessage.Hide();
    mMessage2.Hide();
    mJuice.Reset();
    mTextMessage.Hide();
    mHilite.Reset();
    if (mSongPos != nullptr) {
        mSongPos->Reset();
    }
    mLetterbox.SetClosed(0.0f);
    for (const auto &pButton : mButtons) {
        pButton->Reset();
    }
    if (mLeaderAvatar != nullptr) {
        mLeaderAvatar->Reset();
    }
    if (mController != nullptr) {
        mController->Reset();
    }
    if (mDialog != nullptr) {
        mDialog->ShowNow(false);
        mDialog->SetPosition(0.0f);
    }
    if (mStick != nullptr) {
        mStick->Reset();
    }
    if (mLetterExit != nullptr) {
        mLetterExit->Reset();
    }
    if (mTrackLabel != nullptr) {
        mTrackLabel->Hide();
    }
}
