#include "app/overlay.h"

#include <algorithm>
#include <cstring>

#include "app/hideablepanel.h"
#include "app/ovyremixpanel.h"
#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "math/interpolator.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/manager.h"
#include "rnd/ps.h"

namespace {

// The row of a transform that holds the translation.
constexpr int kXfmRowTranslation = 3;

// The song position, in ticks, after which the parts of the display show.
constexpr float kShowPartsFrame = -3840.0f;

// The number of player slots of a duel.
constexpr int kNumDuelSlots = 4;

// The fraction of the letterbox per frame of its animation.
constexpr float kLetterboxPerFrame = 0.01f;

// The blink frequency of the energy bar is read per second and kept per millisecond.
constexpr float kPerSecondToPerMs = 0.002f;

// The layout children that stay shown.
constexpr char kHudCam[] = "hud.cam";
constexpr char kHudEnv[] = "hud.env";

} // namespace

// The static initialiser of the unit, and the two routines that run it to construct and to destroy
// the statics of the head-up display panels.
// NTSC-U/C: 0x001c60d0, PAL: 0x001cee70
// NTSC-U/C: 0x001c6268, PAL: 0x001cf008
// NTSC-U/C: 0x001c6288, PAL: 0x001cf028

char Overlay::sHudLetter = '\0';
const char *Overlay::sHudPrefix = nullptr;
float Overlay::sAssemblyTime = 2000.0f;
Vector2 Overlay::sHudZ{0.0f, 0.02f};
float Overlay::sPointsScale = 1.0f;
float Overlay::sDuelPointsMoveTicks = 480.0f;
float Overlay::sDuelPointsMoveRate = 1.0f / 480.0f;

void Overlay::SetHudPrefix() {
    const GameDb *pDb = TheGameDb;
    if (pDb->mCommunity == GameDb::kCommunitySolo ||
        (pDb->mCommunity == GameDb::kCommunityOnline && pDb->mRuleSet == GameDb::kRuleSetRemix)) {
        sHudLetter = '1';
        sHudPrefix = "HUD1";
    } else if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        sHudLetter = 'd';
        sHudPrefix = "HUDd";
    } else {
        sHudLetter = 'm';
        sHudPrefix = "HUDm";
    }
}

void Overlay::LoadConfig(DataArray *pConfig, DataArray *pDefaults, [[maybe_unused]] bool bUnused) {
    FindConfigFloat(pConfig, pDefaults, "assembly_time", &sAssemblyTime, true);
    FindConfigVector3(pConfig, pDefaults, "song_pos_start_pos", &OvySongPos::sStartPos, true);
    FindConfigVector3(pConfig, pDefaults, "song_pos_end_pos", &OvySongPos::sEndPos, true);
    FindConfigVector2(pConfig, pDefaults, "freq_z", &OvyAvatar::sDepthRange, true);
    FindConfigFloat(
        pConfig, pDefaults, "avatar_fx_leader_time", &OvyAvatar::sLeaderSwingTime, true);
    FindConfigVector2(pConfig, pDefaults, "hud_z", &sHudZ, true);
    FindConfigFloat(pConfig, pDefaults, "juice_ghost_fade_time", &OvyJuice::sGhostFadeTime, true);
    Vector2 blinkRange;
    FindConfigVector2(pConfig, pDefaults, "juice_blink_range", &blinkRange, true);
    Vector2 blinkFrequency;
    FindConfigVector2(pConfig, pDefaults, "juice_blink_frequency", &blinkFrequency, true);
    blinkFrequency.y *= kPerSecondToPerMs;
    blinkFrequency.x *= kPerSecondToPerMs;
    OvyJuice::sBlinkRate.Reset(blinkFrequency.x, blinkFrequency.y, blinkRange.x, blinkRange.y);
    FindConfigFloat(
        pConfig, pDefaults, "juice_dying_blink_time", &OvyJuice::sWarningBlinkTime, true);
    static const char *const kCheckpointNames[] = {
        "score_checkpoint_pos_1",
        "score_checkpoint_pos_2",
        "score_checkpoint_pos_3",
        "score_checkpoint_pos_4",
    };
    for (int i = 0; i < OvyScore::kNumCheckpointPositions; ++i) {
        FindConfigVector3(
            pConfig, pDefaults, kCheckpointNames[i], &OvyScore::sCheckpointPositions[i], true);
    }
    FindConfigFloat(
        pConfig, pDefaults, "score_checkpoint_move_in_ticks", &OvyScore::sMoveInTicks, true);
    FindConfigFloat(
        pConfig, pDefaults, "score_checkpoint_bar_grow_ticks", &OvyScore::sBarGrowTicks, true);
    FindConfigFloat(
        pConfig, pDefaults, "score_checkpoint_stable_ticks", &OvyScore::sStableTicks, true);
    FindConfigFloat(pConfig, pDefaults, "score_checkpoint_fade_ticks", &OvyScore::sFadeTicks, true);
    FindConfigFloat(
        pConfig, pDefaults, "score_checkpoint_move_out_ticks", &OvyScore::sMoveOutTicks, true);
    FindConfigFloat(pConfig, pDefaults, "hud_points_scale", &sPointsScale, true);
    FindConfigFloat(pConfig, pDefaults, "duel_points_move_ticks", &sDuelPointsMoveTicks, true);
    sDuelPointsMoveRate = 1.0f / sDuelPointsMoveTicks;
    OvyRemixPanel::LoadPositions(pConfig, pDefaults);
}

