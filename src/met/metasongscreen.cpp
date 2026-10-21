#include "met/metasongscreen.h"

#include <stdio.h>
#include <string.h>
#include <vector>

#include "game/campaign.h"
#include "game/gamedb.h"
#include "game/songstats.h"
#include "met/metagame.h"
#include "met/metagameutil.h"
#include "met/songpicpanel.h"
#include "met/songpreview.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/system.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmesh.h"
#include "rnd/rndtext.h"
#include "rnd/rndview.h"
#include "synth/fxmidi.h"
#include "ui/uibutton.h"
#include "ui/uilabel.h"
#include "ui/uimanager.h"

namespace {

#ifdef VIDEO_STANDARD_PAL
// Spanish calls the tutorial songs training songs.
inline bool IsSpanish() {
    return strcmp(GetSystemLanguage(), "spanish") == 0;
}
#endif

// The song types of the "songs" section.
constexpr int kSongTypeBoss = 1;
constexpr int kSongTypeBonus = 2;
constexpr int kSongTypeSecret = 3;
constexpr int kSongTypeTutorial = 4;

// The GameDb::GetArenaSongs() filter of the song screen.
constexpr int kArenaSongFilter = 1;

// The value of mPreviewMixTime while no time is set.
constexpr float kNoMixTime = -1.0f;

// The delay between the end of the song preview and the deferred exit, in milliseconds.
constexpr float kExitDelayMs = 100.0f;

// The delay before the idle mix of the song screen, in milliseconds.
constexpr float kIdleMixDelayMs = 3000.0f;

// The fade of a song clip that plays when the screen enters, in seconds.
constexpr float kEnterFadeSeconds = 0.05f;

// The size of the buffer of a two-digit button index.
constexpr int kIndexBufferSize = 8;

// The rule set and community of the practice mode.
constexpr int kPracticeCommunity = GameDb::kCommunitySolo;
constexpr int kPracticeRuleSet = GameDb::kRuleSetGame;

} // namespace

#ifdef VIDEO_STANDARD_PAL
MetaSongScreen::MetaSongScreen(DataArray *pData)
    : FreqScreen(pData), mSong(static_cast<const char *>(nullptr)),
      mExitScreenName(static_cast<const char *>(nullptr)) {
    mPreviewMixTime = kNoMixTime;
    mBandPanel = nullptr;
    mExitPending = 0;
    mSongChosen = 0;
    mExitStart = 0.0f;
}
#else
MetaSongScreen::MetaSongScreen(DataArray *pData)
    : FreqScreen(pData), mSong(static_cast<const char *>(nullptr)) {
    mPreviewMixTime = kNoMixTime;
    mBandPanel = nullptr;
    mExitScreen = nullptr;
    mSongChosen = 0;
    mExitTime = 0.0f;
    mExitPending = 0;
    mExitStart = 0.0f;
}
#endif

bool MetaSongScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void MetaSongScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    if (mExitPending != 0) {
        if (SongPreview::IsIdle() && mExitStart == 0.0f) {
            mExitStart = SystemMs() + kExitDelayMs;
            return;
        }
        if (mExitStart < SystemMs() && 0.0f < mExitStart) {
            (void)SongPreview::IsIdle(); // Yes, the binary discards this call's result.
#ifdef VIDEO_STANDARD_PAL
            TheUI.GotoScreen(mExitScreenName.c_str());
            mExitStart = 0.0f;
            mExitPending = 0;
            mExitScreenName = "";
#else
            FreqScreen::Exit(mExitScreen, mExitTime);
            mExitStart = 0.0f;
            mExitPending = 0;
#endif
        }
        return;
    }

    if (!SongPreview::IsIdle() || !SongPreview::IsWaiting()) {
        return;
    }
    SongPicPanel *pPicture =
        static_cast<SongPicPanel *>(TheUI.FindPanel("s_g_sel_song_pic", false));
    if (mPreviewSong.GetLength() != 0) {
        if (pPicture->GetPictureReady() == 0) {
            return;
        }
        TheMetagame.SetSongScreenMix(false);
        SongPreview::Load(mPreviewSong.c_str(), true);
        mPreviewSong.Clear();
        mPreviewMixTime = kNoMixTime;
        return;
    }
    if (mPreviewMixTime < 0.0f) {
        mPreviewMixTime = SystemMs() + kIdleMixDelayMs;
    }
    if (mPreviewMixTime < SystemMs()) {
        TheMetagame.SetSongScreenMix(true);
    }
}

