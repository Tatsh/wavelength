#include "app/hudtextnode.h"

#include "rnd/font.h"
#include "rnd/manager.h"
#include "rnd/mat.h"

HudTextNode::HudTextNode(const char *pszText, Rnd::View *pHudView) {
    mText = nullptr;
    mText = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(pszText));
    static_cast<Rnd::Drawable *>(pHudView)->RemoveDraw(mText);
    mText->SetShowing(false);
}

void HudTextNode::SetText(const char *pszText) {
    if (pszText == nullptr || pszText[0] == '\0') {
        mText->SetShowing(false);
        return;
    }
    mText->SetText(pszText);
    mText->SetShowing(true);
}

void HudTextNode::Hide() {
    mText->SetShowing(false);
}

void HudTextNode::Draw() {
    if (mText->mShowing == 0) {
        return;
    }
    mText->mFont->mMat->SetAlpha(1.0f);
    mText->Draw();
}
