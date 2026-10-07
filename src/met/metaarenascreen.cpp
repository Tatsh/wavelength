#include "met/metaarenascreen.h"

#include <algorithm>
#include <string.h>
#include <vector>

#include "game/gamedb.h"
#include "game/gamefx.h"
#include "game/playerprofile.h"
#include "game/songentry.h"
#include "met/metagame.h"
#include "met/songpreview.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "rnd/manager.h"
#include "rnd/text.h"
#include "synth/synth.h"
#include "ui/uibutton.h"
#include "ui/uimanager.h"

namespace {

// The song types of the "songs" section that the band panel labels as locked.
constexpr int kSongTypeBoss = 1;
constexpr int kSongTypeBonus = 2;
constexpr int kSongTypeSecret = 3;
constexpr int kSongTypeTutorial = 4;

// The GameDb::GetArenaSongs() filter of the arena screen.
constexpr int kArenaSongFilter = 1;

} // namespace

MetaArenaScreen::MetaArenaScreen(DataArray *pData)
    : FreqScreen(pData), mFocusArena(nullptr), mChangePending(0) {
}

bool MetaArenaScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void MetaArenaScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    mPendingScreen.Clear();
    std::vector<const char *> arenas;
    mChangePending = 0;
    const int nSkillLevel = TheGameDb->mSkillLevel;
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        TheGameDb->GetUnlockedArenas(&arenas, nSkillLevel, true, true);
    } else {
        TheGameDb->GetUnlockedArenas(&arenas, nSkillLevel, false, false);
        if (arenas.empty()) {
            std::vector<const char *> allArenas;
            TheGameDb->GetArenaNames(&allArenas, false);
            arenas.push_back(allArenas[0]);
        }
    }

    UIPanel *pPanel = TheUI.FindPanel("s_g_sel_arena", false);
    UIComponent *pFocus = pPanel->mFocus;
    for (const auto &entry : pPanel->mComponents) {
        const char *pszArena = entry.first;
        UIComponent *pButton = entry.second;
        std::vector<SongEntry> songs;
        TheGameDb->GetArenaSongs(&songs, pszArena, nSkillLevel, kArenaSongFilter);
        if (songs.empty() || (TheGameDb->mCommunity != GameDb::kCommunitySolo &&
                              strcmp(pszArena, "Tutorial") == 0)) {
            pButton->SetState(UIComponent::kStateDisabled, false);
        } else {
            pButton->SetState(UIComponent::kStateNormal, false);
        }
        if (std::find(arenas.begin(), arenas.end(), pszArena) == arenas.end()) {
            dynamic_cast<UIButton *>(pButton)->SetStyle(
                TheUI.FindStyle("button_style_grey", false));
        }
        if (strcmp(pPrevScreen->mName, "solosong2soloarena") == 0) {
            if (strcmp(pszArena, TheMetagame.mSelectedArena.c_str()) == 0) {
                pFocus = pButton;
            }
        } else if (!arenas.empty() && pszArena == arenas.back()) {
            pFocus = pButton;
        }
    }
    pPanel->SetFocus(pFocus, kPadNone);
    mFocusArena = pFocus;
    FreqScreen::Enter(pPrevScreen, fTime);
}

