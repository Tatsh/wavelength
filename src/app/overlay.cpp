#include "app/overlay.h"

#include <algorithm>
#include <cstring>
#include <memory>

#include "app/hideablepanel.h"
#include "app/ovyremixpanel.h"
#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "gfx/gfxconfig.h"
#include "gfx/gfxmanager.h"
#include "gfx/gfxutil.h"
#include "gfx/unreadvector.h"
#include "math/interpolator.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/ps.h"
#include "rnd/rndmanager.h"

namespace {

// The song position, in ticks, after which the parts of the display show.
constexpr float kShowPartsFrame = -3840.0f;

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
    if (pDb->GetCommunity() == GameDb::kCommunitySolo ||
        (pDb->GetCommunity() == GameDb::kCommunityOnline &&
         pDb->GetRuleSet() == GameDb::kRuleSetRemix)) {
        sHudLetter = '1';
        sHudPrefix = "HUD1";
    } else if (TheGameDb->GetRuleSet() == GameDb::kRuleSetDuel) {
        sHudLetter = 'd';
        sHudPrefix = "HUDd";
    } else {
        sHudLetter = 'm';
        sHudPrefix = "HUDm";
    }
}

void Overlay::LoadConfig(DataArray *pConfig, DataArray *pDefaults, bool bUnused) {
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
    OvyRemixPanel::LoadPositions(pConfig, pDefaults, bUnused);
}

Overlay::Overlay(DataArray *pConfig, DataArray *pDefaults) : mStarted(0) {
    const int nPlayers = TheGameDb->GetNumPlayers();
    HideablePanel::Init();
    SetHudPrefix();
    mHudView = dynamic_cast<RndView *>(TheManager.Find("hud.view"));
    auto *pLayout =
        dynamic_cast<RndView *>(TheManager.Find(FormatString("_hud%c.view", sHudLetter)));
    AttachView(mHudView, pLayout);
    DetachView(pLayout,
               dynamic_cast<RndView *>(TheManager.Find(FormatString("%s test.anim", sHudPrefix))));
    for (RndDrawable *pChild : static_cast<RndDrawable *>(pLayout)->GetDraws()) {
        const char *pszName = pChild->GetName();
        if (std::strcmp(pszName, kHudCam) != 0 && std::strcmp(pszName, kHudEnv) != 0) {
            pChild->SetShowing(false);
        }
    }
    mCam = dynamic_cast<RndCam *>(TheManager.Find(kHudCam));
    mCamPos = mCam->GetLocalXfm().mTranslation;
    mCam->GetZRange() = sHudZ;
    if (TheGameDb->GetRuleSet() != GameDb::kRuleSetDuel) {
        int nPosition = 0;
        for (int i = 0; i < nPlayers; ++i) {
            if (!TheGameDb->IsLocalPlayer(i)) {
                continue;
            }
            mLocals.push_back(std::make_unique<OvyLocalPlayer>(i, nPosition, pLayout));
            mAlls.push_back(std::make_unique<OvyAllPlayer>(i, nPosition, pLayout));
            ++nPosition;
        }
        for (int i = 0; i < nPlayers; ++i) {
            if (TheGameDb->IsLocalPlayer(i)) {
                continue;
            }
            mAlls.push_back(std::make_unique<OvyAllPlayer>(i, nPosition, pLayout));
            ++nPosition;
        }
    } else {
        // Yes, the binary copies this table of slots to the stack.
        const int anSlots[] = {0, 1, 2, 3};
        for (const int nSlot : anSlots) {
            for (int i = 0; i < nPlayers; ++i) {
                if (TheGameDb->GetPlayerSlot(i) != nSlot) {
                    continue;
                }
                if (TheGameDb->IsLocalPlayer(i)) {
                    mLocals.push_back(std::make_unique<OvyLocalPlayer>(i, nSlot, pLayout));
                }
                mAlls.push_back(std::make_unique<OvyAllPlayer>(i, nSlot, pLayout));
                break;
            }
        }
    }
    mCommon = std::make_unique<HudCommon>(pLayout, pConfig, pDefaults);
}

