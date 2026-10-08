#include "met/jukeboxscreen.h"

#include <cstring>

#include "game/campaign.h"
#include "game/gamedb.h"
#include "game/songentry.h"
#include "met/metagame.h"
#include "met/playlist.h"
#include "met/redbookplaybackscreen.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "synth/fxmidi.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kPlaybackScreen[] = "redbook_play";
constexpr char kModeScreen[] = "s_mode";
constexpr char kButtonPanel[] = "jbox";
constexpr char kListPanel[] = "jbox_list";
constexpr char kListComponent[] = "list";
constexpr char kCreateComponent[] = "create";
constexpr char kCursorComponent[] = "cursor";
constexpr char kInOrderComponent[] = "play_in_order";
constexpr char kAllSongsToken[] = "jbox_all_redbook";
constexpr char kListHelpToken[] = "jbox_list_focus_HELP";
constexpr char kListActionToken[] = "jbox_list_focus_ACTION";
constexpr char kButtonsHelpToken[] = "jbox_create_HELP";
constexpr char kButtonsActionToken[] = "default_ACTION";
constexpr char kAllArenas[] = "";

// The skill level that GetUnlockedSongs() and IsSongFinished() take for any skill level.
constexpr int kAnySkillLevel = 4;

// The entries at the start of the playlist that stand for groups of songs.
constexpr int kGroupEntries = 1;

// The first player, whose finished songs the playlist lists.
constexpr int kFirstPlayer = 0;

PlayList *FindPlayList() {
    return dynamic_cast<PlayList *>(TheUI.FindComponent(kListPanel, kListComponent, false));
}

} // namespace

void JukeboxScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    if (pPrevScreen == nullptr || std::strcmp(pPrevScreen->mName, kPlaybackScreen) != 0) {
        mPlaylist.clear();
        mPlaylist.push_back({1, TheLocale.Localize(kAllSongsToken, true)});
        std::vector<SongEntry> songs;
        TheGameDb->GetUnlockedSongs(&songs, kAllArenas, kAnySkillLevel, true);
        for (unsigned int i = 0; i < songs.size(); ++i) {
            const SongEntry song = songs[i];
            if (mShowAllSongs != 0 || TheGameDb->GetProfile(kFirstPlayer)
                                          ->IsSongFinished(song.GetName(), kAnySkillLevel)) {
                mPlaylist.push_back({1, song.GetName()});
            }
        }
    }
    PlayList *pList = FindPlayList();
    pList->SetSongs(mPlaylist, kGroupEntries);
    pList->SetCursorSelected(false);
}

void JukeboxScreen::Exit(UIScreen *pNextScreen, float fTime) {
    FreqScreen::Exit(pNextScreen, fTime);
}

bool JukeboxScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool JukeboxScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    const char *pszButton = pMsg->mComponent->mName;
    if (pMsg->mButton != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }
    if (std::strcmp(pszButton, kCreateComponent) == 0) {
        TheMetagame.SetHelpText(TheLocale.Localize(kListHelpToken, true));
        TheMetagame.SetActionText(TheLocale.Localize(kListActionToken, true));
        SetFocus(TheUI.FindPanel(kListPanel, false));
    } else if (std::strcmp(pszButton, kCursorComponent) != 0) {
        auto *pPlayback =
            dynamic_cast<RedbookPlaybackScreen *>(TheUI.FindScreen(kPlaybackScreen, false));
        pPlayback->ClearSongs();
        pPlayback->mShuffle = std::strcmp(pszButton, kInOrderComponent) != 0;
        mPlaylist = FindPlayList()->mSongs;
        int nSongs = 0;
        for (unsigned int i = kGroupEntries; i < mPlaylist.size(); ++i) {
            if (mPlaylist[i].mSelected != 0) {
                pPlayback->AddSong(mPlaylist[i].mSong);
                ++nSongs;
            }
        }
        if (nSongs != 0) {
            TheUI.GotoScreen(pPlayback);
        }
    }
    return UIScreen::HandleSelect(pMsg);
}

bool JukeboxScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    if (pMsg->mButton == kPadDRight && std::strcmp(pMsg->mPanel->mName, kButtonPanel) == 0 &&
        std::strcmp(pMsg->mComponent->mName, kCreateComponent) == 0) {
        TheMetagame.SetHelpText(TheLocale.Localize(kListHelpToken, true));
        TheMetagame.SetActionText(TheLocale.Localize(kListActionToken, true));
        SetFocus(TheUI.FindPanel(kListPanel, false));
    }
    return FreqScreen::HandleSelectStart(pMsg);
}

bool JukeboxScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (pMsg->mButton == kPadTriangle && pMsg->mPressed != 0) {
        if (std::strcmp(TheUI.FocusPanel()->mName, kButtonPanel) == 0) {
            mPlaylist.clear();
            TheUI.GotoScreen(kModeScreen);
        } else {
            FxMidi::PlayBack();
            TheMetagame.SetHelpText(TheLocale.Localize(kButtonsHelpToken, true));
            TheMetagame.SetActionText(TheLocale.Localize(kButtonsActionToken, true));
            SetFocus(TheUI.FindPanel(kButtonPanel, false));
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}