#ifdef VIDEO_STANDARD_PAL
void MetaSongScreen::GotoScreenAfterPreview(const char *pszScreen) {
    mExitScreenName = pszScreen;
    SongPreview::End(mSongChosen == 0);
    mExitPending = 1;
}
#else
void MetaSongScreen::Exit(UIScreen *pNextScreen, float fTime) {
    SongPreview::End(mSongChosen == 0);
    mExitTime = fTime;
    mExitPending = 1;
    mExitScreen = pNextScreen;
}
#endif

const char *MetaSongScreen::Title() {
    if (TheGameDb->GetTutorial() != 0) {
        return TheLocale.Localize("training_select_TITLE", true);
    }
    const char *pszFormat = FreqScreen::Title();
    const char *pszDifficulty = TheGameDb->GetDifficultyName();
    const char *pszMode = TheGameDb->GetModeName();
    if (TheGameDb->GetTutorial() != 0) {
        pszMode = TheLocale.Localize("mode_tutorial", false);
    }
    return FormatString(pszFormat, pszMode, pszDifficulty, TheMetagame.SelectedArenaName());
}

SongEntry MetaSongScreen::FindSong(UIComponent *pButton) {
    std::vector<SongEntry> songs;
    TheGameDb->GetArenaSongs(
        &songs, TheMetagame.GetSelectedArena(), TheGameDb->GetSkillLevel(), kArenaSongFilter);
    for (unsigned int i = 0; i < songs.size(); ++i) {
        if (strcmp(pButton->GetName(), FormatString("0%d", i + 1)) == 0) {
            return songs[i];
        }
    }
    DebugWarn("Couldn't determine song: %s", pButton->Text());
    return SongEntry{nullptr};
}