Overlay::Overlay(DataArray *pConfig, DataArray *pDefaults) : mCommon(nullptr), mStarted(0) {
    const int nPlayers = TheGameDb->GetNumPlayers();
    HideablePanel::Init();
    SetHudPrefix();
    mHudView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find("hud.view"));
    auto *pLayout =
        dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(FormatString("_hud%c.view", sHudLetter)));
    mHudView->AddView(pLayout);
    pLayout->RemoveView(
        dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(FormatString("%s test.anim", sHudPrefix))));
    for (Rnd::Drawable *pChild : static_cast<Rnd::Drawable *>(pLayout)->mDraws) {
        const char *pszName = pChild->mName.mStr;
        if (std::strcmp(pszName, kHudCam) != 0 && std::strcmp(pszName, kHudEnv) != 0) {
            pChild->SetShowing(false);
        }
    }
    mCam = dynamic_cast<Rnd::Cam *>(Rnd::TheManager.Find(kHudCam));
    const float (&camPos)[Rnd::kXfmRowFloatCount] = mCam->mLocalXfm[kXfmRowTranslation];
    mCamPos.x = camPos[0];
    mCamPos.y = camPos[1];
    mCamPos.z = camPos[2];
    mCamPos.w = camPos[3];
    mCam->mZRange = sHudZ;
    if (TheGameDb->mRuleSet != GameDb::kRuleSetDuel) {
        int nPosition = 0;
        for (int i = 0; i < nPlayers; ++i) {
            if (!TheGameDb->IsLocalPlayer(i)) {
                continue;
            }
            mLocals.push_back(new OvyLocalPlayer(i, nPosition));
            mAlls.push_back(new OvyAllPlayer(i, nPosition, pLayout));
            ++nPosition;
        }
        for (int i = 0; i < nPlayers; ++i) {
            if (TheGameDb->IsLocalPlayer(i)) {
                continue;
            }
            mAlls.push_back(new OvyAllPlayer(i, nPosition, pLayout));
            ++nPosition;
        }
    } else {
        for (int nSlot = 0; nSlot < kNumDuelSlots; ++nSlot) {
            for (int i = 0; i < nPlayers; ++i) {
                if (TheGameDb->GetPlayerSlot(i) != nSlot) {
                    continue;
                }
                if (TheGameDb->IsLocalPlayer(i)) {
                    mLocals.push_back(new OvyLocalPlayer(i, nSlot));
                }
                mAlls.push_back(new OvyAllPlayer(i, nSlot, pLayout));
                break;
            }
        }
    }
    mCommon = new HudCommon(pLayout, pConfig, pDefaults);
}