Overlay::~Overlay() {
    // Member destruction runs after HideablePanel::Terminate(). The panels are destroyed before it.
    mAlls.clear();
    mLocals.clear();
    mCommon.reset();
    ThePs.SetBlurActive(0);
    HideablePanel::Terminate();
}

bool Overlay::DispatchPriv(Message *pMsg) {
    (void)pMsg->Type(); // Yes, the binary discards this call's result.
    return false;
}

void Overlay::CreateTutorialParts() {
    auto *pLayout =
        dynamic_cast<RndView *>(TheManager.Find(FormatString("_hud%c.view", sHudLetter)));
    if (TheGameDb->GetTutorial() == 0) {
        return;
    }
    mCommon->SetController(std::make_unique<OvyController>(pLayout));
    if (TheGameDb->GetTutorial() == 0) {
        return;
    }
    mCommon->SetDialog(std::make_unique<OvyDialog>(pLayout));
}

OvyLocalPlayer *Overlay::FindLocal(int nPlayer) {
    for (const auto &pLocal : mLocals) {
        if (pLocal->GetPlayer() == nPlayer) {
            return pLocal.get();
        }
    }
    return nullptr;
}

OvyAllPlayer *Overlay::FindAll(int nPlayer) {
    for (const auto &pAll : mAlls) {
        if (pAll->GetPlayer() == nPlayer) {
            return pAll.get();
        }
    }
    return nullptr;
}

void Overlay::AddCheckpoint(float fPos, const char *pszLabel) {
    if (mCommon->GetSongPos() != nullptr) {
        mCommon->GetSongPos()->AddCheckpoint(fPos, pszLabel);
    }
}

void Overlay::ClearCheckpoints() {
    if (mCommon->GetSongPos() != nullptr) {
        mCommon->GetSongPos()->ClearCheckpoints();
    }
}

void Overlay::SetEnergy(float fLevel, int nColor) {
    mCommon->GetJuice().SetLevel(fLevel, nColor);
}

void Overlay::SetEnergyWarning(bool bWarn) {
    mCommon->GetJuice().SetWarning(bWarn);
}

void Overlay::SetScore(int nPlayer, int nScore) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll == nullptr) {
        return;
    }
    if (pAll->GetScore() != nullptr) {
        pAll->GetScore()->SetScore(nScore);
    } else if (pAll->GetDuelScore() != nullptr) {
        pAll->GetDuelScore()->SetLitCount(nScore);
    }
}

void Overlay::SetMultiplier(int nPlayer, int nMultiplier, int bHot, const char *pszText) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->GetPoints() != nullptr) {
        pLocal->GetPoints()->SetMultiplier(nMultiplier, bHot, pszText);
    }
}

void Overlay::ShowPoints(int nPlayer, int nPoints, const char *pszText) {
    if (mCommon->GetLetterExit() != nullptr) {
        mCommon->GetLetterExit()->ShowReveal(nPlayer, nPoints, pszText);
        return;
    }
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->GetPoints() != nullptr) {
        pLocal->GetPoints()->ShowPoints(nPoints, pszText);
    }
}

void Overlay::HideMultiplier(int nPlayer) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->GetPoints() != nullptr) {
        pLocal->GetPoints()->HideMultiplier();
    }
}

void Overlay::EndPoints(int nPlayer, int nResult) {
    if (mCommon->GetLetterExit() != nullptr) {
        mCommon->GetLetterExit()->Fly(nResult);
        return;
    }
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->GetPoints() != nullptr) {
        pLocal->GetPoints()->End(nResult);
    }
}

void Overlay::SetPlaying(int nPlayer, bool bPlaying) {
    mCommon->SetPlaying(nPlayer, bPlaying);
}