void MetaSongScreen::UpdatePreview(UIComponent *pButton, UIPanel *pPanel) {
    if (pPanel != TheUI.FindPanel("s_g_sel_song_band", false) || pButton == nullptr) {
        return;
    }

    SongEntry song = FindSong(pButton);
    bool bLocked = false;
    UIButton *pSongButton = dynamic_cast<UIButton *>(pButton);
    if (pSongButton != nullptr) {
        bLocked = pSongButton->GetStyle() == TheUI.FindStyle("locked_song_style", false);
    }
    RndText *pBio = dynamic_cast<RndText *>(TheManager.Find("s_g_sel_song_pic_bio.txt"));
    RndText *pGenre = dynamic_cast<RndText *>(TheManager.Find("s_g_sel_song_pic_genre.txt"));
    RndText *pStatus = dynamic_cast<RndText *>(TheManager.Find("s_g_sel_song_pic_lock.txt"));
    if (bLocked) {
        if (song.GetType() == kSongTypeBonus) {
            pBio->SetText(TheLocale.Localize("locked_bonus_bio", true));
        } else if (song.GetType() == kSongTypeBoss) {
            pBio->SetText(TheLocale.Localize("locked_boss_bio", true));
        } else if (song.GetType() == kSongTypeTutorial) {
#ifdef VIDEO_STANDARD_PAL
            pBio->SetText(
                TheLocale.Localize(IsSpanish() ? "locked_label_train" : "locked_label", true));
#else
            pBio->SetText(TheLocale.Localize("locked_label", true));
#endif
        }
        pGenre->SetShowing(false);
#ifdef VIDEO_STANDARD_PAL
        // Yes, Spanish labels every locked song as a training song.
        pStatus->SetText(
            TheLocale.Localize(IsSpanish() ? "locked_label_train" : "locked_label", true));
#else
        pStatus->SetText(TheLocale.Localize("locked_label", true));
#endif
    } else {
        pBio->SetText(song.GetBio());
        pGenre->SetShowing(true);
        pGenre->SetText(song.GetGenre());
        if (TheGameDb->GetCommunity() == GameDb::kCommunitySolo &&
            TheGameDb->GetProfile(0)->IsSongFinished(song.GetName(), TheGameDb->GetSkillLevel())) {
            pStatus->SetText(TheLocale.Localize("beat_label", true));
#ifdef VIDEO_STANDARD_PAL
        } else if (IsSpanish() && song.GetType() == kSongTypeTutorial) {
            pStatus->SetText(TheLocale.Localize("unlocked_label_train", true));
#endif
        } else {
            pStatus->SetText(TheLocale.Localize("unlocked_label", true));
        }
    }

    RndText *pArtist = dynamic_cast<RndText *>(TheManager.Find("s_g_sel_song_pic_bio_01.txt"));
    if (bLocked) {
        pArtist->SetText("");
    } else {
        pArtist->SetText(song.GetArtist());
    }
    if (strcmp(song.GetName(), "POD") == 0) {
        pArtist->SetText(song.GetArtistShort());
    }
    RndText *pWww = dynamic_cast<RndText *>(TheManager.Find("s_g_sel_song_pic_bio_02.txt"));
    if (bLocked) {
        pWww->SetText("");
    } else {
        pWww->SetText(song.GetWww());
    }

    SongPicPanel *pPicture =
        static_cast<SongPicPanel *>(TheUI.FindPanel("s_g_sel_song_pic", false));
    const char *pszSong = song.GetName();
    const bool bEncrypted = bLocked && song.GetType() != kSongTypeTutorial;
    pPicture->SetBandPicture(pszSong, false, !bLocked, bEncrypted);
    SongPreview::FadeOut();

    if (!bLocked) {
        mPreviewSong = song.GetName();
    } else if (song.GetType() == kSongTypeBonus) {
#ifdef VIDEO_STANDARD_PAL
        mPreviewSong = SongPreview::BonusEncrypt();
#else
        mPreviewSong = SongPreview::sBonusEncrypt;
#endif
    } else if (song.GetType() == kSongTypeBoss || song.GetType() == kSongTypeTutorial) {
#ifdef VIDEO_STANDARD_PAL
        mPreviewSong = SongPreview::LevelEncrypt();
#else
        mPreviewSong = SongPreview::sLevelEncrypt;
#endif
    } else {
        DebugWarn("I don't think we should get here.\n");
    }

    if (song.GetType() == kSongTypeTutorial) {
        TheMetagame.SetActionText(TheLocale.Localize("s_g_sel_song_band_tut_ACTION", true));
        TheMetagame.SetHelpText(TheLocale.Localize("s_g_sel_song_band_tut_HELP", true));
    } else if (TheGameDb->GetCommunity() == GameDb::kCommunityLocal) {
        TheMetagame.SetActionText(TheLocale.Localize("m_g_sel_song_band_ACTION", true));
        TheMetagame.SetHelpText(TheLocale.Localize("s_g_sel_song_band_song_HELP", true));
    } else {
        TheMetagame.SetActionText(TheLocale.Localize("s_g_sel_song_band_song_ACTION", true));
        TheMetagame.SetHelpText(TheLocale.Localize("s_g_sel_song_band_song_HELP", true));
    }

    for (int i = 1; i <= kNumSongButtons; ++i) {
        char szIndex[kIndexBufferSize];
        sprintf(szIndex, "%02d", i);
        UIComponent *pScore =
            TheUI.FindComponent("s_g_sel_song_band", FormatString("hi_%s", szIndex), false);
        if (strcmp(pButton->GetName(), szIndex) == 0) {
            pScore->SetState(UIComponent::kStateSelected, false);
        } else {
            pScore->SetState(UIComponent::kStateNormal, false);
        }
    }
}

