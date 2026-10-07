#include "game/helptext.h"

#include "game/controllerdisplay.h"
#include "game/gamedb.h"
#include "gfx/gfxmanager.h"
#include "os/locale.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"

namespace {

constexpr int kAllPlayers = -1;
// The skill level from which hints for beginners stop.
constexpr int kFirstExpertSkillLevel = 2;
constexpr float kMinIntervalMs = 1000.0f;
constexpr float kOneLineDurationMs = 1500.0f;
constexpr float kTwoLineDurationMs = 3000.0f;
constexpr float kFullScale = 1.0f;
constexpr float kMultiplayerScale = 0.7f;
constexpr float kMultiplayerOffsetY = -45.0f;
constexpr float kNoOffset = 0.0f;
constexpr float kAttentionCueMs = 2000.0f;

} // namespace

HelpText *TheHelpText = HelpText::shared();

HelpText::HelpText()
    : mEnabled(true), mLastShownMs(0.0f),
      mStopCueCmd(NewMemFunCommand(this, &HelpText::StopAttentionCue)) {
}

HelpText::~HelpText() = default;

HelpText *HelpText::shared() {
    static HelpText instance;
    return &instance;
}

void HelpText::SetEnabled(bool bEnabled) {
    mEnabled = bEnabled;
}

void HelpText::Reset() {
    mLastShownMs = 0.0f;
    mEnabled = true;
}

bool HelpText::ShowEnergyLow() {
    return Show("ENERGY_LOW", nullptr, false, true, kAllPlayers, true);
}

bool HelpText::ShowAutocatcherFailed() {
    return Show("AUTOCATCHER_FAILED_1", "AUTOCATCHER_FAILED_2", true, true, kAllPlayers, true);
}

bool HelpText::ShowBumperFailed() {
    return Show("BUMPER_FAILED_1", "BUMPER_FAILED_2", true, true, kAllPlayers, true);
}

bool HelpText::ShowCripplerFailed() {
    return Show("CRIPPLER_FAILED_1", "CRIPPLER_FAILED_2", true, true, kAllPlayers, true);
}

bool HelpText::ShowNotesAreEnergized() {
    if (TheGameDb->mTutorial != 0) {
        return Show("NOTES_ARE_ENERGIZED_TUTORIAL", nullptr, true, true, kAllPlayers, true);
    }
    return Show("NOTES_ARE_ENERGIZED", nullptr, true, true, kAllPlayers, true);
}

bool HelpText::ShowCatchBehind(int nPlayer) {
    const char *pszFirst = TheLocale.Localize("CATCH_BEHIND_1", true);
    const int nShownPlayer =
        TheGameDb->mCommunity == GameDb::kCommunityLocal ? nPlayer : kAllPlayers;
    const char *pszSecond = TheLocale.Localize("CATCH_BEHIND_2", true);
    return Show(pszFirst, pszSecond, true, false, nShownPlayer, false);
}

void HelpText::ShowEnergizeNotes(bool bCatchable) {
    if (TheGameDb->mPracticeMode != 0 || TheGameDb->mCommunity != GameDb::kCommunitySolo) {
        return;
    }
    if (bCatchable) {
        if (Show("ENERGIZE_NOTES_1", "ENERGIZE_NOTES_2A", true, true, kAllPlayers, true)) {
            PlayAttentionCue();
        }
    } else {
        Show("ENERGIZE_NOTES_1", "ENERGIZE_NOTES_2B", true, true, kAllPlayers, true);
    }
}

bool HelpText::ShowDeployAutocatcher() {
    return Show("DEPLOY_AUTOCATCHER", nullptr, true, true, kAllPlayers, true);
}

bool HelpText::Show(const char *pszFirst,
                    const char *pszSecond,
                    bool bBeginnerOnly,
                    bool bOnePadOnly,
                    int nPlayer,
                    bool bLocalize) {
    if (bOnePadOnly && TheGameDb->GetNumPads() >= 2) {
        return false;
    }
    if (bBeginnerOnly && TheGameDb->mSkillLevel >= kFirstExpertSkillLevel) {
        return false;
    }
    if (!mEnabled || TheGameDb->mRuleSet != GameDb::kRuleSetGame ||
        TheGameDb->GetOptions()->mHelpText == 0) {
        return false;
    }
    const float fNowMs = TheSongScheduler.mTime;
    if (fNowMs - mLastShownMs < kMinIntervalMs) {
        return false;
    }

    if (bLocalize) {
        pszFirst = TheLocale.Localize(pszFirst, true);
        if (pszSecond != nullptr) {
            pszSecond = TheLocale.Localize(pszSecond, true);
        }
    }
    const float fDurationMs = pszSecond != nullptr ? kTwoLineDurationMs : kOneLineDurationMs;
    const int nCommunity = TheGameDb->mCommunity;
    const bool bMultiplayer =
        nCommunity == GameDb::kCommunityLocal || nCommunity == GameDb::kCommunityOnline;
    const float fOffsetY = bMultiplayer ? kMultiplayerOffsetY : kNoOffset;
    const float fScale = bMultiplayer ? kMultiplayerScale : kFullScale;
    TheGfxManager.ShowMessage(
        pszFirst, pszSecond, nPlayer, fDurationMs, fScale, fOffsetY, kNoOffset);
    mLastShownMs = fNowMs;
    return true;
}

void HelpText::PlayAttentionCue() {
    TheSongScheduler.Cancel(mStopCueCmd.Get());
    TheControllerDisplay->Show();
    TheControllerDisplay->SetHighlight(true);
    TheSongScheduler.PostAfter(mStopCueCmd.Get(), kAttentionCueMs, false);
}

void HelpText::StopAttentionCue() {
    TheControllerDisplay->Hide();
    TheControllerDisplay->SetHighlight(false);
}
