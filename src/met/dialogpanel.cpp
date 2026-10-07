#include "met/dialogpanel.h"

#include <string.h>

#include "os/joypad.h"
#include "os/locale.h"
#include "rnd/manager.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "ui/uicomponent.h"

namespace {

// The rows 2 to 4 of the box start hidden.
constexpr int kFirstHiddenRow = 2;
constexpr int kLastHiddenRow = 4;

// The text of the panel's name is the second node of the description.
constexpr int kNameNode = 1;

// The height view reaches its last frame, 100, at 12 rows.
constexpr float kRowsPerHeight = 12.0f;
constexpr float kHeightFrames = 100.0f;

// The frames of the button view for no buttons, one or two buttons, and three buttons.
constexpr float kNoButtonsFrame = 25.0f;
constexpr float kTwoButtonsFrame = 50.0f;
constexpr float kThreeButtonsFrame = 75.0f;

// A box with this many buttons needs a row for them below the text.
constexpr unsigned kButtonRowCount = 3;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(Rnd::TheManager.Find(pszName));
}

} // namespace

DialogPanel::DialogPanel(DataArray *pData, const char *pszDir)
    : FreqPanel(pData, pszDir), mTriCancel(0) {
    mFile.Clear();
    mFile.Printf("%s/dialog.rnd", pszDir);
    pData->FindBool("tri_cancel", &mTriCancel, false);
}

DialogPanel::~DialogPanel() {
}

void DialogPanel::FinishLoad() {
    if (mLoaded) {
        return;
    }
    mName = "dialog";
    const char *pszFocus = "";
    mData->FindSymbol("focus", &pszFocus, false);
    FreqPanel::FinishLoad();
    if (pszFocus != nullptr && strcmp(pszFocus, "") != 0) {
        SetFocus(FindComponent(pszFocus, false), kPadNone);
    }
    mName = mData->Sym(kNameNode);
    mPanelView = FindObject<Rnd::View>("dialog_panel.view");
    mButtonView = FindObject<Rnd::View>("dialog_text.view");
}

void DialogPanel::Enter(bool bForce, float fTime) {
    FreqPanel::Enter(bForce, fTime);
    for (int i = kFirstHiddenRow; i <= kLastHiddenRow; ++i) {
        FindObject<Rnd::Mesh>(FormatString("dialog_%02d.mesh", i))->SetShowing(false);
        FindObject<Rnd::Text>(FormatString("dialog_%02d.txt", i))->SetShowing(false);
    }
    FindObject<Rnd::Text>("dialog_05a.txt")->SetShowing(mTriCancel);
    FindObject<Rnd::Text>("dialog_05b.txt")->SetShowing(mTriCancel);
    if (strcmp(mPendingText.c_str(), "") == 0) {
        SetText(TheLocale.Localize(mName, false));
    } else {
        SetText(mPendingText.c_str());
        mPendingText = "";
    }
}

void DialogPanel::SetText(const char *pszText) {
    if (!IsLoaded()) {
        mPendingText = pszText;
        return;
    }

    Rnd::Text *pText = FindObject<Rnd::Text>("dialog_01.txt");
    pText->SetShowing(true);
    pText->SetText(pszText);
    const int nLines = pText->CountLines();
    const int nRows = mComponents.size() == kButtonRowCount ? nLines + 1 : nLines;
    mPanelView->SetFrame(static_cast<float>(nRows) / kRowsPerHeight * kHeightFrames);
    UpdateBgBox();

    switch (mComponents.size()) {
    case 0:
        mButtonView->SetFrame(mTriCancel == 0 ? kNoButtonsFrame : kTwoButtonsFrame);
        break;
    case 1:
    case 2:
        mButtonView->SetFrame(kTwoButtonsFrame);
        break;
    case kButtonRowCount:
        mButtonView->SetFrame(kThreeButtonsFrame);
        break;
    default:
        break;
    }

    UIComponent *pFocus = mFocus;
    for (const auto &entry : mComponents) {
        UIComponent *pComponent = entry.second;
        pComponent->SetShowing(true);
        pComponent->SetText(TheLocale.Localize(pComponent->mName, false));
        pComponent->SetState(UIComponent::kStateNormal, true);
    }
    if (pFocus != nullptr) {
        SetFocus(pFocus, kPadNone);
        pFocus->SetState(UIComponent::kStateSelected, true);
    }
}
