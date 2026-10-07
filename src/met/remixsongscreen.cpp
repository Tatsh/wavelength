#include "met/remixsongscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "game/songentry.h"
#include "met/songsellist.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "ui/uilist.h"
#include "ui/uimanager.h"

namespace {

constexpr char kPanel[] = "s_r_sel_song";
constexpr char kList[] = "list";
constexpr char kTutorialSequence[] = "pre_multitut2launchseq";

// Without unlocked songs, the screen lists the songs of type 0 of the first arena.
constexpr int kFallbackArena = 0;
constexpr int kFallbackFilter = 2;
constexpr int kFallbackType = 0;

} // namespace

RemixSongScreen::RemixSongScreen(DataArray *pData) : FreqScreen(pData), mSong(nullptr) {
}

void RemixSongScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    auto *pList = static_cast<SongSelList *>(TheUI.FindComponent(kPanel, kList, false));
    std::vector<SongEntry> songs;
    TheGameDb->GetUnlockedSongs(
        &songs, "", GameDb::kSkillAny, TheGameDb->mCommunity == GameDb::kCommunitySolo);
    if (songs.empty()) {
        std::vector<const char *> arenas;
        TheGameDb->GetArenaNames(&arenas, false);
        std::vector<SongEntry> arenaSongs;
        TheGameDb->GetArenaSongs(
            &arenaSongs, arenas[kFallbackArena], GameDb::kSkillAny, kFallbackFilter);
        for (unsigned int i = 0; i < arenaSongs.size(); ++i) {
            if (arenaSongs[i].GetType() == kFallbackType) {
                songs.push_back(arenaSongs[i]);
            }
        }
    }

    mSongs.clear();
    for (unsigned int i = 0; i < songs.size(); ++i) {
        if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
            String song(songs[i].GetName());
            mSongs.push_back(song);
        } else {
            const char *pszSong = songs[i].GetName();
            const char *pszMidiFile;
            if (TheGameDb->FindSong(pszSong)->FindSymbol("remix_midi_file", &pszMidiFile, false)) {
                String song(pszSong);
                mSongs.push_back(song);
            }
        }
    }
    pList->SetSongs(mSongs, 0);
    FreqScreen::Enter(pPrevScreen, fTime);
}

bool RemixSongScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool RemixSongScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        auto *pList = static_cast<UIList *>(TheUI.FindComponent(kPanel, kList, false));
        mSong = mSongs[pList->mSelected];
        TheGameDb->SetTutorial(0);
        TheGameDb->SetPracticeMode(false);
        DebugPrint("going to launch remix or duel\n");
        TheGameDb->SetSong(mSong.c_str());
        if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
            TheGameDb->SetRemixActive(1);
            RemixInfo record = *TheGameDb->GetRemixInfo();
            strcpy(record.mSong, mSong.c_str());
            TheGameDb->SetRemix(&record);
            if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
                TheUI.GotoScreen("remix_create_mode");
            } else {
                TheGameDb->SetRemixActive(1);
                TheUI.GotoScreen(kTutorialSequence);
            }
        } else {
            TheUI.GotoScreen(kTutorialSequence);
        }
    }
    return UIScreen::HandleSelect(pMsg);
}
