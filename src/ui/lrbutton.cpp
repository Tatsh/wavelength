#include "ui/lrbutton.h"

#include <cmath>

#include "os/joypad.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uimanager.h"

namespace {

// Indices of a button's description.
constexpr int kBaseIndex = 3;
constexpr int kArrowStyleIndex = 4;

} // namespace

int LRButton::sArrowFlashes = 1;

LRButton::LRButton(DataArray *pData, const char *pszPanel) : UIButton(pData, pszPanel) {
    mArrowButton = kPadNone;
    mArrowFlashDone = true;
    mArrowStyle = TheUI.FindStyle(pData->Sym(kArrowStyleIndex), false);
    const char *pszBase = pData->Sym(kBaseIndex);
    mLeftArrow = dynamic_cast<Rnd::Mesh *>(
        Rnd::TheManager.Find(FormatString("%s_%s_left.mesh", pszPanel, pszBase)));
    if (mLeftArrow != nullptr) {
        mLeftArrow->SetMat(mStyle->GetMat(mState)); // The button's style, not mArrowStyle.
    }
    mRightArrow = dynamic_cast<Rnd::Mesh *>(
        Rnd::TheManager.Find(FormatString("%s_%s_right.mesh", pszPanel, pszBase)));
    if (mRightArrow != nullptr) {
        mRightArrow->SetMat(mStyle->GetMat(mState));
    }
}

LRButton::~LRButton() {
}

void LRButton::StartFlash(float fTime,
                          int nFlashes,
                          int nSelectedState,
                          int nNormalState,
                          float fSelectedMs,
                          float fNormalMs) {
    UIButton::StartFlash(fTime, nFlashes, nSelectedState, nNormalState, fSelectedMs, fNormalMs);
}

void LRButton::SetState(int nState, bool bForce) {
    if (!bForce && GetState() == nState) {
        return;
    }
    UIButton::SetState(nState, bForce);
    if (mLeftArrow != nullptr) {
        mLeftArrow->SetMat(mArrowStyle->GetMat(nState));
    }
    if (mRightArrow != nullptr) {
        mRightArrow->SetMat(mArrowStyle->GetMat(nState));
    }
}

void LRButton::Unfocus() {
    if (mFlashDone) {
        return;
    }
    SetState(mRevertState, true);
    mArrowFlashDone = true;
    mFlashDone = true;
}

void LRButton::SetShowing(bool bShowing) {
    UIButton::SetShowing(bShowing);
    if (mLeftArrow != nullptr) {
        mLeftArrow->SetShowing(bShowing);
    }
    if (mRightArrow != nullptr) {
        mRightArrow->SetShowing(bShowing);
    }
}

bool LRButton::SetArrowState(int nState) {
    if (mLeftArrow != nullptr && mArrowButton == kPadDLeft) {
        mLeftArrow->SetMat(mArrowStyle->GetMat(nState));
        return true;
    }
    if (mRightArrow != nullptr && mArrowButton == kPadDRight) {
        mRightArrow->SetMat(mArrowStyle->GetMat(nState));
        return true;
    }
    return false;
}

void LRButton::Poll(float fTime) {
    UIButton::Poll(fTime);
    if (mArrowButton == kPadNone || mArrowFlashDone) {
        return;
    }
    if (mFlashEnd <= fTime) {
        SetArrowState(mRevertState);
        mArrowFlashDone = true;
        mArrowButton = kPadNone;
        UIPanel *pPanel = TheUI.FocusPanel();
        // The button is read after it is cleared, so the message carries kPadNone.
        UIComponentSelectMsg msg(this, pPanel, TheUI.mCurrentScreen, mArrowButton);
        Dispatch(&msg);
        return;
    }
    const float fPeriod = mFlashPeriod;
    float fPhase = std::fmod(fTime - mFlashStart, fPeriod);
    if (fPhase < 0.0f) {
        fPhase += fPeriod;
    }
    SetArrowState(fPhase < mSelectedMs ? mSelectedState : mNormalState);
}

void LRButton::SetArrowShowing(int nArrow, bool bShowing) {
    if (nArrow == kArrowLeft) {
        mLeftArrow->SetShowing(bShowing);
    } else {
        mRightArrow->SetShowing(bShowing);
    }
}

bool LRButton::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleSelect(static_cast<JoypadInputMsg *>(pMsg));
    }
    return UIComponent::DispatchPriv(pMsg);
}

bool LRButton::HandleSelect(JoypadInputMsg *pMsg) {
    if (UIButton::HandleSelect(pMsg)) {
        return true;
    }
    const bool bArrow = (mLeftArrow != nullptr && pMsg->mButton == kPadDLeft) ||
                        (mRightArrow != nullptr && pMsg->mButton == kPadDRight);
    if (bArrow && !mFlashDone) {
        mFlashDone = true;
        if (mArrowButton == kPadDLeft) {
            mLeftArrow->SetMat(mArrowStyle->GetMat(mRevertState));
        } else if (mArrowButton == kPadDRight) {
            mRightArrow->SetMat(mArrowStyle->GetMat(mRevertState));
        }
        SetState(mRevertState, true);
        mArrowButton = kPadNone;
        mArrowFlashDone = true;
    }
    if (!bArrow || pMsg->mPressed == 0) {
        return false;
    }
    UIScreen *pScreen = TheUI.mCurrentScreen;
    UIComponentSelectStartMsg msg(this, TheUI.FocusPanel(), pScreen, pMsg->mButton, pMsg->mPad);
    (void)Dispatch(&msg); // The arrow flashes even when a handler stops the choice.
    if (mArrowButton == kPadDLeft) {
        mLeftArrow->SetMat(mArrowStyle->GetMat(mRevertState));
    } else if (mArrowButton == kPadDRight) {
        mRightArrow->SetMat(mArrowStyle->GetMat(mRevertState));
    }
    mArrowButton = pMsg->mButton;
    StartFlash(TheUI.mTime, sArrowFlashes, kStateSelected, kStateNormal, sSelectedMs, sNormalMs);
    mFlashDone = true;
    mArrowFlashDone = false;
    return true;
}