void MetaSongScreen::ShowPicture(bool bShowPicture) {
    RndView *pBio = dynamic_cast<RndView *>(TheManager.Find("s_g_sel_song_pic_bio.view"));
    RndView *pImage = dynamic_cast<RndView *>(TheManager.Find("s_g_sel_song_pic_image.view"));
    const char *pszFirst;
    const char *pszSecond;
    if (strcmp(TheMetagame.GetSelectedArena(), "Tutorial") == 0) {
        pszFirst = TheLocale.Localize("info_label", true);
        pszSecond = TheLocale.Localize("program_label", true);
    } else {
        pszFirst = TheLocale.Localize("s_g_sel_song_pic_label_01", true);
        pszSecond = TheLocale.Localize("s_g_sel_song_pic_label_02", true);
    }

    UIPanel *pPanel = TheUI.FindPanel("s_g_sel_song_pic", false);
    if (bShowPicture) {
        pBio->SetShowing(false);
        pImage->SetShowing(true);
        pPanel->FindComponent("label_01", false)->SetText(pszSecond);
        pPanel->FindComponent("label_02", false)->SetText(pszFirst);
    } else {
        pBio->SetShowing(true);
        pImage->SetShowing(false);
        pPanel->FindComponent("label_01", false)->SetText(pszFirst);
        pPanel->FindComponent("label_02", false)->SetText(pszSecond);
    }
}

void MetaSongScreen::SetUpSongButton(SongEntry *pSong, int nIndex, bool bLocked) {
    const int nNumber = nIndex + 1;
    UIButton *pButton =
        static_cast<UIButton *>(mBandPanel->FindComponent(FormatString("%02d", nNumber), false));
    UILabel *pScore =
        static_cast<UILabel *>(mBandPanel->FindComponent(FormatString("hi_%02d", nNumber), false));
    RndMesh *pGrade = dynamic_cast<RndMesh *>(
        TheManager.Find(FormatString("s_g_sel_song_band_g_%02d.mesh", nNumber)));
    pButton->SetState(UIComponent::kStateNormal, false);
    pButton->SetShowing(true);

    const bool bFinished =
        TheGameDb->GetProfile(0)->IsSongFinished(pSong->GetName(), TheGameDb->GetSkillLevel());
    if (pSong->GetType() != kSongTypeTutorial &&
        TheGameDb->GetCommunity() == GameDb::kCommunitySolo && bFinished) {
        SongStats record;
        TheGameDb->GetProfile(0)->GetSongStats(
            pSong->GetName(), TheGameDb->GetSkillLevel(), &record);
        String token(FormatString("%s_%s", mBandPanel->GetName(), pScore->GetName()));
        pScore->SetText(FormatString(TheLocale.Localize(token.c_str(), true), record.mScore));
        pScore->SetShowing(true);
        pGrade->SetShowing(true);
        pGrade->SetMat(FindGradeMaterial(
            TheGameDb->GetProfile(0)->GetMedal(pSong->GetName(), TheGameDb->GetSkillLevel()),
            false));
    } else {
        pScore->SetText("");
        pScore->SetShowing(false);
        pGrade->SetShowing(false);
    }

    UIStyle *pStyle;
    if (!bLocked) {
        pStyle = TheUI.FindStyle("beat_song_style", false);
        if (TheGameDb->GetCommunity() == GameDb::kCommunitySolo && bFinished) {
            pStyle = TheUI.FindStyle("beat_song_style", false);
        } else {
            pStyle = TheUI.FindStyle("unlocked_song_style", false);
        }
    } else {
        pStyle = TheUI.FindStyle("locked_song_style", false);
        switch (pSong->GetType()) {
        case kSongTypeBoss:
            pScore->SetStyle(pStyle);
            pButton->SetStyle(pStyle);
            pButton->SetText(TheLocale.Localize("locked_boss_bio", true));
            return;
        case kSongTypeBonus:
            pScore->SetStyle(pStyle);
            pButton->SetStyle(pStyle);
            pButton->SetText(TheLocale.Localize("locked_bonus_bio", true));
            return;
        case kSongTypeSecret:
            pButton->SetState(UIComponent::kStateDisabled, false);
            pButton->SetText("");
            return;
        case kSongTypeTutorial:
            break;
        default:
            DebugWarn(" Greyed out song that isn't boss, bonus, secret, or tut");
            return;
        }
    }
    pScore->SetStyle(pStyle);
    pButton->SetStyle(pStyle);
    pButton->SetText(pSong->GetArtist());
}

void MetaSongScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    (void)SongPreview::IsPlaying(); // Yes, the binary discards this call's result.
    SongPreview::Stop(kEnterFadeSeconds);
    mBandPanel = TheUI.FindPanel("s_g_sel_song_band", false);
    const char *pszArena = TheMetagame.GetSelectedArena();
    const int nSkillLevel = TheGameDb->GetSkillLevel();
    std::vector<SongEntry> songs;
    TheGameDb->GetArenaSongs(&songs, pszArena, nSkillLevel, kArenaSongFilter);
    std::vector<SongEntry> unlocked;
    TheGameDb->GetUnlockedSongs(
        &unlocked, pszArena, nSkillLevel, TheGameDb->GetCommunity() == GameDb::kCommunitySolo);

    UIComponent *pFocus = nullptr;
    int i = 0;
    for (; i < static_cast<int>(songs.size()); ++i) {
        bool bLocked = true;
        for (unsigned int j = 0; j < unlocked.size(); ++j) {
            if (songs[i].GetName() == unlocked[j].GetName()) {
                bLocked = false;
                break;
            }
        }
        SetUpSongButton(&songs[i], i, bLocked);
        if (pFocus == nullptr && !bLocked &&
            !TheGameDb->GetProfile(0)->IsSongFinished(songs[i].GetName(), nSkillLevel)) {
            pFocus = mBandPanel->FindComponent(FormatString("%02d", i + 1), false);
        }
    }
    for (; i < kNumSongButtons; ++i) {
        UIComponent *pButton = mBandPanel->FindComponent(FormatString("0%d", i + 1), false);
        pButton->SetText("");
        pButton->SetState(UIComponent::kStateDisabled, false);
        pButton->SetShowing(false);
        UIComponent *pScore = mBandPanel->FindComponent(FormatString("hi_0%d", i + 1), false);
        pScore->SetText("");
        pScore->SetShowing(false);
        RndMesh *pGrade = dynamic_cast<RndMesh *>(
            TheManager.Find(FormatString("s_g_sel_song_band_g_%02d.mesh", i + 1)));
        pGrade->SetShowing(false);
    }
    if (pFocus == nullptr) {
        pFocus = mBandPanel->FindComponent("01", false);
    }
    mBandPanel->SetFocus(pFocus, kPadNone);
    FreqScreen::Enter(pPrevScreen, fTime);
    ShowPicture(true);

    RndText *pBeat = dynamic_cast<RndText *>(TheManager.Find("s_g_sel_song_band_07.txt"));
    RndText *pHigh = dynamic_cast<RndText *>(TheManager.Find("s_g_sel_song_band_hi_07.txt"));
    RndText *pLabel = dynamic_cast<RndText *>(TheManager.Find("s_g_sel_song_band_label.txt"));
    if (TheGameDb->GetCommunity() == GameDb::kCommunitySolo && strcmp(pszArena, "Tutorial") != 0) {
        const int nTarget = TheGameDb->GetProfile(0)->GetArenaScoreToBeat(pszArena, nSkillLevel);
        pBeat->SetText(FormatString(TheLocale.Localize("s_g_sel_song_band_07", true), nTarget));
        const int nScore = TheGameDb->GetProfile(0)->GetArenaScore(pszArena, nSkillLevel);
        pHigh->SetText(FormatString(TheLocale.Localize("s_g_sel_song_band_hi_07", true), nScore));
        pLabel->SetText(TheLocale.Localize("s_g_sel_arena_band_label", true));
    } else {
        pBeat->SetText("");
        pHigh->SetText("");
        pLabel->SetText(TheLocale.Localize("programs_label", true));
    }
}