void MetaArenaScreen::UpdateBand(UIComponent *pArena, UIPanel *pPanel) {
    if (pArena == nullptr || strcmp(pPanel->mName, "s_g_sel_arena") != 0) {
        return;
    }

    const char *pszArena = pArena->mName;
    const int nSkillLevel = TheGameDb->mSkillLevel;
    std::vector<SongEntry> songs;
    TheGameDb->GetArenaSongs(&songs, pszArena, nSkillLevel, kArenaSongFilter);
    if (!songs.empty()) {
        std::vector<SongEntry> unlocked;
        TheGameDb->GetUnlockedSongs(
            &unlocked, pszArena, nSkillLevel, TheGameDb->mCommunity == GameDb::kCommunitySolo);
        UIPanel *pBand = TheUI.FindPanel("s_g_sel_arena_band", false);
        unsigned int i = 0;
        for (const auto &entry : pBand->mComponents) {
            UIComponent *pLabel = entry.second;
            if (i >= songs.size()) {
                pLabel->SetText("");
                ++i;
                continue;
            }
            SongEntry song = songs[i];
            ++i;
            unsigned int j = 0;
            for (; j < unlocked.size(); ++j) {
                if (song.GetName() == unlocked[j].GetName()) {
                    break;
                }
            }
            if (j != unlocked.size()) {
                pLabel->SetText(song.GetArtistShort());
            } else if (song.GetType() == kSongTypeBoss) {
                pLabel->SetText(TheLocale.Localize("locked_boss_bio", true));
            } else if (song.GetType() == kSongTypeBonus) {
                pLabel->SetText(TheLocale.Localize("locked_bonus_bio", true));
            } else if (song.GetType() == kSongTypeSecret) {
                pLabel->SetText("");
            } else if (song.GetType() != kSongTypeTutorial) {
                DebugWarn(" Greyed out song that isn't boss, bonus, secret, or tut");
            } else {
                pLabel->SetText(song.GetArtistShort());
            }
        }

        if (TheGameDb->mCommunity == GameDb::kCommunitySolo && strcmp(pszArena, "Tutorial") != 0) {
            UIComponent *pHigh = pBand->FindComponent("score_hi", false);
            const int nScore = TheGameDb->GetProfile(0)->GetArenaScore(pszArena, nSkillLevel);
            pHigh->SetText(
                FormatString(TheLocale.Localize("s_g_sel_arena_band_score_hi", true), nScore));
            UIComponent *pBeat = pBand->FindComponent("score_beat", false);
            const int nTarget =
                TheGameDb->GetProfile(0)->GetArenaScoreToBeat(pszArena, nSkillLevel);
            pBeat->SetText(
                FormatString(TheLocale.Localize("s_g_sel_arena_band_score_beat", true), nTarget));
        } else {
            pBand->FindComponent("score_hi", false)->SetText("");
            pBand->FindComponent("score_beat", false)->SetText("");
        }
    }

    Rnd::Text *pLabel =
        dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find("s_g_sel_arena_band_label.txt"));
    if (strcmp(pszArena, "Tutorial") == 0) {
        pLabel->SetText(TheLocale.Localize("programs_label", true));
    } else {
        pLabel->SetText(TheLocale.Localize("s_g_sel_arena_band_label", true));
    }
}

bool MetaArenaScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    UIComponent *pArena = pMsg->mComponent;
    UpdateBand(pArena, pMsg->mPanel);
    if (pArena != nullptr) {
        String arena(pArena->mName);
        if (arena != "Tutorial") {
            if (pArena != mFocusArena) {
                GameFx::PlayLazySusan();
            }
            mFocusArena = pArena;
        }
    }
    return false;
}

bool MetaArenaScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    UIButton *pButton = dynamic_cast<UIButton *>(pMsg->mComponent);
    if (mChangePending != 0) {
        return true;
    }
    if (pButton->mStyle == TheUI.FindStyle("button_style_grey", false) ||
        pMsg->mButton != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }

    TheSynth->SetOutputLevel(0.0f);
    TheMetagame.mSelectedArena = pMsg->mComponent->mName;
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        if (strcmp(pMsg->mComponent->mName, "Tutorial") == 0) {
            TheGameDb->SetTutorial(1);
            mPendingScreen = "soloarena2solotut";
        } else {
            TheGameDb->SetTutorial(0);
            mPendingScreen = "soloarena2solosong";
        }
    } else {
        if (strcmp(pMsg->mComponent->mName, "Tutorial") == 0) {
            return UIScreen::HandleSelect(pMsg);
        }
        mPendingScreen = "multiarena2multisong";
    }
    mChangePending = 1;
    return UIScreen::HandleSelect(pMsg);
}

void MetaArenaScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    if (mChangePending != 0 && SongPreview::IsPlaying()) {
        TheUI.GotoScreen(mPendingScreen.c_str());
        mChangePending = 0;
        mPendingScreen.Clear();
    }
}

const char *MetaArenaScreen::Title() {
    const char *pszFormat = FreqScreen::Title();
    const char *pszDifficulty = TheGameDb->GetDifficultyName();
    return FormatString(pszFormat, TheGameDb->GetModeName(), pszDifficulty);
}

bool MetaArenaScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (mChangePending != 0) {
        return true;
    }
    if (pMsg->mPressed == 0 || pMsg->mButton != kPadTriangle) {
        return false;
    }

    TheMetagame.mSelectedArena = "";
    const char *pszScreen;
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        pszScreen = "soloarena2soloskill";
    } else if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
        pszScreen = "m_r_mode";
    } else {
        pszScreen = "multiarena2multiskill";
    }
    TheUI.GotoScreen(TheUI.FindScreen(pszScreen, false));
    return false;
}
