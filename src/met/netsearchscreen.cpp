#include "met/netsearchscreen.h"

#include <cstring>
#include <vector>

#include "game/gamedb.h"
#include "game/songentry.h"
#include "met/netsortedscreen.h"
#include "met/songsellist.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kListComponent[] = "list";
constexpr char kSearchComponent[] = "search";
constexpr char kAnyChoice[] = "param_any";

constexpr const char *kDuelSkills[] = {"duel_easy", "duel_medium", "duel_hard", kAnyChoice};
constexpr const char *kSkills[] = {
    "skill_easy", "skill_norm", "skill_exp", "skill_insane", kAnyChoice};

// The entries of mModes, in their order. The last entry searches every mode.
enum SearchMode {
    kSearchModeGame = 0,
    kSearchModeDuel = 1,
    kSearchModeRemix = 2,
};

// The rule set a search for any mode passes, and the skill a search for any skill passes.
constexpr int kAnyRuleSet = 0;
constexpr int kAnySkill = -1;

// The duel skill entries start one level higher.
constexpr int kDuelSkillOffset = 1;

// A skill entry from this one on moves with the list when the list grows by the extra level.
constexpr int kLastSharedSkill = 2;

// The song list starts with the choice of every song.
constexpr int kAllSongsChoiceCount = 1;

template <size_t N>
void AddChoices(std::vector<String> *pChoices,
                const char *pszFormat,
                const char *const (&tokens)[N]) {
    for (const char *pszToken : tokens) {
        String choice(FormatString(pszFormat, TheLocale.Localize(pszToken, true)));
        pChoices->push_back(choice);
    }
}

} // namespace

NetSearchScreen::NetSearchScreen(DataArray *pData) : NetParamsScreen(pData) {
}

void NetSearchScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    mModes.clear();
    const char *pszFormat = TheLocale.Localize("NET_HOST_MODE", true);
    {
        String mode(FormatString(pszFormat, TheLocale.Localize("mode_game", true)));
        mModes.push_back(mode);
    }
    {
        String mode(FormatString(pszFormat, TheLocale.Localize("mode_duel", true)));
        mModes.push_back(mode);
    }
    mDuelMode = kSearchModeDuel;
    {
        String mode(FormatString(pszFormat, TheLocale.Localize("mode_remix", true)));
        mModes.push_back(mode);
    }
    mRemixMode = kSearchModeRemix;
    {
        String mode(FormatString(pszFormat, TheLocale.Localize(kAnyChoice, true)));
        mModes.push_back(mode);
    }
    mLastSkillMode = kSearchModeDuel;
    mMode = static_cast<int>(mModes.size()) - 1;
    OnModeChanged();
    mSkill = static_cast<int>(mSkills.size()) - 1;
    OnChoiceChanged(1);
    NetParamsScreen::Enter(pPrevScreen, fTime);
}

void NetSearchScreen::OnModeChanged() {
    const int nOldCount = static_cast<int>(mSkills.size());
    mSkills.clear();
    const char *pszFormat = TheLocale.Localize("NET_HOST_SKILL", true);
    if (mMode == mDuelMode) {
        AddChoices(&mSkills, pszFormat, kDuelSkills);
    } else {
        AddChoices(&mSkills, pszFormat, kSkills);
    }
    const int nNewCount = static_cast<int>(mSkills.size());
    if (nOldCount == nNewCount) {
        return;
    }
    if (nNewCount < nOldCount) {
        if (mSkill == nNewCount) {
            mSkill = nNewCount - 1;
        }
    } else if (mSkill >= kLastSharedSkill) {
        ++mSkill;
    }
}

void NetSearchScreen::OnChoiceChanged([[maybe_unused]] int nReset) {
    auto *pList =
        static_cast<SongSelList *>(TheUI.FindComponent(mSongPanelName, kListComponent, false));
    String selected;
    if (!pList->mSongs.empty()) {
        selected = pList->mSongs[pList->mSelected].c_str();
    }
    std::vector<String> choices;
    {
        String choice(TheLocale.Localize("search_all", true));
        choices.push_back(choice);
    }
    std::vector<SongEntry> songs;
    if (mMode == mDuelMode) {
        TheGameDb->GetHostSongs(&songs, mSkill, true);
    } else if (mMode == mRemixMode) {
        TheGameDb->GetHostSongs(&songs, GameDb::kSkillAny, false);
    } else {
        TheGameDb->GetHostSongs(&songs, mSkill, false);
    }
    int nSelected = 0;
    for (unsigned int i = 0; i < songs.size(); ++i) {
        SongEntry song = songs[i];
        if (strcmp(selected.c_str(), song.GetName()) == 0) {
            // Yes, the binary adds mChoiceCount before it updates the member.
            nSelected = static_cast<int>(i) + mChoiceCount;
        }
        String choice(songs[i].GetName());
        choices.push_back(choice);
    }
    mChoiceCount = kAllSongsChoiceCount;
    pList->SetSongs(choices, kAllSongsChoiceCount);
    pList->SetSelected(nSelected);
}

bool NetSearchScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return NetParamsScreen::DispatchPriv(pMsg);
}

bool NetSearchScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return NetParamsScreen::HandleSelect(pMsg);
    }
    if (strcmp(pMsg->mComponent->mName, kSearchComponent) != 0) {
        // The binary also tests for `cursor`, with the same result on both paths.
        UIPanel *pPanel = TheUI.FindPanel(mButtonPanelName, false);
        pPanel->SetFocus(pPanel->FindComponent(kSearchComponent, false), kPadNone);
        return NetParamsScreen::HandleSelect(pMsg);
    }

    auto *pList =
        static_cast<SongSelList *>(TheUI.FindComponent("fn_search_song", kListComponent, false));
    auto *pSorted = dynamic_cast<NetSortedScreen *>(TheUI.FindScreen("fn_sorted", false));
    int nRuleSet;
    if (mMode == kSearchModeGame) {
        nRuleSet = GameDb::kRuleSetGame;
    } else if (mMode == kSearchModeDuel) {
        nRuleSet = GameDb::kRuleSetDuel;
    } else if (mMode == kSearchModeRemix) {
        nRuleSet = GameDb::kRuleSetRemix;
    } else {
        nRuleSet = kAnyRuleSet;
    }
    int nSkillLevel = nRuleSet == GameDb::kRuleSetDuel ? mSkill + kDuelSkillOffset : mSkill;
    if (mSkill == GameDb::kSkillAny) {
        nSkillLevel = kAnySkill;
    }
    const char *pszArena = pList->mSelected != 0 ? pList->mSongs[pList->mSelected].c_str() : "";
    pSorted->SetSearch(pszArena, nRuleSet, nSkillLevel);
    TheUI.GotoScreen(pSorted);
    return NetParamsScreen::HandleSelect(pMsg);
}