void Overlay::ShowPowerup(int nPlayer, int nPowerup) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll != nullptr && pAll->GetScore() != nullptr &&
        pAll->GetScore()->GetPowerup() != nullptr) {
        pAll->GetScore()->GetPowerup()->ShowPowerup(nPowerup);
    }
}

void Overlay::SetPowerupEnabled(int nPlayer, int nEnabled) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll != nullptr && pAll->GetScore() != nullptr &&
        pAll->GetScore()->GetPowerup() != nullptr) {
        HudPowerup *pPowerup = pAll->GetScore()->GetPowerup();
        pPowerup->SetEnabled(nEnabled);
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
    mCommon->GetTextMessage().Show(pszText, pszSmallText, fDuration, fScale, nPlayer, fX, fZ);
}

void Overlay::SetMessage(const char *pszText, bool bFirstLine) {
    if (bFirstLine) {
        mCommon->GetMessage().SetText(pszText);
    } else {
        mCommon->GetMessage2().SetText(pszText);
    }
}

void Overlay::SetLetterbox(bool bClosed) {
    mCommon->GetLetterbox().SetClosed(bClosed ? 1.0f : 0.0f);
}

float Overlay::GetLetterbox() {
    return mCommon->GetLetterbox().GetSlide().GetValue() * kLetterboxPerFrame;
}

void Overlay::SetPartsShown(bool bShow, [[maybe_unused]] int nUnused) {
    if (bShow && TheGameDb->GetRuleSet() == GameDb::kRuleSetRemix &&
        TheGameDb->GetRemixInfo()->mReadOnly != 0) {
        if (mCommon->GetSections() != nullptr) {
            mCommon->GetSections()->Show(false);
        }
        for (const auto &pLocal : mLocals) {
            if (pLocal->GetRemix() != nullptr) {
                static_cast<HideablePanel *>(pLocal->GetRemix())->Show(false);
            }
            if (pLocal->GetPoints() != nullptr) {
                pLocal->GetPoints()->SetEnabled(false);
            }
        }
        if (mCommon->GetSongPos() != nullptr) {
            mCommon->GetSongPos()->Show(true);
        }
        if (mCommon->GetLeaderAvatar() != nullptr) {
            mCommon->GetLeaderAvatar()->Show(true);
        }
        for (const auto &pAll : mAlls) {
            if (pAll->GetAvatar() != nullptr) {
                pAll->GetAvatar()->Show(true);
            }
            if (pAll->GetScore() != nullptr) {
                pAll->GetScore()->Show(false);
                if (pAll->GetScore()->GetPowerup() != nullptr) {
                    pAll->GetScore()->GetPowerup()->SetEnabled(0);
                    pAll->GetScore()->GetPowerup()->UpdateSlide();
                }
            }
            if (pAll->GetDuelScore() != nullptr) {
                pAll->GetDuelScore()->Show(false);
            }
        }
        mCommon->GetJuice().Show(false);
        return;
    }
    if (mCommon->GetSections() != nullptr) {
        mCommon->GetSections()->Show(bShow);
    }
    if (!bShow && mCommon->GetTrackLabel() != nullptr) {
        mCommon->GetTrackLabel()->Show(false);
    }
    for (const auto &pLocal : mLocals) {
        if (pLocal->GetRemix() != nullptr) {
            static_cast<HideablePanel *>(pLocal->GetRemix())->Show(bShow);
        }
        if (pLocal->GetPoints() != nullptr) {
            pLocal->GetPoints()->SetEnabled(bShow);
        }
    }
    const bool bPlayParts = TheGameDb->GetTutorial() != 0 ? false : bShow;
    if (mCommon->GetSongPos() != nullptr) {
        mCommon->GetSongPos()->Show(bPlayParts && TheGameDb->GetRuleSet() != GameDb::kRuleSetRemix);
    }
    if (mCommon->GetLeaderAvatar() != nullptr) {
        mCommon->GetLeaderAvatar()->Show(bPlayParts);
    }
    for (const auto &pAll : mAlls) {
        if (pAll->GetAvatar() != nullptr) {
            pAll->GetAvatar()->Show(bPlayParts);
        }
        if (pAll->GetScore() != nullptr) {
            pAll->GetScore()->Show(bPlayParts);
            if (pAll->GetScore()->GetPowerup() != nullptr) {
                pAll->GetScore()->GetPowerup()->SetEnabled(bPlayParts);
                pAll->GetScore()->GetPowerup()->UpdateSlide();
            }
        }
        if (pAll->GetDuelScore() != nullptr) {
            pAll->GetDuelScore()->Show(bPlayParts);
        }
    }
    mCommon->GetJuice().Show(TheGameDb->GetPracticeMode() == 0 ? bPlayParts : false);
}