Overlay::~Overlay() {
    for (OvyAllPlayer *pAll : mAlls) {
        delete pAll;
    }
    for (OvyLocalPlayer *pLocal : mLocals) {
        delete pLocal;
    }
    delete mCommon;
    ThePs.mBlurActive = 0;
    HideablePanel::Terminate();
}

bool Overlay::DispatchPriv(Message *pMsg) {
    (void)pMsg->Type(); // Yes, the binary discards this call's result.
    return false;
}

void Overlay::CreateTutorialParts() {
    auto *pLayout =
        dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(FormatString("_hud%c.view", sHudLetter)));
    if (TheGameDb->mTutorial == 0) {
        return;
    }
    mCommon->mController = new OvyController(pLayout);
    if (TheGameDb->mTutorial == 0) {
        return;
    }
    mCommon->mDialog = new OvyDialog(pLayout);
}

OvyLocalPlayer *Overlay::FindLocal(int nPlayer) {
    for (OvyLocalPlayer *pLocal : mLocals) {
        if (pLocal->mPlayer == nPlayer) {
            return pLocal;
        }
    }
    return nullptr;
}

OvyAllPlayer *Overlay::FindAll(int nPlayer) {
    for (OvyAllPlayer *pAll : mAlls) {
        if (pAll->mPlayer == nPlayer) {
            return pAll;
        }
    }
    return nullptr;
}

void Overlay::AddCheckpoint(float fPos, const char *pszLabel) {
    if (mCommon->mSongPos != nullptr) {
        mCommon->mSongPos->AddCheckpoint(fPos, pszLabel);
    }
}

void Overlay::ClearCheckpoints() {
    if (mCommon->mSongPos != nullptr) {
        mCommon->mSongPos->ClearCheckpoints();
    }
}

void Overlay::SetEnergy(float fLevel, int nColor) {
    mCommon->mJuice.SetLevel(fLevel, nColor);
}

void Overlay::SetEnergyWarning(bool bWarn) {
    mCommon->mJuice.SetWarning(bWarn);
}

void Overlay::SetScore(int nPlayer, int nScore) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll == nullptr) {
        return;
    }
    if (pAll->mScore != nullptr) {
        pAll->mScore->SetScore(nScore);
    } else if (pAll->mDuelScore != nullptr) {
        pAll->mDuelScore->SetLitCount(nScore);
    }
}

void Overlay::SetMultiplier(int nPlayer, int nMultiplier, int bHot, const char *pszText) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->mPoints != nullptr) {
        pLocal->mPoints->SetMultiplier(nMultiplier, bHot, pszText);
    }
}

void Overlay::ShowPoints(int nPlayer, int nPoints, const char *pszText) {
    if (mCommon->mLetterExit != nullptr) {
        mCommon->mLetterExit->ShowReveal(nPlayer, nPoints, pszText);
        return;
    }
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->mPoints != nullptr) {
        pLocal->mPoints->ShowPoints(nPoints, pszText);
    }
}

void Overlay::HideMultiplier(int nPlayer) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->mPoints != nullptr) {
        pLocal->mPoints->HideMultiplier();
    }
}

void Overlay::EndPoints(int nPlayer, int nResult) {
    if (mCommon->mLetterExit != nullptr) {
        mCommon->mLetterExit->Fly(nResult);
        return;
    }
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->mPoints != nullptr) {
        pLocal->mPoints->End(nResult);
    }
}

void Overlay::SetPlaying(int nPlayer, bool bPlaying) {
    mCommon->SetPlaying(nPlayer, bPlaying);
}

void Overlay::ShowPowerup(int nPlayer, int nPowerup) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll != nullptr && pAll->mScore != nullptr && pAll->mScore->mPowerup != nullptr) {
        pAll->mScore->mPowerup->ShowPowerup(nPowerup);
    }
}

