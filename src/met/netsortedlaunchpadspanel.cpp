#include "met/netsortedlaunchpadspanel.h"

#include <cstring>

#include "met/netsorteddatapanel.h"
#include "met/netsortedlaunchpadslist.h"
#include "netflow/netlobby.h"
#include "os/joypad.h"
#include "rnd/manager.h"
#include "rnd/mesh.h"
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
    pList->mUpArrow->SetShowing(false);
    pList->mShowUpArrow = false;
    pList->mDownArrow->SetShowing(false);
    pList->mShowDownArrow = false;
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
    mNoneText = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(kNoneText));
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
    StartFlash(TheUI.mTime, false);
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
    if (pMsg->mPressed == 0) {
        return false;
    }
    UIScreen *pScreen = TheUI.mCurrentScreen;
    if (pScreen->mNextScreen != nullptr || pScreen->mPrevScreen != nullptr) {
        return true;
    }
    if (pMsg->mButton == kPadL1) {
        Request(kPageRefresh);
    }
    return false;
}

bool NetSortedLaunchpadsPanel::HandleLobbyLaunchpads(LobbyLaunchpadsMsg *pMsg) {
    if (mData == nullptr) {
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
    mNoneText->SetShowing(pMsg->mLaunchpads->empty());

    NetSortedLaunchpadsList *pList =
        static_cast<NetSortedLaunchpadsList *>(FindComponent(kListComponent, false));
    pList->SetLaunchpads(pMsg->mLaunchpads, nSelected);
    mHasPrevious = pMsg->mHasPrevious;
    const unsigned int nCount = pMsg->mLaunchpads->size();
    pList->mUpArrow->SetShowing(mHasPrevious != 0);
    pList->mShowUpArrow = mHasPrevious != 0;
    pList->mDownArrow->SetShowing(nCount >= kRowsShown);
    pList->mShowDownArrow = nCount >= kRowsShown;
    HideFlash();
    return false;
}

bool NetSortedLaunchpadsPanel::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross && std::strcmp(pMsg->mComponent->mName, kCursorComponent) == 0) {
        TheUI.GotoScreen(kJoinScreen);
    }
    return false;
}
