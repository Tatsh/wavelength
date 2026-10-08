#include "met/netmainpanel.h"

#include "os/joypad.h"
#include "os/system.h"
#include "ui/uimanager.h"
#include "ui/uiscreen.h"

namespace {

constexpr char kFlashPanelTag[] = "hilite_flash_panel";
constexpr char kMainPanel[] = "fn_main";

// The milliseconds by which Enter() and RequestNow() set the request time before the clock.
constexpr float kRequestLeadMs = 1.0f;

} // namespace

NetMainPanel::NetMainPanel(DataArray *pData, const char *pszDir)
    : FocusChangePanel(pData, pszDir), mRequestTime(0.0f) {
    pData->FindBool(kFlashPanelTag, &mFlashPanel, false);
}

void NetMainPanel::FinishLoad() {
    FreqPanel::FinishLoad();
    if (mFlashPanel != 0) {
        InitFlash(mName, mMaterialPrefix.c_str());
    }
}

void NetMainPanel::Exit(bool bForce, float fTime) {
    mRequestTime = 0.0f;
    FreqPanel::Exit(bForce, fTime);
}

void NetMainPanel::Enter(bool bForce, float fTime) {
    FreqPanel::Enter(bForce, fTime);
    Unfocus();
    mRequestTime = SystemMs() - kRequestLeadMs;
    if (mFlashPanel != 0) {
        mRefresh->SetShowing(false);
        mTitles->SetShowing(true);
    }
}

void NetMainPanel::Poll(float fTime) {
    FreqPanel::Poll(fTime);
    if (mRequestTime != 0.0f && mRequestTime < SystemMs()) {
        mRequestTime = 0.0f;
        RequestUpdate();
    }
    if (mFlashPanel != 0) {
        PollFlash(fTime);
    }
}

void NetMainPanel::RequestNow(bool bHilite) {
    mRequestTime = SystemMs() - kRequestLeadMs;
    if (mFlashPanel != 0) {
        StartFlash(TheUI.mTime, bHilite);
    }
}

bool NetMainPanel::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return UIPanel::DispatchPriv(pMsg);
}

bool NetMainPanel::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return false;
    }
    UIScreen *pScreen = TheUI.mCurrentScreen;
    if (pScreen->mNextScreen != nullptr || pScreen->mPrevScreen != nullptr) {
        return true;
    }
    const int nButton = pMsg->mButton;
    if (nButton == kPadTriangle || nButton == kPadDLeft) {
        pScreen->SetFocus(TheUI.FindPanel(kMainPanel, false));
    } else if (nButton == kPadL1) {
        RequestNow(true);
    }
    return false;
}