void Overlay::ShowEnergy(bool bShow) {
    mCommon->GetJuice().Show(bShow);
}

void Overlay::ShowScore(int nPlayer, bool bShow) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll != nullptr) {
        if (pAll->GetScore() != nullptr) {
            pAll->GetScore()->Show(bShow);
        } else if (pAll->GetDuelScore() != nullptr) {
            pAll->GetDuelScore()->Show(bShow);
        }
    }
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->GetPoints() != nullptr) {
        pLocal->GetPoints()->SetEnabled(bShow);
    }
}

void Overlay::ShowBox(bool bShow) {
    mCommon->GetHilite().ShowBox(bShow);
}

void Overlay::ShowArrow(bool bShow) {
    mCommon->GetHilite().ShowArrow(bShow);
}

void Overlay::ShowAvatar(int nPlayer, bool bShow) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll != nullptr && pAll->GetAvatar() != nullptr) {
        pAll->GetAvatar()->Show(bShow);
    } else if (nPlayer == 0 && mCommon->GetLeaderAvatar() != nullptr) {
        mCommon->GetLeaderAvatar()->Show(bShow);
    }
}

void Overlay::ShowSongPos(bool bShow) {
    if (mCommon->GetSongPos() != nullptr) {
        mCommon->GetSongPos()->Show(bShow);
    }
}

void Overlay::ShowTrackLabel(bool bShow) {
    if (mCommon->GetTrackLabel() != nullptr) {
        mCommon->GetTrackLabel()->Show(bShow);
    }
}

void Overlay::SetFreqSize(int nPlayer, int nFreqSize) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll != nullptr && pAll->GetAvatar() != nullptr) {
        pAll->GetAvatar()->SetFreqSize(nFreqSize);
    }
}

void Overlay::SetTrack(int nPlayer, int nInstrument, int nTrackKind) {
    OvyAllPlayer *pAll = FindAll(nPlayer);
    if (pAll != nullptr && pAll->GetAvatar() != nullptr) {
        pAll->GetAvatar()->SetInstrument(nPlayer, nInstrument, nTrackKind);
    }
    if (mCommon->GetTrackLabel() != nullptr) {
        mCommon->GetTrackLabel()->SetTrack(nPlayer, nInstrument, nTrackKind);
    }
}

void Overlay::FlashPoints(int nPlayer, float fValue) {
    if (!(0.0f < fValue)) {
        return;
    }
    if (mCommon->GetLetterExit() != nullptr) {
        mCommon->GetLetterExit()->SetTime(fValue);
        return;
    }
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->GetPoints() != nullptr) {
        pLocal->GetPoints()->Flash(fValue);
    }
}

void Overlay::SetBoxRect(float fX0, float fY0, float fX1, float fY1, float fDuration) {
    mCommon->GetHilite().SetBoxRect(fX0, fY0, fX1, fY1, fDuration);
}

void Overlay::SetArrowTarget(float fX, float fY, float fAngle, float fDuration) {
    mCommon->GetHilite().SetArrowTarget(fX, fY, fAngle, fDuration);
}

void Overlay::FlyButton(int nLane, const Vector2 *pFrom, const Vector2 *pTo) {
    mCommon->GetButton(nLane).Fly(pFrom, pTo);
}

