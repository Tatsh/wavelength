#include "app/hudtextnode.h"

#include "rnd/rndfont.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmat.h"

HudTextNode::HudTextNode(const char *pszText, RndView *pHudView) {
    mText = nullptr;
    mText = dynamic_cast<RndText *>(TheManager.Find(pszText));
    static_cast<RndDrawable *>(pHudView)->RemoveDraw(mText);
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
    if (mText->GetShowing() == 0) {
        return;
    }
    mText->GetFont()->GetMat()->SetAlpha(1.0f);
    mText->Draw();
}