void Overlay::SetPowerupEnabled(int nPlayer, int nEnabled) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll != nullptr && pAll->mScore != nullptr && pAll->mScore->mPowerup != nullptr) {
        HudPowerup *pPowerup = pAll->mScore->mPowerup;
        pPowerup->mEnabled = nEnabled;
        pPowerup->UpdateSlide();
    }
}

void Overlay::ShowTextMessage(const char *pszText,
                              const char *pszSmallText,
                              float fDuration,
                              float fScale,
                              int nPlayer,
                              float fX,
                              float fZ) {
    mCommon->mTextMessage.Show(pszText, pszSmallText, fDuration, fScale, nPlayer, fX, fZ);
}

void Overlay::SetMessage(const char *pszText, bool bFirstLine) {
    if (bFirstLine) {
        mCommon->mMessage.SetText(pszText);
    } else {
        mCommon->mMessage2.SetText(pszText);
    }
}

void Overlay::SetLetterbox(bool bClosed) {
    mCommon->mLetterbox.SetClosed(bClosed ? 1.0f : 0.0f);
}

float Overlay::GetLetterbox() {
    return mCommon->mLetterbox.mSlide.mValue * kLetterboxPerFrame;
}

void Overlay::SetPartsShown(bool bShow, [[maybe_unused]] int nUnused) {
    if (bShow && TheGameDb->mRuleSet == GameDb::kRuleSetRemix &&
        TheGameDb->GetRemixInfo()->mReadOnly != 0) {
        if (mCommon->mSections != nullptr) {
            mCommon->mSections->Show(false);
        }
        for (OvyLocalPlayer *pLocal : mLocals) {
            if (pLocal->mRemix != nullptr) {
                static_cast<HideablePanel *>(pLocal->mRemix)->Show(false);
            }
            if (pLocal->mPoints != nullptr) {
                pLocal->mPoints->SetEnabled(false);
            }
        }
        if (mCommon->mSongPos != nullptr) {
            mCommon->mSongPos->Show(true);
        }
        if (mCommon->mLeaderAvatar != nullptr) {
            mCommon->mLeaderAvatar->Show(true);
        }
        for (OvyAllPlayer *pAll : mAlls) {
            if (pAll->mAvatar != nullptr) {
                pAll->mAvatar->Show(true);
            }
            if (pAll->mScore != nullptr) {
                pAll->mScore->Show(false);
                if (pAll->mScore->mPowerup != nullptr) {
                    pAll->mScore->mPowerup->mEnabled = 0;
                    pAll->mScore->mPowerup->UpdateSlide();
                }
            }
            if (pAll->mDuelScore != nullptr) {
                pAll->mDuelScore->Show(false);
            }
        }
        mCommon->mJuice.Show(false);
        return;
    }
    if (mCommon->mSections != nullptr) {
        mCommon->mSections->Show(bShow);
    }
    if (!bShow && mCommon->mTrackLabel != nullptr) {
        mCommon->mTrackLabel->Show(false);
    }
    for (OvyLocalPlayer *pLocal : mLocals) {
        if (pLocal->mRemix != nullptr) {
            static_cast<HideablePanel *>(pLocal->mRemix)->Show(bShow);
        }
        if (pLocal->mPoints != nullptr) {
            pLocal->mPoints->SetEnabled(bShow);
        }
    }
    const bool bPlayParts = TheGameDb->mTutorial != 0 ? false : bShow;
    if (mCommon->mSongPos != nullptr) {
        mCommon->mSongPos->Show(bPlayParts && TheGameDb->mRuleSet != GameDb::kRuleSetRemix);
    }
    if (mCommon->mLeaderAvatar != nullptr) {
        mCommon->mLeaderAvatar->Show(bPlayParts);
    }
    for (OvyAllPlayer *pAll : mAlls) {
        if (pAll->mAvatar != nullptr) {
            pAll->mAvatar->Show(bPlayParts);
        }
        if (pAll->mScore != nullptr) {
            pAll->mScore->Show(bPlayParts);
            if (pAll->mScore->mPowerup != nullptr) {
                pAll->mScore->mPowerup->mEnabled = bPlayParts;
                pAll->mScore->mPowerup->UpdateSlide();
            }
        }
        if (pAll->mDuelScore != nullptr) {
            pAll->mDuelScore->Show(bPlayParts);
        }
    }
    mCommon->mJuice.Show(TheGameDb->mPracticeMode == 0 ? bPlayParts : false);
}

