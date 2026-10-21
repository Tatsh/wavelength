#include "met/netsortedlaunchpadspanel.h"

#include <cstring>

#include "met/netsorteddatapanel.h"
#include "met/netsortedlaunchpadslist.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmesh.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kFlashKind[] = "sub";
constexpr char kNoneText[] = "fn_sorted_none.txt";
constexpr char kListComponent[] = "list";
constexpr char kDataPanel[] = "fn_sorted_pic";
constexpr char kCursorComponent[] = "cursor";
constexpr char kJoinScreen[] = "net_join_lpad";
constexpr char kNoQuery[] = "";

constexpr int kNoMode = 0;
constexpr int kSelectFirst = 0;
constexpr int kSelectLast = 1000;
constexpr int kNoSelection = -1;
constexpr int kNoOrder = -1;
constexpr unsigned int kRowsShown = 7;

void HideArrows(UIList *pList) {
    pList->GetUpArrow()->SetShowing(false);
    pList->SetShowUpArrow(0);
    pList->GetDownArrow()->SetShowing(false);
    pList->SetShowDownArrow(0);
}

} // namespace

NetSortedLaunchpadsPanel::NetSortedLaunchpadsPanel(DataArray *pData, const char *pszDir)
    : FreqPanel(pData, pszDir), mArena(kNoQuery) {
    mSkillLevel = kNoOrder;
    mRuleSet = 0;
    mHasPrevious = 0;
    mPage = kPageFirst;
}

void NetSortedLaunchpadsPanel::SetSearch(const char *pszArena, int nRuleSet, int nSkillLevel) {
    mArena = pszArena;
    mSkillLevel = nSkillLevel;
    mRuleSet = nRuleSet;
}

void NetSortedLaunchpadsPanel::Enter(bool bForce, float fTime) {
    FreqPanel::Enter(bForce, fTime);
    InitFlash(mName, kFlashKind);
    mHasPrevious = 0;
    mPage = kPageFirst;
    TheNetLobby->RequestLaunchpads(
        this, kPageFirst, kNoMode, mArena.c_str(), mRuleSet, mSkillLevel);
    mHilite = 0;
    mNoneText = dynamic_cast<RndText *>(TheManager.Find(kNoneText));
    mNoneText->SetShowing(false);
    HideArrows(static_cast<UIList *>(FindComponent(kListComponent, false)));
}

void NetSortedLaunchpadsPanel::Request(int nPage) {
    if (nPage == kPagePrevious && mHasPrevious == 0) {
        return;
    }
    mPage = nPage;
    static_cast<NetSortedDataPanel *>(TheUI.FindPanel(kDataPanel, false))->Reset();
    TheNetLobby->RequestLaunchpads(this, nPage, kNoMode, mArena.c_str(), mRuleSet, mSkillLevel);
    StartFlash(TheUI.GetTime(), false);
    HideArrows(static_cast<UIList *>(FindComponent(kListComponent, false)));
}

bool NetSortedLaunchpadsPanel::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nLobbyLaunchpadsMsgType) {
        return HandleLobbyLaunchpads(static_cast<LobbyLaunchpadsMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return UIPanel::DispatchPriv(pMsg);
}

bool NetSortedLaunchpadsPanel::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->GetPressed() == 0) {
        return false;
    }
    UIScreen *pScreen = TheUI.GetCurrentScreen();
    if (pScreen->GetNextScreen() != nullptr || pScreen->GetPrevScreen() != nullptr) {
        return true;
    }
    if (pMsg->GetButton() == kPadL1) {
        Request(kPageRefresh);
    }
    return false;
}

bool NetSortedLaunchpadsPanel::HandleLobbyLaunchpads(LobbyLaunchpadsMsg *pMsg) {
    if (mLoaded == 0) {
        return false;
    }
    int nSelected;
    if (mPage == kPageRefresh) {
        nSelected = kNoSelection;
    } else if (mPage == kPagePrevious) {
        nSelected = kSelectLast;
    } else {
        nSelected = kSelectFirst;
    }
    mNoneText->SetShowing(pMsg->GetLaunchpads()->empty());

    NetSortedLaunchpadsList *pList =
        static_cast<NetSortedLaunchpadsList *>(FindComponent(kListComponent, false));
    pList->SetLaunchpads(pMsg->GetLaunchpads(), nSelected);
    mHasPrevious = pMsg->GetHasPrevious();
    const unsigned int nCount = pMsg->GetLaunchpads()->size();
    pList->GetUpArrow()->SetShowing(mHasPrevious != 0);
    pList->SetShowUpArrow(mHasPrevious != 0);
    pList->GetDownArrow()->SetShowing(nCount >= kRowsShown);
    pList->SetShowDownArrow(nCount >= kRowsShown);
    HideFlash();
    return false;
}

bool NetSortedLaunchpadsPanel::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->GetButton() == kPadCross &&
        std::strcmp(pMsg->GetComponent()->GetName(), kCursorComponent) == 0) {
        TheUI.GotoScreen(kJoinScreen);
    }
    return false;
}
