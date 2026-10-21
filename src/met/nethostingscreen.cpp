#include "met/nethostingscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "game/songentry.h"
#include "math/rand.h"
#include "met/songsellist.h"
#include "msg/chatmsg.h"
#include "netflow/netlaunchpad.h"
#include "os/locale.h"
#include "ui/uibutton.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kCursorComponent[] = "cursor";
constexpr char kIsEditTag[] = "is_edit";
constexpr char kModeFormatToken[] = "NET_HOST_MODE";
constexpr char kGameToken[] = "mode_game";
constexpr char kDuelToken[] = "mode_duel";
constexpr char kRemixToken[] = "mode_remix";
constexpr char kSkillFormatToken[] = "NET_HOST_SKILL";
#ifdef VIDEO_STANDARD_PAL
constexpr char kDuelEasyToken[] = "duel_easy_abbrev";
constexpr char kDuelMediumToken[] = "duel_medium_abbrev";
constexpr char kDuelHardToken[] = "duel_hard_abbrev";
#else
constexpr char kDuelEasyToken[] = "duel_easy";
constexpr char kDuelMediumToken[] = "duel_medium";
constexpr char kDuelHardToken[] = "duel_hard";
#endif
constexpr char kSkillEasyToken[] = "skill_easy";
constexpr char kSkillNormalToken[] = "skill_norm";
constexpr char kSkillExpertToken[] = "skill_exp";
constexpr char kSkillInsaneToken[] = "skill_insane";
constexpr char kPlayersFormatToken[] = "NET_HOST_PLAYERS";
constexpr char kPowerupFormatToken[] = "NET_HOST_PUP";
constexpr char kPowerupLowToken[] = "pup_low";
constexpr char kPowerupHighToken[] = "pup_hi";
constexpr char kPowerupOffToken[] = "pup_off";
constexpr char kRandomToken[] = "host_random";
constexpr char kCustomToken[] = "host_custom";
constexpr char kDoneEditToken[] = "host_done_edit_msg";
constexpr char kLaunchpadScreen[] = "fn_h_lpad";
constexpr char kHostingPanel[] = "fn_hosting";
constexpr char kNumPlayersComponent[] = "num_players";
constexpr char kPowerupComponent[] = "powerup";
constexpr char kHostComponent[] = "host";
constexpr char kListComponent[] = "list";
constexpr char kEditRemixListScreen[] = "load_remix_list_net_edit";
constexpr char kRemixListScreen[] = "load_remix_list_net";
constexpr char kHostAttemptScreen[] = "net_host_attempt";
constexpr char kUntitled[] = "Untitled";

// The fewest and the most players of a session.
constexpr int kMinPlayers = 2;
constexpr int kMaxPlayers = 4;

// The first entry of the song list, which picks a song at random.
constexpr int kRandomChoice = 0;

// The number of entries before the songs, without and with `host_custom`.
constexpr int kDuelChoiceCount = 1;
constexpr int kChoiceCount = 2;

// The entry of `host_custom` after `host_random`.
constexpr int kCustomAfterRandom = 1;
constexpr int kNoCustomChoice = -1;

// The skill levels of a duel start one above those of a game.
constexpr int kDuelSkillOffset = 1;

// The skill entry from which the longer list has one more entry below it.
constexpr int kSkillShift = 2;

constexpr int kNoSkill = -1;
constexpr int kNoticeChannel = 2;
constexpr int kEveryConsole = 0;
constexpr int kEcho = 1;

void AddLabel(std::vector<String> &labels, const char *pszFormat, const char *pszToken) {
    labels.push_back(String(FormatString(pszFormat, TheLocale.Localize(pszToken, true))));
}

} // namespace

NetHostingScreen::NetHostingScreen(DataArray *pData) : NetParamsScreen(pData) {
    mNumPlayers = 0;
    mSavedParams.SetPracticeMode(0);
    mSavedParams.SetTutorial(0);
    mSavedParams.SetSkillLevel(0);
    mSavedParams.SetPowerupLevel(1);
    mSavedParams.SetRuleSet(GameDb::kRuleSetGame);
    mSavedParams.SetCommunity(GameDb::kCommunitySolo);
    mSavedParams.SetMaxPlayers(kMaxPlayers);
    pData->FindBool(kIsEditTag, &mIsEdit, false);
}

void NetHostingScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    mModes.clear();
    const char *pszModeFormat = TheLocale.Localize(kModeFormatToken, true);
    AddLabel(mModes, pszModeFormat, kGameToken);
    if (mIsEdit != 0 && std::strcmp(pPrevScreen->GetName(), kLaunchpadScreen) == 0) {
        mSavedParams.SetSong(TheGameDb->GetSong().c_str());
        mSavedParams.SetLoadRemix(TheGameDb->GetLoadRemix());
        mSavedParams.SetRemixReadOnly(TheGameDb->GetRemixReadOnly());
        mSavedParams.SetRemixName(TheGameDb->GetRemixName().c_str());
        mSavedParams.SetPracticeMode(TheGameDb->GetPracticeMode());
        mSavedParams.SetTutorial(TheGameDb->GetTutorial());
        mSavedParams.SetSkillLevel(TheGameDb->GetSkillLevel());
        mSavedParams.SetPowerupLevel(TheGameDb->GetPowerupLevel());
        mSavedParams.SetRuleSet(TheGameDb->GetRuleSet());
        mSavedParams.SetCommunity(TheGameDb->GetCommunity());
        mSavedParams.SetMaxPlayers(TheGameDb->GetMaxPlayers());
    }
    if (mIsEdit != 0 && TheGameDb->GetRuleSet() != GameDb::kRuleSetDuel &&
        TheGameDb->GetMaxPlayers() > kMinPlayers) {
        mDuelMode = -1;
        mRemixMode = 1;
        mLastSkillMode = 0;
    } else {
        AddLabel(mModes, pszModeFormat, kDuelToken);
        mLastSkillMode = 1;
        mRemixMode = 2;
        mDuelMode = 1;
    }
    AddLabel(mModes, pszModeFormat, kRemixToken);

    if (TheGameDb->GetRuleSet() == GameDb::kRuleSetDuel) {
        mMode = mDuelMode;
    } else if (TheGameDb->GetRuleSet() == GameDb::kRuleSetRemix) {
        mMode = mRemixMode;
    } else {
        mMode = 0;
    }
    OnModeChanged();
    if (TheGameDb->GetSkillLevel() != kNoSkill) {
        mSkill = TheGameDb->GetSkillLevel();
    }
    if (TheGameDb->GetRuleSet() == GameDb::kRuleSetDuel) {
        mSkill -= kDuelSkillOffset;
    }

    mPlayerCounts.clear();
    const char *pszPlayersFormat = TheLocale.Localize(kPlayersFormatToken, true);
    for (int nPlayers = kMinPlayers; nPlayers <= kMaxPlayers; ++nPlayers) {
        mPlayerCounts.push_back(String(FormatString(pszPlayersFormat, nPlayers)));
    }
    if (mIsEdit != 0) {
        UIComponent *pComponent = TheUI.FindComponent(kHostingPanel, kNumPlayersComponent, false);
        UIButton *pButton = pComponent != nullptr ? dynamic_cast<UIButton *>(pComponent) : nullptr;
        pButton->SetState(UIComponent::kStateDisabled, false);
    }

    mPowerups.clear();
    const char *pszPowerupFormat = TheLocale.Localize(kPowerupFormatToken, true);
    AddLabel(mPowerups, pszPowerupFormat, kPowerupLowToken);
    AddLabel(mPowerups, pszPowerupFormat, kPowerupHighToken);
    AddLabel(mPowerups, pszPowerupFormat, kPowerupOffToken);
    mPowerup = TheGameDb->GetPowerupLevel();

    OnChoiceChanged(1);
    mDuelNumPlayers = 0;
    mLastNumPlayers = static_cast<int>(mPlayerCounts.size()) - 1;
    if (mIsEdit != 0) {
        mNumPlayers = TheGameDb->GetMaxPlayers() - kMinPlayers;
    } else {
        mNumPlayers = static_cast<int>(mPlayerCounts.size()) - 1;
    }
    NetParamsScreen::Enter(pPrevScreen, fTime);
}