void Overlay::ShowEnergy(bool bShow) {
    mCommon->mJuice.Show(bShow);
}

void Overlay::ShowScore(int nPlayer, bool bShow) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll != nullptr) {
        if (pAll->mScore != nullptr) {
            pAll->mScore->Show(bShow);
        } else if (pAll->mDuelScore != nullptr) {
            pAll->mDuelScore->Show(bShow);
        }
    }
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->mPoints != nullptr) {
        pLocal->mPoints->SetEnabled(bShow);
    }
}

void Overlay::ShowBox(bool bShow) {
    mCommon->mHilite.ShowBox(bShow);
}

void Overlay::ShowArrow(bool bShow) {
    mCommon->mHilite.ShowArrow(bShow);
}

void Overlay::ShowAvatar(int nPlayer, bool bShow) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll != nullptr && pAll->mAvatar != nullptr) {
        pAll->mAvatar->Show(bShow);
    } else if (nPlayer == 0 && mCommon->mLeaderAvatar != nullptr) {
        mCommon->mLeaderAvatar->Show(bShow);
    }
}

void Overlay::ShowSongPos(bool bShow) {
    if (mCommon->mSongPos != nullptr) {
        mCommon->mSongPos->Show(bShow);
    }
}

void Overlay::ShowTrackLabel(bool bShow) {
    if (mCommon->mTrackLabel != nullptr) {
        mCommon->mTrackLabel->Show(bShow);
    }
}

void Overlay::SetFreqSize(int nPlayer, int nFreqSize) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll != nullptr && pAll->mAvatar != nullptr) {
        pAll->mAvatar->SetFreqSize(nFreqSize);
    }
}

void Overlay::SetTrack(int nPlayer, int nInstrument, int nTrackKind) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll != nullptr && pAll->mAvatar != nullptr) {
        pAll->mAvatar->SetInstrument(nPlayer, nInstrument, nTrackKind);
    }
    if (mCommon->mTrackLabel != nullptr) {
        mCommon->mTrackLabel->SetTrack(nPlayer, nInstrument, nTrackKind);
    }
}

void Overlay::FlashPoints(int nPlayer, float fValue) {
    if (!(0.0f < fValue)) {
        return;
    }
    if (mCommon->mLetterExit != nullptr) {
        mCommon->mLetterExit->SetTime(fValue);
        return;
    }
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->mPoints != nullptr) {
        pLocal->mPoints->Flash(fValue);
    }
}

void Overlay::SetBoxRect(float fX0, float fY0, float fX1, float fY1, float fDuration) {
    mCommon->mHilite.SetBoxRect(fX0, fY0, fX1, fY1, fDuration);
}

void Overlay::SetArrowTarget(float fX, float fY, float fAngle, float fDuration) {
    mCommon->mHilite.SetArrowTarget(fX, fY, fAngle, fDuration);
}

void Overlay::FlyButton(int nLane, const Vector2 *pFrom, const Vector2 *pTo) {
    mCommon->mButtons[nLane]->Fly(pFrom, pTo);
}

void Overlay::SetButtonPulse(int nLane, bool bPulse) {
    mCommon->mButtons[nLane]->SetPulse(bPulse);
}

void Overlay::HideButton(int nLane) {
    mCommon->mButtons[nLane]->Hide();
}

void Overlay::SetButtonGlyph(int nLane, const char *pszGlyph) {
    mCommon->mButtons[nLane]->mText->SetText(pszGlyph);
}

void Overlay::ShowController(bool bShow, float fPosition) {
    OvyController *pController = mCommon->mController;
    if (pController == nullptr) {
        return;
    }
    pController->Show(bShow);
    if (0.0f <= fPosition) {
        mCommon->mController->mPosAnim->SetFrame(fPosition);
    }
}

