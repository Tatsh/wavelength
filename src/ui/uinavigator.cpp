#include "ui/uinavigator.h"

#include <cstring>

#include "os/joypad.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uipanel.h"

namespace {

// Index of the list type in a list entry, and of its first component name.
constexpr int kListTypeIndex = 0;
constexpr int kFirstNameIndex = 1;

constexpr int kNotFound = -1;

} // namespace

UINavigator::UINavigator(DataArray *pData, UIPanel *pPanel) {
    mEnabled = true;
    mPanel = pPanel;
    mWrap = true;
    for (int i = 1; i < pData->Size(); ++i) {
        DataArray *pList = pData->Array(i);
        bool bVertical = false;
        const char *pszType = pList->Sym(kListTypeIndex);
        if (std::strcmp(pszType, "horizontal") != 0) {
            bVertical = std::strcmp(pszType, "vertical") == 0;
        }
        std::vector<UIComponent *> row;
        for (int j = kFirstNameIndex; j < pList->Size(); ++j) {
            row.push_back(pPanel->FindComponent(pList->Sym(j), true));
        }
        Highlight(mPanel->mFocus, row);
        if (bVertical) {
            mVertical.push_back(row);
        } else {
            mHorizontal.push_back(row);
        }
    }
    // mRow stays unset when the description has no list.
    if (!mHorizontal.empty()) {
        mRow = &mHorizontal.front();
    } else if (!mVertical.empty()) {
        mRow = &mVertical.front();
    }
}

UINavigator::~UINavigator() {
    for (unsigned int i = 0; i < mVertical.size(); ++i) {
        mVertical[i].clear();
    }
    for (unsigned int i = 0; i < mHorizontal.size(); ++i) {
        mHorizontal[i].clear();
    }
}

bool UINavigator::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return false;
}

int UINavigator::FindFocus(int nAxis) {
    UIComponent *pFocus = mPanel->mFocus;
    int nIndex = kNotFound;
    std::vector<UIComponent *> *pRow = nullptr;
    if (pFocus != nullptr) {
        std::vector<std::vector<UIComponent *> > &lists =
            nAxis == kAxisVertical ? mVertical : mHorizontal;
        const int nLists = static_cast<int>(lists.size());
        for (int i = 0; i < nLists && nIndex == kNotFound; ++i) {
            pRow = &lists[i];
            for (unsigned int j = 0; j < pRow->size(); ++j) {
                if ((*pRow)[j] == pFocus) {
                    nIndex = static_cast<int>(j);
                    break;
                }
            }
        }
    }
    if (nIndex != kNotFound) {
        mRow = pRow;
    }
    return nIndex;
}

bool UINavigator::HandleJoypad(JoypadInputMsg *pMsg) {
    if (!mEnabled || !pMsg->mPressed) {
        return false;
    }
    int nAxis;
    bool bForward;
    switch (pMsg->mButton) {
    case kPadDUp:
        nAxis = kAxisVertical;
        bForward = false;
        break;
    case kPadDRight:
        nAxis = kAxisHorizontal;
        bForward = true;
        break;
    case kPadDDown:
        nAxis = kAxisVertical;
        bForward = true;
        break;
    case kPadDLeft:
        nAxis = kAxisHorizontal;
        bForward = false;
        break;
    default:
        return false;
    }
    const int nIndex = FindFocus(nAxis);
    if (nIndex != kNotFound) {
        Step(bForward, pMsg->mButton, nIndex);
    }
    return false;
}

bool UINavigator::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    for (unsigned int i = 0; i < mHorizontal.size(); ++i) {
        Highlight(pMsg->mComponent, mHorizontal[i]);
    }
    for (unsigned int i = 0; i < mVertical.size(); ++i) {
        Highlight(pMsg->mComponent, mVertical[i]);
    }
    return false;
}

bool UINavigator::HandleSelectStart([[maybe_unused]] UIComponentSelectStartMsg *pMsg) {
    mEnabled = false;
    return false;
}

bool UINavigator::HandleSelect([[maybe_unused]] UIComponentSelectMsg *pMsg) {
    mEnabled = true;
    return false;
}

void UINavigator::Highlight(UIComponent *pFocus, std::vector<UIComponent *> &row) {
    for (auto it = row.begin(); it != row.end(); ++it) {
        UIComponent *pComponent = *it;
        if (pComponent->GetState() == UIComponent::kStateDisabled) {
            continue;
        }
        if (pComponent == pFocus) {
            pComponent->SetState(UIComponent::kStateSelected, false);
        } else {
            pComponent->SetState(UIComponent::kStateNormal, false);
        }
    }
}

void UINavigator::Step(bool bForward, int nButton, int nIndex) {
    if (nIndex == kNotFound) {
        return;
    }
    const int nStep = bForward ? 1 : -1;
    int nNext = nIndex;
    for (;;) {
        nNext += nStep;
        if (mWrap) {
            const int nCount = static_cast<int>(mRow->size());
            const int nRemainder = nNext % nCount;
            nNext = nRemainder > -1 ? nRemainder : nRemainder + nCount;
        } else if (nNext < 0 || nNext >= static_cast<int>(mRow->size())) {
            return;
        }
        if (nNext == nIndex) {
            return;
        }
        UIComponent *pComponent = (*mRow)[nNext];
        if (pComponent->GetState() == UIComponent::kStateDisabled || !pComponent->IsShowing()) {
            continue;
        }
        mPanel->SetFocus(pComponent, nButton);
        return;
    }
}
