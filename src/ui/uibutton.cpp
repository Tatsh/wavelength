#include "ui/uibutton.h"

#include <cmath>

#include "os/joypad.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uimanager.h"

namespace {

// Indices of a button's description.
constexpr int kStyleIndex = 2;
constexpr int kBaseIndex = 3;

} // namespace

UIButton::UIButton(DataArray *pData, const char *pszPanel) : UIComponent(pData) {
    mFlashDone = true;
    mStyle = TheUI.FindStyle(pData->Sym(kStyleIndex), false);
    const char *pszBase = pData->Size() > kBaseIndex ? pData->Sym(kBaseIndex) : mName;
    mText = dynamic_cast<Rnd::Text *>(
        Rnd::TheManager.Find(FormatString("%s_%s.txt", pszPanel, pszBase)));
    if (mText != nullptr) {
        mText->SetFont(mStyle->GetFont(mState));
    }
    mMesh = dynamic_cast<Rnd::Mesh *>(
        Rnd::TheManager.Find(FormatString("%s_%s.mesh", pszPanel, pszBase)));
    if (mMesh != nullptr) {
        mMesh->SetMat(mStyle->GetMat(mState));
    }
}

UIButton::~UIButton() {
}

const char *UIButton::Text() const {
    return mText->mPreWrapText.mStr;
}

void UIButton::SetText(const char *pszText) {
    mText->SetText(pszText);
}

void UIButton::SetStyle(UIStyle *pStyle) {
    mStyle = pStyle;
    SetState(GetState(), true);
}

void UIButton::SetShowing(bool bShowing) {
    mShowing = bShowing;
    if (mText != nullptr) {
        mText->SetShowing(bShowing);
    }
    if (mMesh != nullptr) {
        mMesh->SetShowing(bShowing);
    }
}

void UIButton::SetState(int nState, bool bForce) {
    if (!bForce && GetState() == nState) {
        return;
    }
    mState = nState;
    if (mMesh != nullptr) {
        mMesh->SetMat(mStyle->GetMat(nState));
    }
    if (mText != nullptr) {
        mText->SetFont(mStyle->GetFont(nState));
    }
}

bool UIButton::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleSelect(static_cast<JoypadInputMsg *>(pMsg));
    }
    return UIComponent::DispatchPriv(pMsg);
}

bool UIButton::HandleSelect(JoypadInputMsg *pMsg) {
    if (pMsg->mButton != kPadCross || !mFlashDone || pMsg->mPressed == 0) {
        return false;
    }
    UIScreen *pScreen = TheUI.mCurrentScreen;
    UIComponentSelectStartMsg msg(this, TheUI.FocusPanel(), pScreen, pMsg->mButton, pMsg->mPad);
    if (!Dispatch(&msg)) {
        StartFlash(TheUI.mTime, sNumFlashes, kStateSelected, kStateNormal, sSelectedMs, sNormalMs);
    }
    return true;
}

void UIButton::Unfocus() {
    if (mFlashDone) {
        return;
    }
    SetState(mRevertState, true);
    mFlashDone = true;
}

void UIButton::Poll(float fTime) {
    if (mFlashDone) {
        return;
    }
    if (mFlashEnd <= fTime) {
        SetState(mRevertState, true);
        mFlashDone = true;
        UIPanel *pPanel = TheUI.FocusPanel();
        UIComponentSelectMsg msg(this, pPanel, TheUI.mCurrentScreen, kPadCross);
        Dispatch(&msg);
        return;
    }
    const float fPeriod = mFlashPeriod;
    float fPhase = std::fmod(fTime - mFlashStart, fPeriod);
    if (fPhase < 0.0f) {
        fPhase += fPeriod;
    }
    SetState(fPhase < mSelectedMs ? mSelectedState : mNormalState, false);
}

void UIButton::StartFlash(float fTime,
                          int nFlashes,
                          int nSelectedState,
                          int nNormalState,
                          float fSelectedMs,
                          float fNormalMs) {
    mSelectedState = nSelectedState;
    mNormalState = nNormalState;
    mFlashPeriod = fSelectedMs + fNormalMs;
    mFlashStart = fTime;
    mSelectedMs = fSelectedMs;
    mNormalMs = fNormalMs;
    mFlashDone = false;
    mRevertState = GetState();
    mFlashStart -= fSelectedMs;
    mFlashEnd = fTime + static_cast<float>(nFlashes) * mFlashPeriod + fNormalMs;
}

void UIButton::Print(PrnStream &stream) {
    stream << "{UIButton " << mName << "\n"
           << "   visible: " << mShowing << "\n"
           << "   RndButton object: " << mName << " (state: " << GetState() << ")\n";
    if (!mFlashDone) {
        stream << "   button flash characteristics:\n"
               << "      flash start frame:    " << mFlashStart << "\n"
               << "      flash end frame:      " << mFlashEnd << "\n"
               << "      flash period:         " << mFlashPeriod << "\n"
               << "      frames selected:      " << mSelectedMs << "\n"
               << "      frames normal:        " << mNormalMs << "\n"
               << "      flash normal state:   " << mNormalState << "\n"
               << "      flash selected state: " << mSelectedState << "\n"
               << "      flash revert state:   " << mRevertState << "\n";
    }
    stream << "}\n";
}