void Overlay::SetControllerHighlight(bool bHighlight) {
    if (mCommon->mController != nullptr) {
        mCommon->mController->SetHighlight(bHighlight);
    }
}

void Overlay::ShowControllerButton(int nButton, bool bShow) {
    if (mCommon->mController != nullptr) {
        mCommon->mController->ShowButton(nButton, bShow);
    }
}

void Overlay::OpenDialog(const char *pszText, float fSize, float fPosition) {
    if (mCommon->mDialog != nullptr) {
        mCommon->mDialog->Open(pszText, fSize, fPosition);
    }
}

void Overlay::CloseDialog() {
    if (mCommon->mDialog != nullptr) {
        mCommon->mDialog->Close();
    }
}

void Overlay::SetSectionCount(int nCount) {
    if (mCommon->mSections != nullptr) {
        mCommon->mSections->SetSectionCount(nCount);
    }
}

void Overlay::SetSectionLabel(int nIndex, const char *pszLabel) {
    if (mCommon->mSections != nullptr) {
        mCommon->mSections->SetSectionLabel(nIndex, pszLabel);
    }
}

void Overlay::ShowSections(bool bShow) {
    if (mCommon->mSections != nullptr) {
        mCommon->mSections->Show(bShow);
    }
}

void Overlay::HighlightSection(int nIndex) {
    if (mCommon->mSections != nullptr) {
        mCommon->mSections->HighlightSection(nIndex);
    }
}

void Overlay::SetRemixPanelText(int nPlayer, int nIndex, const char *pszText, int nPanel) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->mRemix != nullptr) {
        pLocal->mRemix->FindSubPanel(nPanel)->SetText(nIndex, pszText);
    }
}

void Overlay::SelectRemixPanelEntry(int nPlayer, int nIndex, int nPanel) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->mRemix != nullptr) {
        pLocal->mRemix->FindSubPanel(nPanel)->Select(nIndex);
    }
}

void Overlay::FlashRemixPanel(int nPlayer, int nPanel) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->mRemix != nullptr) {
        pLocal->mRemix->FindSubPanel(nPanel)->Flash();
    }
}

void Overlay::SetRemixPanelLit(int nPlayer, int nIndex, int bLit, int nPanel) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->mRemix != nullptr) {
        pLocal->mRemix->FindSubPanel(nPanel)->SetLit(nIndex, bLit);
    }
}

void Overlay::ShowRemixPanel(int nPlayer, int nPanel) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->mRemix != nullptr) {
        pLocal->mRemix->ShowSubPanel(nPanel);
    }
}

void Overlay::ShowRemix(int nPlayer, bool bShow) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->mRemix != nullptr) {
        static_cast<HideablePanel *>(pLocal->mRemix)->Show(bShow);
    }
}

void Overlay::ShowStick(int nDirection, const Vector2 *pPosition) {
    if (mCommon->mStick != nullptr) {
        mCommon->mStick->Show(nDirection, pPosition);
    }
}

void Overlay::HideStick() {
    if (mCommon->mStick != nullptr) {
        mCommon->mStick->Hide();
    }
}

void Overlay::SetCameraOffset(const Vector3 *pOffset) {
    float position[Rnd::kXfmRowFloatCount]; // Yes, the binary copies the unset fourth word.
    position[0] = pOffset->x + mCamPos.x;
    position[1] = pOffset->y + mCamPos.y;
    position[2] = pOffset->z + mCamPos.z;
    float (&current)[Rnd::kXfmRowFloatCount] = mCam->mLocalXfm[kXfmRowTranslation];
    if (position[0] != current[0] || position[1] != current[1] || position[2] != current[2]) {
        mCam->mDirty = 1;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        for (int i = 0; i < Rnd::kXfmRowFloatCount; ++i) {
            current[i] = position[i];
        }
#pragma GCC diagnostic pop
    }
}

void Overlay::Reset() {
    mCommon->Reset();
    for (OvyLocalPlayer *pLocal : mLocals) {
        pLocal->Reset();
    }
    for (OvyAllPlayer *pAll : mAlls) {
        pAll->Reset();
    }
    SetPartsShown(false, 0);
}

