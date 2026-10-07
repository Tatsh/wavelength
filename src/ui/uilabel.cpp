#include "ui/uilabel.h"

#include "os/string.h"
#include "rnd/manager.h"
#include "ui/uimanager.h"

namespace {

// Indices of a label's description.
constexpr int kBaseIndex = 2;
constexpr int kStyleIndex = 3;

} // namespace

UILabel::UILabel(DataArray *pData, const char *pszPanel) : UIComponent(pData) {
    if (pData->Size() > kStyleIndex) {
        mStyle = TheUI.FindStyle(pData->Sym(kStyleIndex), false);
    } else {
        mStyle = nullptr;
    }
    const char *pszBase = pData->Size() > kBaseIndex ? pData->Sym(kBaseIndex) : mName;
    mText = dynamic_cast<Rnd::Text *>(
        Rnd::TheManager.Find(FormatString("%s_%s.txt", pszPanel, pszBase)));
    if (mText != nullptr && mStyle != nullptr) {
        mText->SetFont(mStyle->GetFont(mState));
    }
}

void UILabel::SetShowing(bool bShowing) {
    mShowing = bShowing;
    mText->SetShowing(bShowing);
}

const char *UILabel::Text() const {
    return mText->mPreWrapText.mStr;
}

void UILabel::SetText(const char *pszText) {
    mText->SetText(pszText);
}

void UILabel::SetStyle(UIStyle *pStyle) {
    mStyle = pStyle;
    if (mText != nullptr) {
        mText->SetFont(pStyle->GetFont(mState));
    }
}

void UILabel::SetState(int nState, bool bForce) {
    if (!bForce && GetState() == nState) {
        return;
    }
    mState = nState;
    if (mText != nullptr) {
        mText->SetFont(mStyle->GetFont(nState));
    }
}

void UILabel::Print(PrnStream &stream) {
    stream << "{UILabel " << mName << "\n";
    stream << "   visible: " << mShowing << "\n";
    stream << "   RndText object: " << mText->mName.mStr << "\n";
    stream << "}\n";
}