bool MetaSongScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    mSongChosen = 0;
    if (mExitPending != 0) {
        return true;
    }
    if (pMsg->GetButton() != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }

    mSongChosen = 1;
    UIButton *pButton = dynamic_cast<UIButton *>(pMsg->GetComponent());
    if (pButton->GetStyle() == TheUI.FindStyle("locked_song_style", false)) {
        return UIScreen::HandleSelect(pMsg);
    }
    SongEntry song = FindSong(pMsg->GetComponent());
    DebugPrint("going to launch\n");
    TheGameDb->SetSong(song.GetName());
    TheGameDb->SetPracticeMode(false);
    TheGameDb->SetLoadRemix(false);
    if (song.GetType() == kSongTypeTutorial) {
        TheMetagame.SetSelectedArena("No arena");
        TheGameDb->SetTutorial(1);
    } else {
        TheGameDb->SetTutorial(0);
        TheGameDb->SetRuleSet(GameDb::kRuleSetGame);
    }
    mSong = song.GetName();

    if (TheGameDb->GetCommunity() != GameDb::kCommunitySolo) {
        GotoNextScreen("pre_multi2launchseq");
    } else if (TheGameDb->GetTutorial() == 1) {
        GotoNextScreen("pre_solotut2launchseq");
    } else {
        const char *pszPowerup =
            TheGameDb->GetProfile(0)->TakePendingPowerup(TheGameDb->GetSkillLevel());
        if (pszPowerup != nullptr) {
            GotoNextScreen(FormatString("%s_tip", pszPowerup));
        } else {
            GotoNextScreen("pre_launchpad2launchseq");
        }
    }
    return UIScreen::HandleSelect(pMsg);
}

bool MetaSongScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->GetPressed() != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    mSongChosen = 0;
    if (mExitPending != 0) {
        return true;
    }
    if (pMsg->GetPressed() == 0) {
        return FreqScreen::HandleJoypad(pMsg);
    }
    if (mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }

    if (pMsg->GetButton() == kPadSquare && TheGameDb->GetCommunity() == kPracticeCommunity &&
        TheGameDb->GetRuleSet() == kPracticeRuleSet) {
        FxMidi::PlaySquare();
        UIComponent *pFocus = mBandPanel->GetFocus();
        SongEntry song = FindSong(pFocus);
        bool bUnlocked = true;
        UIButton *pButton = dynamic_cast<UIButton *>(pFocus);
        if (pButton != nullptr) {
            bUnlocked = pButton->GetStyle() != TheUI.FindStyle("locked_song_style", false);
        }
        if (song.GetType() != kSongTypeTutorial && bUnlocked) {
            DebugPrint("going to launch - practice\n");
            TheGameDb->SetPracticeMode(true);
            TheGameDb->SetTutorial(0);
            TheGameDb->SetLoadRemix(false);
            TheGameDb->SetSong(song.GetName());
            mSong = song.GetName();
            GotoNextScreen("pre_launchpad2launchseq");
        }
    } else if (pMsg->GetButton() == kPadCircle) {
        (void)TheUI.FindPanel("s_g_sel_song_band", false); // Yes, the binary discards these.
        (void)FindSong(dynamic_cast<UIButton *>(mBandPanel->GetFocus()));
        (void)TheUI.FindPanel("s_g_sel_song_pic", false);
        RndView *pBio = dynamic_cast<RndView *>(TheManager.Find("s_g_sel_song_pic_bio.view"));
        ShowPicture(pBio->GetShowing() != 0);
    } else if (pMsg->GetButton() == kPadTriangle) {
        if (TheGameDb->GetCommunity() != GameDb::kCommunitySolo) {
            GotoNextScreen("multisong2multiarena");
        } else if (strcmp(TheMetagame.SelectedArenaName(), "Tutorial") == 0) {
            GotoNextScreen("solotut2soloarena");
        } else {
            GotoNextScreen("solosong2soloarena");
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool MetaSongScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    UpdatePreview(pMsg->GetComponent(), pMsg->GetPanel());
    return false;
}

bool MetaSongScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (pMsg->GetScreen() == this) {
        UpdatePreview(mFocusPanel->GetFocus(), mFocusPanel);
    }
    return false;
}
