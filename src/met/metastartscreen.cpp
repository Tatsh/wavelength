#include "met/metastartscreen.h"

#include "game/campaign.h"
#include "game/gamedb.h"
#include "math/rand.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/manager.h"
#include "rnd/text.h"
#include "synth/fxmidi.h"
#include "ui/uimanager.h"

namespace {

// The value of mAttractTime while the attract demo is disarmed.
constexpr float kAttractDisarmed = -1.0f;

// The value of the attract song counter before the first demo.
constexpr int kNoAttractSong = -1;

// The power-up level of an attract demo.
constexpr int kAttractPowerupLevel = 1;

// The controller whose cross and START buttons play the choice sound.
constexpr int kFirstPad = 0;

} // namespace

MetaStartScreen::MetaStartScreen(DataArray *pData) : FreqScreen(pData) {
    mAttractTime = kAttractDisarmed;
    mMusicView = nullptr;
}

void MetaStartScreen::Poll(float fTime) {
    FreqScreen::Poll(fTime);
    if (mMusicView != nullptr) {
        mMusicView->SetFrame(TheGameDb->mSongTick);
    }
    if (mAttractTime > 0.0f && mAttractTime < SystemMs()) {
        LaunchAttract();
    }
}

void MetaStartScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    DataArray *pVersion = DataArray::Read("version.txt", nullptr);
    Rnd::Text *pText = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find("start_version.txt"));
    if (pText != nullptr) {
        pText->SetText(""); // Yes, the binary blanks the text after reading the version file.
    }
    pVersion->Release();
    mMusicView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find("logo_music_anim.view"));
    FreqScreen::Enter(pPrevScreen, fTime);
}

void MetaStartScreen::Exit(UIScreen *pNextScreen, float fTime) {
    mMusicView = nullptr;
    mAttractTime = kAttractDisarmed;
    FreqScreen::Exit(pNextScreen, fTime);
}

void MetaStartScreen::LaunchAttract() {
    // NTSC-U/C: 0x003af88c
    static int sAttractSong = kNoAttractSong;

    int nEnabled = 0;
    DataArray *pMetagame = SystemConfig()->FindArray("metagame", false);
    pMetagame->FindBool("attract_enabled", &nEnabled, false);
    if (nEnabled == 0) {
        return;
    }

    DataArray *pSongs = pMetagame->FindArray("attract_songs", false);
    if (sAttractSong == kNoAttractSong) {
        sAttractSong = RandomInt(1, pSongs->Size());
    } else if (++sAttractSong >= pSongs->Size()) {
        sAttractSong = 1;
    }
    DataArray *pSong = pSongs->Array(sAttractSong);

    const char *pszSong = nullptr;
    const char *pszFile = nullptr;
    int nSkillLevel = -1;
    int nNumPlayers = -1;
    pSong->FindSymbol("song", &pszSong, true);
    pSong->FindSymbol("file", &pszFile, true);
    pSong->FindInt("skill_level", &nSkillLevel, true);
    pSong->FindInt("num_players", &nNumPlayers, true);

    if (TheGameDb->GetProfile(0)->mCustom != 0) {
        Campaign profile(*TheGameDb->GetProfile(0));
        TheGameDb->ClearPlayers();
        TheGameDb->AddPlayer(&profile);
    } else {
        TheGameDb->ClearPlayers();
        Campaign profile;
        profile.mName = TheLocale.Localize("default_name_1", true);
        TheGameDb->AddPlayer(&profile);
    }
    for (int i = 1; i < nNumPlayers; ++i) {
        Campaign profile;
        profile.mName = TheLocale.Localize(FormatString("default_name_%d", i + 1), true);
        TheGameDb->AddPlayer(&profile);
    }

    TheGameDb->SetDemo(pszFile);
    TheGameDb->SetSong(pszSong);
    TheGameDb->SetSkillLevel(nSkillLevel);
    TheGameDb->SetLoadRemix(false);
    TheGameDb->SetPracticeMode(false);
    TheGameDb->SetTutorial(0);
    TheGameDb->SetPowerupLevel(kAttractPowerupLevel);
    TheGameDb->SetRuleSet(GameDb::kRuleSetGame);
    TheGameDb->SetCommunity(nNumPlayers == 1 ? GameDb::kCommunitySolo : GameDb::kCommunityLocal);
    TheUI.GotoScreen("pre_launchpad2launchseq");
}

bool MetaStartScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    float fDelayMs = 0.0f;
    DataArray *pMetagame = SystemConfig()->FindArray("metagame", false);
    if (TheGameDb->GetDemo() != 0) {
        pMetagame->FindFloat("attract_short_delay_ms", &fDelayMs, true);
    } else {
        pMetagame->FindFloat("attract_delay_ms", &fDelayMs, true);
    }
    TheGameDb->SetDemo(nullptr);
    mAttractTime = SystemMs() + fDelayMs;
    return FreqScreen::HandleTransitionComplete(pMsg);
}

bool MetaStartScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->mPad == kFirstPad && (pMsg->mButton == kPadCross || pMsg->mButton == kPadStart)) {
            FxMidi::PlayMenuSelect();
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool MetaStartScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}