void NetHostingScreen::OnModeChanged() {
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
    if (nOldCount == 0) {
        return;
    }
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

void NetHostingScreen::OnChoiceChanged(int bReset) {
    auto *pList =
        static_cast<SongSelList *>(TheUI.FindComponent(mSongPanelName, kListComponent, false));
    String selected;
    if (!pList->GetSongs().empty()) {
        selected = pList->GetSongs()[pList->GetSelected()].c_str();
    }

    std::vector<String> choices;
    choices.push_back(String(TheLocale.Localize(kRandomToken, true)));
    std::vector<SongEntry> songs;
    mChoiceCount = 0;
    if (mMode == mDuelMode) {
        mChoiceCount = kDuelChoiceCount;
        TheGameDb->GetHostSongs(&songs, GameDb::kSkillAny, true);
        mCustomChoice = kNoCustomChoice;
    } else {
        mChoiceCount = kChoiceCount;
        TheGameDb->GetHostSongs(
            &songs, mMode == mRemixMode ? static_cast<int>(GameDb::kSkillAny) : mSkill, false);
        if (songs.empty()) {
            choices.clear();
            mCustomChoice = 0;
        } else {
            mCustomChoice = kCustomAfterRandom;
        }
        choices.push_back(String(TheLocale.Localize(kCustomToken, true)));
    }

    int nSelected = kRandomChoice;
    for (unsigned int i = 0; i < songs.size(); ++i) {
        if (std::strcmp(selected.c_str(), songs[i].GetName()) == 0 ||
            std::strcmp(TheGameDb->GetSong().c_str(), songs[i].GetName()) == 0) {
            nSelected = static_cast<int>(i) + mChoiceCount;
        }
        choices.push_back(String(songs[i].GetName()));
    }
    if (bReset != 0) {
        nSelected = kRandomChoice;
    } else if (std::strcmp(selected.c_str(), TheLocale.Localize(kRandomToken, true)) == 0) {
        nSelected = kRandomChoice;
    } else if (std::strcmp(selected.c_str(), TheLocale.Localize(kCustomToken, true)) == 0) {
        nSelected = mCustomChoice;
    }

    pList->SetSongs(choices, mChoiceCount);
    pList->SetSelected(nSelected);
    mDuelNumPlayers = 0;
    mLastNumPlayers = static_cast<int>(mPlayerCounts.size()) - 1;
}

void NetHostingScreen::UpdateLabels() {
    UIPanel *pPanel = mFocusPanel;
    NetParamsScreen::UpdateLabels();
    UIComponent *pPowerup = pPanel->FindComponent(kPowerupComponent, false);
    if (mMode != 0) {
        pPowerup->SetState(UIComponent::kStateDisabled, false);
    } else if (pPowerup->GetState() != UIComponent::kStateSelected) {
        pPowerup->SetState(UIComponent::kStateNormal, false);
    }

    UIComponent *pPlayers = pPanel->FindComponent(kNumPlayersComponent, false);
    if (mMode == mDuelMode) {
        mNumPlayers = mDuelNumPlayers;
        pPlayers->SetState(UIComponent::kStateDisabled, false);
    } else if (mIsEdit == 0 && pPlayers->GetState() != UIComponent::kStateSelected) {
        pPlayers->SetState(UIComponent::kStateNormal, false);
    }
    pPlayers->SetText(mPlayerCounts[mNumPlayers].c_str());
    pPowerup->SetText(mPowerups[mPowerup].c_str());
}

bool NetHostingScreen::DispatchPriv(Message *pMsg) {
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
    return NetParamsScreen::DispatchPriv(pMsg);
}

bool NetHostingScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->GetButton() != kPadCross) {
        return NetParamsScreen::HandleSelect(pMsg);
    }
    if (std::strcmp(pMsg->GetComponent()->GetName(), kHostComponent) != 0) {
        // Yes, the binary focuses the host button on both paths.
        if (std::strcmp(pMsg->GetComponent()->GetName(), kCursorComponent) == 0) {
            UIPanel *pPanel = TheUI.FindPanel(mButtonPanelName, false);
            pPanel->SetFocus(pPanel->FindComponent(kHostComponent, false), kPadNone);
        } else {
            UIPanel *pPanel = TheUI.FindPanel(mButtonPanelName, false);
            pPanel->SetFocus(pPanel->FindComponent(kHostComponent, false), kPadNone);
        }
        return NetParamsScreen::HandleSelect(pMsg);
    }

    TheGameDb->SetTutorial(0);
    if (mMode == mDuelMode) {
        TheGameDb->SetRuleSet(GameDb::kRuleSetDuel);
    } else if (mMode == mRemixMode) {
        TheGameDb->SetRuleSet(GameDb::kRuleSetRemix);
    } else {
        TheGameDb->SetRuleSet(GameDb::kRuleSetGame);
    }
    if (TheGameDb->GetRuleSet() == GameDb::kRuleSetDuel) {
        TheGameDb->SetSkillLevel(mSkill + kDuelSkillOffset);
    } else {
        TheGameDb->SetSkillLevel(mSkill);
    }
    TheGameDb->SetPowerupLevel(mPowerup);
    TheGameDb->SetMaxPlayers(mNumPlayers + kMinPlayers);

    auto *pList =
        static_cast<SongSelList *>(TheUI.FindComponent(mSongPanelName, kListComponent, false));
    int nSong = pList->GetSelected();
    if (nSong == mCustomChoice && nSong < mChoiceCount) {
        TheUI.GotoScreen(mIsEdit != 0 ? kEditRemixListScreen : kRemixListScreen);
        return NetParamsScreen::HandleSelect(pMsg);
    }
    TheGameDb->SetLoadRemix(false);
    if (nSong == kRandomChoice) {
        nSong = RandomInt(mChoiceCount, static_cast<int>(pList->GetSongs().size()));
    }
    TheGameDb->SetSong(pList->GetSongs()[nSong].c_str());

    if (mMode == mRemixMode) {
        RemixInfo info;
        std::strncpy(info.mName, kUntitled, kRemixInfoNameSize - 1);
        info.mSource = RemixInfo::kSourceNone;
        info.mDataSize = 0;
        for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
            std::strncpy(info.mCreators[i], TheGameDb->GetPlayerName(i), kRemixInfoCreatorSize - 1);
        }
        info.mDate.ReadClock();
        info.mReadOnly = 0;
        std::strcpy(info.mSong, pList->GetSongs()[nSong].c_str());
        TheGameDb->SetRemix(&info);
        TheGameDb->SetRemixReadOnly(0);
    }

    if (mIsEdit != 0) {
        ChatMsg::Send(
            kNoticeChannel, kEveryConsole, TheLocale.Localize(kDoneEditToken, true), kEcho);
        if (TheNetLaunchpad != nullptr) {
            TheNetLaunchpad->EndEdit();
        }
        TheUI.GotoScreen(kLaunchpadScreen);
    } else {
        TheUI.GotoScreen(kHostAttemptScreen);
    }
    return NetParamsScreen::HandleSelect(pMsg);
}

bool NetHostingScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    const char *pszComponent = pMsg->GetComponent()->GetName();
    const int nButton = pMsg->GetButton();
    if (std::strcmp(pszComponent, kNumPlayersComponent) == 0) {
        if (nButton == kPadDLeft || nButton == kPadDRight) {
            mNumPlayers = StepChoice(mNumPlayers, static_cast<int>(mPlayerCounts.size()), nButton);
        }
        UpdateLabels();
    } else if (std::strcmp(pszComponent, kPowerupComponent) == 0) {
        if (nButton == kPadDLeft || nButton == kPadDRight) {
            mPowerup = StepChoice(mPowerup, static_cast<int>(mPowerups.size()), nButton);
        }
        UpdateLabels();
    }
    return NetParamsScreen::HandleSelectStart(pMsg);
}

bool NetHostingScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->GetPressed() != 0) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->GetButton() == kPadTriangle && mIsEdit != 0) {
            TheGameDb->SetGameParams(&mSavedParams);
        }
    }
    return NetParamsScreen::HandleJoypad(pMsg);
}