void Overlay::ClearStarted() {
    mStarted = 0;
}

void Overlay::ShowCheckpoint() {
    if (mAlls.size() < 2 || TheGameDb->mRuleSet != GameDb::kRuleSetGame) {
        return;
    }
    std::vector<OvyScore *> scores(OvyScore::kNumCheckpointPositions);
    scores.clear();
    int nMaxScore = 0;
    for (OvyAllPlayer *pAll : mAlls) {
        OvyScore *pScore = pAll->mScore;
        if (pScore == nullptr) {
            continue;
        }
        scores.push_back(pScore);
        if (nMaxScore < pScore->mScore) {
            nMaxScore = pScore->mScore;
        }
    }
    std::stable_sort(scores.begin(), scores.end(), [](OvyScore *pLeft, OvyScore *pRight) {
        return pLeft->mScore < pRight->mScore;
    });
    if (!(0 < nMaxScore)) {
        nMaxScore = 1;
    }
    const int nCount = static_cast<int>(scores.size());
    for (int i = nCount - 1; i >= 0; --i) {
        OvyScore *pScore = scores[i];
        pScore->ShowCheckpoint(static_cast<float>(pScore->mScore) / static_cast<float>(nMaxScore),
                               nCount - (i + 1));
    }
}

void Overlay::ShowResult(bool bWon, [[maybe_unused]] int nMode) {
    if (!bWon) {
        mCommon->mJuice.mHideBar = 1;
        return;
    }
    for (OvyLocalPlayer *pLocal : mLocals) {
        if (pLocal->mPoints == nullptr) {
            continue;
        }
        pLocal->mPoints->SetMultiplier(1, 0, "");
        pLocal->mPoints->End(GfxManager::kPendingPointsCleared);
    }
}

void Overlay::Poll(float fFrame, float fSongDelta, float fUnused, float fRealDelta, float fFuture) {
    if (mStarted == 0 && 0.0f < fRealDelta && kShowPartsFrame < fFrame) {
        mStarted = 1;
        SetPartsShown(true, 1); // Yes, the binary passes 1 for the unread argument here.
    }
    const float fNow = SystemMs();
    const float fDelta = fNow - mLastTime;
    mLastTime = fNow;
    HideablePanel::SetDeltaTime(fDelta);
    mHudView->SetFrame(fFrame);
    if (mCommon->mSongPos != nullptr) {
        mCommon->mSongPos->SetFuture(fFuture);
    }
    for (OvyAllPlayer *pAll : mAlls) {
        pAll->Poll(fSongDelta, fRealDelta, fDelta);
    }
    for (OvyLocalPlayer *pLocal : mLocals) {
        pLocal->Poll(fFrame, fSongDelta, fUnused, fRealDelta);
    }
    mCommon->Poll(fSongDelta, fRealDelta, fDelta);
}

void Overlay::Draw() {
    TheGfxManager.mWinnerDrawn = 0;
    static_cast<Rnd::Transformable *>(mHudView)->UpdateWorldXfm(nullptr, 0);
    HideablePanel::PoseMaterials();
    static_cast<Rnd::Drawable *>(mHudView)->Draw();
    mCommon->Draw();
    for (OvyLocalPlayer *pLocal : mLocals) {
        pLocal->DrawHud();
    }
    for (OvyLocalPlayer *pLocal : mLocals) {
        pLocal->DrawAvatar();
    }
    for (OvyAllPlayer *pAll : mAlls) {
        pAll->DrawHud();
    }
    mCommon->DrawAvatar();
    const int nWinner = TheGfxManager.mWinner;
    for (OvyAllPlayer *pAll : mAlls) {
        if (pAll->mPlayer != nWinner) {
            pAll->DrawAvatar();
        }
    }
    if (nWinner >= 0) {
        OvyAllPlayer *pWinner = FindAll(nWinner);
        if (pWinner != nullptr) {
            pWinner->DrawAvatar();
        }
    }
    mCommon->DrawText();
}