void Overlay::SetButtonPulse(int nLane, bool bPulse) {
    mCommon->GetButton(nLane).SetPulse(bPulse);
}

void Overlay::HideButton(int nLane) {
    mCommon->GetButton(nLane).Hide();
}

void Overlay::SetButtonGlyph(int nLane, const char *pszGlyph) {
    mCommon->GetButton(nLane).GetText()->SetText(pszGlyph);
}

void Overlay::ShowController(bool bShow, float fPosition) {
    OvyController *pController = mCommon->GetController();
    if (pController == nullptr) {
        return;
    }
    pController->Show(bShow);
    if (0.0f <= fPosition) {
        mCommon->GetController()->GetPosAnim()->SetFrame(fPosition);
    }
}

void Overlay::SetControllerHighlight(bool bHighlight) {
    if (mCommon->GetController() != nullptr) {
        mCommon->GetController()->SetHighlight(bHighlight);
    }
}

void Overlay::ShowControllerButton(int nButton, bool bShow) {
    if (mCommon->GetController() != nullptr) {
        mCommon->GetController()->ShowButton(nButton, bShow);
    }
}

void Overlay::OpenDialog(const char *pszText, float fSize, float fPosition) {
    if (mCommon->GetDialog() != nullptr) {
        mCommon->GetDialog()->Open(pszText, fSize, fPosition);
    }
}

void Overlay::CloseDialog() {
    if (mCommon->GetDialog() != nullptr) {
        mCommon->GetDialog()->Close();
    }
}

void Overlay::SetSectionCount(int nCount) {
    if (mCommon->GetSections() != nullptr) {
        mCommon->GetSections()->SetSectionCount(nCount);
    }
}

void Overlay::SetSectionLabel(int nIndex, const char *pszLabel) {
    if (mCommon->GetSections() != nullptr) {
        mCommon->GetSections()->SetSectionLabel(nIndex, pszLabel);
    }
}

void Overlay::ShowSections(bool bShow) {
    if (mCommon->GetSections() != nullptr) {
        mCommon->GetSections()->Show(bShow);
    }
}

void Overlay::HighlightSection(int nIndex) {
    if (mCommon->GetSections() != nullptr) {
        mCommon->GetSections()->HighlightSection(nIndex);
    }
}

void Overlay::SetRemixPanelText(int nPlayer, int nIndex, const char *pszText, int nPanel) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->GetRemix() != nullptr) {
        pLocal->GetRemix()->FindSubPanel(nPanel, 0)->SetText(nIndex, pszText);
    }
}

void Overlay::SelectRemixPanelEntry(int nPlayer, int nIndex, int nPanel) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->GetRemix() != nullptr) {
        pLocal->GetRemix()->FindSubPanel(nPanel, 0)->Select(nIndex);
    }
}

void Overlay::FlashRemixPanel(int nPlayer, int nPanel) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->GetRemix() != nullptr) {
        pLocal->GetRemix()->FindSubPanel(nPanel, 0)->Flash();
    }
}

void Overlay::SetRemixPanelLit(int nPlayer, int nIndex, int bLit, int nPanel) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->GetRemix() != nullptr) {
        pLocal->GetRemix()->FindSubPanel(nPanel, 0)->SetLit(nIndex, bLit);
    }
}

void Overlay::ShowRemixPanel(int nPlayer, int nPanel) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->GetRemix() != nullptr) {
        pLocal->GetRemix()->ShowSubPanel(nPanel);
    }
}

void Overlay::ShowRemix(int nPlayer, bool bShow) {
    OvyLocalPlayer *pLocal = FindLocal(nPlayer);
    if (pLocal != nullptr && pLocal->GetRemix() != nullptr) {
        static_cast<HideablePanel *>(pLocal->GetRemix())->Show(bShow);
    }
}

void Overlay::ShowStick(int nDirection, const Vector2 *pPosition) {
    if (mCommon->GetStick() != nullptr) {
        mCommon->GetStick()->Show(nDirection, pPosition);
    }
}

