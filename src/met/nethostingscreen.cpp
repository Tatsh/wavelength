#include "met/nethostingscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "game/songentry.h"
#include "math/rand.h"
#include "met/songsellist.h"
#include "msg/chatmsg.h"
#include "netflow/netlaunchpad.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uibutton.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kListComponent[] = "list";
constexpr char kHostComponent[] = "host";
constexpr char kPlayersComponent[] = "num_players";
constexpr char kPowerupComponent[] = "powerup";
constexpr char kLaunchpadScreen[] = "fn_h_lpad";
constexpr char kRandomChoice[] = "host_random";
constexpr char kCustomChoice[] = "host_custom";

constexpr const char *kDuelSkills[] = {"duel_easy", "duel_medium", "duel_hard"};
constexpr const char *kSkills[] = {"skill_easy", "skill_norm", "skill_exp", "skill_insane"};
constexpr const char *kPowerups[] = {"pup_low", "pup_hi", "pup_off"};

// The player choices run from two to four players.
constexpr int kMinNetPlayers = 2;
constexpr int kMaxNetPlayers = 4;

// A duel needs exactly two players, and its skill entries start one level higher.
constexpr int kDuelPlayers = 2;
constexpr int kDuelSkillOffset = 1;

// A skill entry from this one on moves with the list when the list grows by the extra level.
constexpr int kLastSharedSkill = 2;

constexpr int kNoDuel = -1;
constexpr int kNoCustomChoice = -1;
constexpr int kNoSkill = -1;
constexpr int kRemixWithoutDuel = 1;
constexpr int kRemixAfterDuel = 2;
constexpr int kDuelMode = 1;

// The random choice comes first in the song list, and the remix choice second.
constexpr int kRandomChoiceCount = 1;
constexpr int kRandomAndCustomCount = 2;

// The longest name and player name strncpy() copies into a RemixInfo.
constexpr size_t kNameCopyLength = 29;
constexpr size_t kCreatorCopyLength = 15;

// The notices of the session go to every console on this channel, and show here too.
constexpr int kNoticeChannel = 2;
constexpr int kEveryConsole = 0;
constexpr int kEcho = 1;

template <size_t N>
void AddChoices(std::vector<String> *pChoices,
                const char *pszFormat,
                const char *const (&tokens)[N]) {
    for (const char *pszToken : tokens) {
        String choice(FormatString(pszFormat, TheLocale.Localize(pszToken, true)));
        pChoices->push_back(choice);
    }
}

// Copies the settings of the game database the way the inline copy of the binary does.
void CopyGameParams(NetGameParams *pParams) {
    pParams->mSong = TheGameDb->mSong;
    pParams->mLoadRemix = TheGameDb->mLoadRemix;
    pParams->mRemixReadOnly = TheGameDb->mRemixReadOnly;
    pParams->mRemixName = TheGameDb->mRemixName;
    pParams->mPracticeMode = TheGameDb->mPracticeMode;
    pParams->mTutorial = TheGameDb->mTutorial;
    pParams->mSkillLevel = TheGameDb->mSkillLevel;
    pParams->mPowerupLevel = TheGameDb->mPowerupLevel;
    pParams->mRuleSet = TheGameDb->mRuleSet;
    pParams->mCommunity = TheGameDb->mCommunity;
    pParams->mNetPlayers = TheGameDb->mNetPlayers;
}

// Enables a choice the mode allows, unless it has the focus.
void EnableChoice(UIComponent *pComponent) {
    if (pComponent->GetState() != UIComponent::kStateSelected) {
        pComponent->SetState(UIComponent::kStateNormal, false);
    }
}

int StepEntry(int nEntry, int nCount, int nButton) {
    if (nButton == kPadDLeft) {
        return nEntry - 1 > -1 ? nEntry - 1 : nCount - 1;
    }
    if (nButton == kPadDRight) {
        return nEntry + 1 < nCount ? nEntry + 1 : 0;
    }
    return nEntry;
}

} // namespace

NetHostingScreen::NetHostingScreen(DataArray *pData)
    : NetParamsScreen(pData), mNetPlayersChoice(0) {
    pData->FindBool("is_edit", &mIsEdit, false);
}

void NetHostingScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    mModes.clear();
    const char *pszModeFormat = TheLocale.Localize("NET_HOST_MODE", true);
    {
        String mode(FormatString(pszModeFormat, TheLocale.Localize("mode_game", true)));
        mModes.push_back(mode);
    }
    if (mIsEdit && strcmp(pPrevScreen->mName, kLaunchpadScreen) != 0) {
        CopyGameParams(&mParams);
    }
    if (mIsEdit && TheGameDb->mRuleSet != GameDb::kRuleSetDuel &&
        TheGameDb->mNetPlayers > kDuelPlayers) {
        mDuelMode = kNoDuel;
        mRemixMode = kRemixWithoutDuel;
        mLastSkillMode = 0;
    } else {
        String mode(FormatString(pszModeFormat, TheLocale.Localize("mode_duel", true)));
        mModes.push_back(mode);
        mLastSkillMode = kDuelMode;
        mRemixMode = kRemixAfterDuel;
        mDuelMode = kDuelMode;
    }
    {
        String mode(FormatString(pszModeFormat, TheLocale.Localize("mode_remix", true)));
        mModes.push_back(mode);
    }
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        mMode = mDuelMode;
    } else if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
        mMode = mRemixMode;
    } else {
        mMode = 0;
    }
    OnModeChanged();
    if (TheGameDb->mSkillLevel != kNoSkill) {
        mSkill = TheGameDb->mSkillLevel;
    }
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        mSkill -= kDuelSkillOffset;
    }

    mPlayerChoices.clear();
    const char *pszPlayersFormat = TheLocale.Localize("NET_HOST_PLAYERS", true);
    for (int nPlayers = kMinNetPlayers; nPlayers <= kMaxNetPlayers; ++nPlayers) {
        String choice(FormatString(pszPlayersFormat, nPlayers));
        mPlayerChoices.push_back(choice);
    }
    if (mIsEdit) {
        dynamic_cast<UIButton *>(TheUI.FindComponent("fn_hosting", kPlayersComponent, false))
            ->SetState(UIComponent::kStateDisabled, false);
    }

    mPowerupChoices.clear();
    AddChoices(&mPowerupChoices, TheLocale.Localize("NET_HOST_PUP", true), kPowerups);
    mPowerupChoice = TheGameDb->mPowerupLevel;
    OnChoiceChanged(1);
    mDuelPlayersChoice = 0;
    const int nLastPlayersChoice = static_cast<int>(mPlayerChoices.size()) - 1;
    mLastPlayersChoice = nLastPlayersChoice;
    mNetPlayersChoice = mIsEdit ? TheGameDb->mNetPlayers - kMinNetPlayers : nLastPlayersChoice;
    NetParamsScreen::Enter(pPrevScreen, fTime);
}

