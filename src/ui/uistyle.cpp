#include "ui/uistyle.h"

#include "rnd/manager.h"
#include "ui/uicomponent.h"

namespace {

// Indices of an entry's state array.
constexpr int kStateMatIndex = 1;
constexpr int kStateFontIndex = 2;

// Resolve the material and the font a state array lists.
void FindStateObjects(DataArray *pState, Rnd::Mat **ppMat, Rnd::Font **ppFont) {
    *ppMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(pState->Sym(kStateMatIndex)));
    *ppFont = dynamic_cast<Rnd::Font *>(Rnd::TheManager.Find(pState->Sym(kStateFontIndex)));
}

} // namespace

UIStyle::UIStyle(DataArray *pData)
    : mNormalMat(nullptr), mSelectedMat(nullptr), mGreyMat(nullptr), mNormalFont(nullptr),
      mSelectedFont(nullptr), mGreyFont(nullptr) {
    mName = pData->Sym(1);

    Rnd::Mat *pMat;
    Rnd::Font *pFont;
    DataArray *pState = pData->FindArray("normal", false);
    if (pState != nullptr) {
        FindStateObjects(pState, &pMat, &pFont);
        SetNormal(pMat, pFont);
    }
    pState = pData->FindArray("selected", false);
    if (pState != nullptr) {
        FindStateObjects(pState, &pMat, &pFont);
        SetSelected(pMat, pFont);
    }
    pState = pData->FindArray("grey", false);
    if (pState != nullptr) {
        FindStateObjects(pState, &pMat, &pFont);
        SetGrey(pMat, pFont);
    }
}

Rnd::Mat *UIStyle::GetMat(int nState) const {
    if (nState == UIComponent::kStateNormal) {
        return mNormalMat;
    }
    if (nState == UIComponent::kStateSelected) {
        return mSelectedMat;
    }
    return mGreyMat;
}

Rnd::Font *UIStyle::GetFont(int nState) const {
    if (nState == UIComponent::kStateNormal) {
        return mNormalFont;
    }
    if (nState == UIComponent::kStateSelected) {
        return mSelectedFont;
    }
    return mGreyFont;
}

void UIStyle::SetNormal(Rnd::Mat *pMat, Rnd::Font *pFont) {
    mNormalFont = pFont;
    mNormalMat = pMat;
}

void UIStyle::SetSelected(Rnd::Mat *pMat, Rnd::Font *pFont) {
    mSelectedFont = pFont;
    mSelectedMat = pMat;
}

void UIStyle::SetGrey(Rnd::Mat *pMat, Rnd::Font *pFont) {
    mGreyFont = pFont;
    mGreyMat = pMat;
}
