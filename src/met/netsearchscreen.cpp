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
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kModeFormatToken[] = "NET_HOST_MODE";
constexpr char kGameToken[] = "mode_game";
constexpr char kDuelToken[] = "mode_duel";
constexpr char kRemixToken[] = "mode_remix";
constexpr char kAnyToken[] = "param_any";
constexpr char kSkillFormatToken[] = "NET_HOST_SKILL";
constexpr char kDuelEasyToken[] = "duel_easy";
constexpr char kDuelMediumToken[] = "duel_medium";
constexpr char kDuelHardToken[] = "duel_hard";
constexpr char kSkillEasyToken[] = "skill_easy";
constexpr char kSkillNormalToken[] = "skill_norm";
constexpr char kSkillExpertToken[] = "skill_exp";
constexpr char kSkillInsaneToken[] = "skill_insane";
constexpr char kAllToken[] = "search_all";
constexpr char kSearchComponent[] = "search";
constexpr char kListComponent[] = "list";
constexpr char kSongPanel[] = "fn_search_song";
constexpr char kSortedScreen[] = "fn_sorted";
constexpr char kNoSong[] = "";

// The entries of mModes, which HandleSelect() reads without mDuelMode and mRemixMode.
enum Mode {
    kModeGame = 0,
    kModeDuel = 1,
    kModeRemix = 2,
};

// The rule set NetSortedScreen::SetSearch() receives for any mode.
constexpr int kAnyRuleSet = 0;

// The skill entry for any skill of a game, and the skill NetSortedScreen::SetSearch() receives
// for it.
constexpr int kAnySkillEntry = 4;
constexpr int kAnySkill = -1;

// The skill levels of a duel start one above those of a game.
constexpr int kDuelSkillOffset = 1;

// The skill entry from which the longer list has one more entry below it.
constexpr int kSkillShift = 2;

// The entries of the song list before the songs, and the entry of `search_all`.
constexpr int kChoiceCount = 1;
constexpr int kAllChoice = 0;

void AddLabel(std::vector<String> &labels, const char *pszFormat, const char *pszToken) {
    labels.push_back(String(FormatString(pszFormat, TheLocale.Localize(pszToken, true))));
}

} // namespace

void NetSearchScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    mModes.clear();
    const char *pszFormat = TheLocale.Localize(kModeFormatToken, true);
    AddLabel(mModes, pszFormat, kGameToken);
    AddLabel(mModes, pszFormat, kDuelToken);
    mDuelMode = kModeDuel;
    AddLabel(mModes, pszFormat, kRemixToken);
    mRemixMode = kModeRemix;
    AddLabel(mModes, pszFormat, kAnyToken);
    mLastSkillMode = kModeDuel;
    mMode = static_cast<int>(mModes.size()) - 1;
    OnModeChanged();
    mSkill = static_cast<int>(mSkills.size()) - 1;
    OnChoiceChanged(1);
    NetParamsScreen::Enter(pPrevScreen, fTime);
}

void NetSearchScreen::OnModeChanged() {
    const int nOldCount = static_cast<int>(mSkills.size());
    mSkills.clear();
    const char *pszFormat = TheLocale.Localize(kSkillFormatToken, true);
    if (mMode == mDuelMode) {
        AddLabel(mSkills, pszFormat, kDuelEasyToken);
        AddLabel(mSkills, pszFormat, kDuelMediumToken);
        AddLabel(mSkills, pszFormat, kDuelHardToken);
    } else {
        AddLabel(mSkills, pszFormat, kSkillEasyToken);
        AddLabel(mSkills, pszFormat, kSkillNormalToken);
        AddLabel(mSkills, pszFormat, kSkillExpertToken);
        AddLabel(mSkills, pszFormat, kSkillInsaneToken);
    }
    AddLabel(mSkills, pszFormat, kAnyToken);

    const int nCount = static_cast<int>(mSkills.size());
    if (nCount < nOldCount) {
        if (mSkill == nCount) {
            mSkill = nCount - 1;
        }
    } else if (nOldCount < nCount) {
        if (mSkill >= kSkillShift) {
            ++mSkill;
        }
    }
}

void NetSearchScreen::OnChoiceChanged([[maybe_unused]] int bReset) {
    auto *pList =
        static_cast<SongSelList *>(TheUI.FindComponent(mSongPanelName, kListComponent, false));
    String selected;
    if (!pList->mSongs.empty()) {
        selected = pList->mSongs[pList->mSelected].c_str();
    }

    std::vector<String> choices;
    choices.push_back(String(TheLocale.Localize(kAllToken, true)));
    std::vector<SongEntry> songs;
    if (mMode == mDuelMode) {
        TheGameDb->GetHostSongs(&songs, mSkill, true);
    } else if (mMode == mRemixMode) {
        TheGameDb->GetHostSongs(&songs, GameDb::kSkillAny, false);
    } else {
        TheGameDb->GetHostSongs(&songs, mSkill, false);
    }

    int nSelected = kAllChoice;
    for (unsigned int i = 0; i < songs.size(); ++i) {
        if (std::strcmp(selected.c_str(), songs[i].GetName()) == 0) {
            // The binary offsets by mChoiceCount before it sets it below.
            nSelected = static_cast<int>(i) + mChoiceCount;
        }
        choices.push_back(String(songs[i].GetName()));
    }
    mChoiceCount = kChoiceCount;
    pList->SetSongs(choices, kChoiceCount);
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
    if (std::strcmp(pMsg->mComponent->mName, kSearchComponent) != 0) {
        UIPanel *pPanel = TheUI.FindPanel(mButtonPanelName, false);
        pPanel->SetFocus(pPanel->FindComponent(kSearchComponent, false), kPadNone);
        return NetParamsScreen::HandleSelect(pMsg);
    }

    auto *pList =
        static_cast<SongSelList *>(TheUI.FindComponent(kSongPanel, kListComponent, false));
    UIScreen *pScreen = TheUI.FindScreen(kSortedScreen, false);
    auto *pSorted = pScreen != nullptr ? dynamic_cast<NetSortedScreen *>(pScreen) : nullptr;

    int nRuleSet;
    if (mMode == kModeGame) {
        nRuleSet = GameDb::kRuleSetGame;
    } else if (mMode == kModeDuel) {
        nRuleSet = GameDb::kRuleSetDuel;
    } else if (mMode == kModeRemix) {
        nRuleSet = GameDb::kRuleSetRemix;
    } else {
        nRuleSet = kAnyRuleSet;
    }
    int nSkill = nRuleSet == GameDb::kRuleSetDuel ? mSkill + kDuelSkillOffset : mSkill;
    if (mSkill == kAnySkillEntry) {
        nSkill = kAnySkill;
    }
    const char *pszSong =
        pList->mSelected != kAllChoice ? pList->mSongs[pList->mSelected].c_str() : kNoSong;
    pSorted->SetSearch(pszSong, nRuleSet, nSkill);
    TheUI.GotoScreen(pSorted);
    return NetParamsScreen::HandleSelect(pMsg);
}