void Overlay::HideStick() {
    if (mCommon->GetStick() != nullptr) {
        mCommon->GetStick()->Hide();
    }
}

void Overlay::SetCameraOffset(const Vector3 *pOffset) {
    const float fX = pOffset->x + mCamPos.x;
    const float fY = pOffset->y + mCamPos.y;
    const float fZ = pOffset->z + mCamPos.z;
    Vector3 &current = mCam->GetLocalXfm().mTranslation;
    if (fX != current.x || fY != current.y || fZ != current.z) {
        mCam->SetDirty(1);
        // The binary also copies an unset fourth word, which is left as it was here.
        current.x = fX;
        current.y = fY;
        current.z = fZ;
    }
}

void Overlay::Reset() {
    mCommon->Reset();
    for (const auto &pLocal : mLocals) {
        pLocal->Reset();
    }
    for (const auto &pAll : mAlls) {
        pAll->Reset();
    }
    SetPartsShown(false, 0);
}

void Overlay::ClearStarted() {
    mStarted = 0;
}

void Overlay::ShowCheckpoint() {
    if (mAlls.size() < 2 || TheGameDb->GetRuleSet() != GameDb::kRuleSetGame) {
        return;
    }
    std::vector<OvyScore *> scores(OvyScore::kNumCheckpointPositions);
    scores.clear();
    int nMaxScore = 0;
    for (const auto &pAll : mAlls) {
        OvyScore *pScore = pAll->GetScore();
        if (pScore == nullptr) {
            continue;
        }
        scores.push_back(pScore);
        if (nMaxScore < pScore->GetScore()) {
            nMaxScore = pScore->GetScore();
        }
    }
    std::stable_sort(scores.begin(), scores.end(), [](OvyScore *pLeft, OvyScore *pRight) {
        return pLeft->GetScore() < pRight->GetScore();
    });
    if (!(0 < nMaxScore)) {
        nMaxScore = 1;
    }
    const int nCount = static_cast<int>(scores.size());
    for (int i = nCount - 1; i >= 0; --i) {
        OvyScore *pScore = scores[i];
        pScore->ShowCheckpoint(static_cast<float>(pScore->GetScore()) /
                                   static_cast<float>(nMaxScore),
                               nCount - (i + 1));
    }
}

void Overlay::ShowResult(bool bWon, [[maybe_unused]] int nMode) {
    if (!bWon) {
        mCommon->GetJuice().SetHideBar(1);
        return;
    }
    for (const auto &pLocal : mLocals) {
        if (pLocal->GetPoints() == nullptr) {
            continue;
        }
        pLocal->GetPoints()->SetMultiplier(1, 0, "");
        pLocal->GetPoints()->End(GfxManager::kPendingPointsCleared);
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
    if (mCommon->GetSongPos() != nullptr) {
        mCommon->GetSongPos()->SetFuture(fFuture);
    }
    for (const auto &pAll : mAlls) {
        pAll->Poll(fSongDelta, fRealDelta, fDelta);
    }
    for (const auto &pLocal : mLocals) {
        pLocal->Poll(fFrame, fSongDelta, fUnused, fRealDelta);
    }
    mCommon->Poll(fSongDelta, fRealDelta, fDelta);
}

void Overlay::Draw() {
    TheGfxManager.SetWinnerDrawn(0);
    static_cast<RndTransformable *>(mHudView)->UpdateWorldXfm(nullptr, 0);
    HideablePanel::PoseMaterials();
    static_cast<RndDrawable *>(mHudView)->Draw();
    mCommon->Draw();
    for (const auto &pLocal : mLocals) {
        pLocal->DrawHud();
    }
    for (const auto &pLocal : mLocals) {
        pLocal->DrawAvatar();
    }
    for (const auto &pAll : mAlls) {
        pAll->DrawHud();
    }
    mCommon->DrawAvatar();
    const int nWinner = TheGfxManager.GetWinner();
    for (const auto &pAll : mAlls) {
        if (pAll->GetPlayer() != nWinner) {
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