void NetHostingScreen::OnModeChanged() {
    const int nOldCount = static_cast<int>(mSkills.size());
    mSkills.clear();
    const char *pszFormat = TheLocale.Localize("NET_HOST_SKILL", true);
    if (mMode == mDuelMode) {
        AddChoices(&mSkills, pszFormat, kDuelSkills);
    } else {
        AddChoices(&mSkills, pszFormat, kSkills);
    }
    const int nNewCount = static_cast<int>(mSkills.size());
    if (nOldCount == 0 || nOldCount == nNewCount) {
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

void NetHostingScreen::OnChoiceChanged(int nReset) {
    auto *pList =
        static_cast<SongSelList *>(TheUI.FindComponent(mSongPanelName, kListComponent, false));
    String selected;
    if (!pList->mSongs.empty()) {
        selected = pList->mSongs[pList->mSelected].c_str();
    }

    std::vector<String> choices;
    {
        String choice(TheLocale.Localize(kRandomChoice, true));
        choices.push_back(choice);
    }
    std::vector<SongEntry> songs;
    mChoiceCount = 0;
    if (mMode == mDuelMode) {
        mChoiceCount = kRandomChoiceCount;
        TheGameDb->GetHostSongs(&songs, GameDb::kSkillAny, true);
        mCustomChoice = kNoCustomChoice;
    } else {
        if (mMode == mRemixMode) {
            mChoiceCount = kRandomAndCustomCount;
            TheGameDb->GetHostSongs(&songs, GameDb::kSkillAny, false);
        } else {
            TheGameDb->GetHostSongs(&songs, mSkill, false);
            mChoiceCount = kRandomAndCustomCount;
        }
        if (songs.empty()) {
            choices.clear();
            mCustomChoice = 0;
        } else {
            mCustomChoice = kRandomChoiceCount;
        }
        String choice(TheLocale.Localize(kCustomChoice, true));
        choices.push_back(choice);
    }

    int nSelected = 0;
    for (unsigned int i = 0; i < songs.size(); ++i) {
        SongEntry song = songs[i];
        if (strcmp(selected.c_str(), song.GetName()) == 0 ||
            strcmp(TheGameDb->mSong.c_str(), song.GetName()) == 0) {
            nSelected = static_cast<int>(i) + mChoiceCount;
        }
        String choice(song.GetName());
        choices.push_back(choice);
    }
    if (nReset != 0 || strcmp(selected.c_str(), TheLocale.Localize(kRandomChoice, true)) == 0) {
        nSelected = 0;
    } else if (strcmp(selected.c_str(), TheLocale.Localize(kCustomChoice, true)) == 0) {
        nSelected = mCustomChoice;
    }
    pList->SetSongs(choices, mChoiceCount);
    pList->SetSelected(nSelected);
    mDuelPlayersChoice = 0;
    mLastPlayersChoice = static_cast<int>(mPlayerChoices.size()) - 1;
}

void NetHostingScreen::UpdateLabels() {
    UIPanel *pPanel = mFocusPanel;
    NetParamsScreen::UpdateLabels();
    UIComponent *pPowerup = pPanel->FindComponent(kPowerupComponent, false);
    if (mMode != 0) {
        pPowerup->SetState(UIComponent::kStateDisabled, false);
    } else {
        EnableChoice(pPowerup);
    }
    UIComponent *pPlayers = pPanel->FindComponent(kPlayersComponent, false);
    if (mMode == mDuelMode) {
        mNetPlayersChoice = mDuelPlayersChoice;
        pPlayers->SetState(UIComponent::kStateDisabled, false);
    } else if (!mIsEdit) {
        EnableChoice(pPlayers);
    }
    pPlayers->SetText(mPlayerChoices[mNetPlayersChoice].c_str());
    pPowerup->SetText(mPowerupChoices[mPowerupChoice].c_str());
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
    if (pMsg->mButton != kPadCross) {
        return NetParamsScreen::HandleSelect(pMsg);
    }
    if (strcmp(pMsg->mComponent->mName, kHostComponent) != 0) {
        // The binary also tests for `cursor`, with the same result on both paths.
        UIPanel *pPanel = TheUI.FindPanel(mButtonPanelName, false);
        pPanel->SetFocus(pPanel->FindComponent(kHostComponent, false), kPadNone);
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
    if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
        TheGameDb->SetSkillLevel(mSkill + kDuelSkillOffset);
    } else {
        TheGameDb->SetSkillLevel(mSkill);
    }
    TheGameDb->SetPowerupLevel(mPowerupChoice);
    TheGameDb->SetNetPlayers(mNetPlayersChoice + kMinNetPlayers);

    auto *pList =
        static_cast<SongSelList *>(TheUI.FindComponent(mSongPanelName, kListComponent, false));
    int nSelected = pList->mSelected;
    if (nSelected == mCustomChoice && nSelected < mChoiceCount) {
        TheUI.GotoScreen(mIsEdit ? "load_remix_list_net_edit" : "load_remix_list_net");
        return NetParamsScreen::HandleSelect(pMsg);
    }
    TheGameDb->SetLoadRemix(false);
    if (nSelected == 0) {
        nSelected = RandomInt(mChoiceCount, static_cast<int>(pList->mSongs.size()));
    }
    TheGameDb->SetSong(pList->mSongs[nSelected].c_str());
    if (mMode == mRemixMode) {
        RemixInfo info;
        strncpy(info.mName, "Untitled", kNameCopyLength);
        info.mSource = RemixInfo::kSourceNone;
        info.mDataSize = 0;
        for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
            strncpy(info.mCreators[i], TheGameDb->GetPlayerName(i), kCreatorCopyLength);
        }
        info.mDate.ReadClock();
        info.mReadOnly = 0;
        strcpy(info.mSong, pList->mSongs[nSelected].c_str());
        TheGameDb->SetRemix(&info);
        TheGameDb->SetRemixReadOnly(0);
    }
    if (mIsEdit) {
        ChatMsg::Send(
            kNoticeChannel, kEveryConsole, TheLocale.Localize("host_done_edit_msg", true), kEcho);
        if (TheNetLaunchpad != nullptr) {
            TheNetLaunchpad->EndEdit();
        }
        TheUI.GotoScreen(kLaunchpadScreen);
    } else {
        TheUI.GotoScreen("net_host_attempt");
    }
    return NetParamsScreen::HandleSelect(pMsg);
}

bool NetHostingScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    const char *pszComponent = pMsg->mComponent->mName;
    if (strcmp(pszComponent, kPlayersComponent) == 0) {
        if (pMsg->mButton == kPadDLeft || pMsg->mButton == kPadDRight) {
            mNetPlayersChoice = StepEntry(
                mNetPlayersChoice, static_cast<int>(mPlayerChoices.size()), pMsg->mButton);
        }
        UpdateLabels();
    } else if (strcmp(pszComponent, kPowerupComponent) == 0) {
        if (pMsg->mButton == kPadDLeft || pMsg->mButton == kPadDRight) {
            mPowerupChoice =
                StepEntry(mPowerupChoice, static_cast<int>(mPowerupChoices.size()), pMsg->mButton);
        }
        UpdateLabels();
    }
    return NetParamsScreen::HandleSelectStart(pMsg);
}

bool NetHostingScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->mButton == kPadTriangle && mIsEdit) {
            TheGameDb->SetGameParams(&mParams);
        }
    }
    return NetParamsScreen::HandleJoypad(pMsg);
}
