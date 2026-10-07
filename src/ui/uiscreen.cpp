#include "ui/uiscreen.h"

#include "os/debug.h"
#include "os/joypad.h"
#include "os/string.h"
#include "script/symbol.h"
#include "ui/uimanager.h"
#include "ui/uitransitioncompletemsg.h"

namespace {

// Indices of a description entry and of one entry of its transition table.
constexpr int kNameIndex = 1;
constexpr int kTransitionTriggerIndex = 0;
constexpr int kTransitionScreenIndex = 1;

} // namespace

UIScreen::UIScreen(DataArray *pData)
    : mGroup(nullptr), mPrevScreen(nullptr), mNextScreen(nullptr), mForceEntryExit(false) {
    mFocusPanel = nullptr;
    mName = pData->Sym(kNameIndex);

    DataArray *pPanels = pData->FindArray("panels", false);
    if (pPanels != nullptr) {
        for (int i = 1; i < pPanels->Size(); ++i) {
            // Yes, the binary walks a nested array of the list without using it.
            if (pPanels->Type(i) == DataArray::kNodeSymbol) {
                AddPanel(TheUI.FindPanel(pPanels->Sym(i), true));
            }
        }
    }

    DataArray *pFocus = pData->FindArray("focus", false);
    if (pFocus != nullptr) {
        SetFocus(TheUI.FindPanel(pFocus->Sym(kNameIndex), true));
    }

    DataArray *pTransitions = pData->FindArray("screen_transitions", false);
    if (pTransitions != nullptr) {
        mTransitions.resize(pTransitions->Size() - 1);
        for (int i = 1; i < pTransitions->Size(); ++i) {
            DataArray *pEntry = pTransitions->Array(i);
            const char *pszScreen = pEntry->Sym(kTransitionScreenIndex);
            const char *pszComponent = nullptr;
            int nButton = kPadNone;
            if (pEntry->Type(kTransitionTriggerIndex) == DataArray::kNodeInt) {
                nButton = pEntry->Int(kTransitionTriggerIndex);
            } else {
                pszComponent = pEntry->Sym(kTransitionTriggerIndex);
            }
            mTransitions[i - 1] = Transition{pszScreen, pszComponent, nButton};
        }
    }

    pData->FindBool("force_entry_exit", &mForceEntryExit, false);
    pData->FindSymbol("group", &mGroup, false);
    TheUI.VerifyGroup(&mGroup);

    if (mFocusPanel == nullptr && !mPanels.empty()) {
        SetFocus(mPanels.begin()->second);
    }
}

UIScreen::~UIScreen() {
}

void UIScreen::ClearTransitions() {
    mTransitions.clear();
}

void UIScreen::AddTransition(const char *pszComponent, int nButton, const char *pszScreen) {
    const char *pszComponentSymbol = LookupSymbol(pszComponent);
    mTransitions.push_back(Transition{LookupSymbol(pszScreen), pszComponentSymbol, nButton});
}

void UIScreen::AddPanel(UIPanel *pPanel) {
    mPanels[pPanel->mName] = pPanel;
}

bool UIScreen::Dispatch(Message *pMsg) {
    const bool bNavigation = TheUI.IsNavigationMsg(pMsg);
    if (!bNavigation && TheUI.Dispatch(pMsg)) {
        return true;
    }
    if (DispatchPriv(pMsg)) {
        return true;
    }
    if (!bNavigation) {
        return false;
    }
    return mFocusPanel != nullptr && mFocusPanel->Dispatch(pMsg);
}

bool UIScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return false;
}

bool UIScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return false;
    }
    if (mNextScreen != nullptr || mPrevScreen != nullptr) {
        return true;
    }
    Transition *pTransition = FindTransition(pMsg->mButton, true);
    if (pTransition != nullptr) {
        TheUI.GotoScreen(TheUI.FindScreen(pTransition->mScreen, false));
    }
    return false; // Yes, the binary reports a followed transition as unhandled.
}

bool UIScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return false;
    }
    Transition *pTransition = FindTransition(pMsg->mComponent, true);
    if (pTransition == nullptr) {
        return false;
    }
    TheUI.GotoScreen(TheUI.FindScreen(pTransition->mScreen, false));
    return true;
}

void UIScreen::Poll(float fTime) {
    bool bWasAnimating = false;
    bool bAnimating = false;
    for (const auto &entry : mPanels) {
        UIPanel *pPanel = entry.second;
        if (pPanel->mTransitionStart != 0.0f) {
            bWasAnimating = true;
        }
        pPanel->Poll(fTime);
        if (pPanel->mTransitionStart != 0.0f) {
            bAnimating = true;
        }
    }

    if (mNextScreen == nullptr && mPrevScreen == nullptr) {
        return;
    }
    if (bAnimating) {
        return;
    }
    if (mNextScreen != nullptr) {
        if (!mNextScreen->IsLoaded()) {
            return;
        }
        Unload();
        mNextScreen->Enter(this, fTime);
        mNextScreen = nullptr;
    } else if (!bWasAnimating) {
        UITransitionCompleteMsg msg(this, mPrevScreen);
        Dispatch(&msg);
        mPrevScreen = nullptr;
    }
}

void UIScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    mPrevScreen = pPrevScreen;
    mNextScreen = nullptr;
    TheUI.SetCurrentScreen(this);
    for (const auto &entry : mPanels) {
        entry.second->Enter(mForceEntryExit, fTime);
    }
    Poll(fTime);
}

void UIScreen::Draw() {
    for (const auto &entry : mPanels) {
        entry.second->Draw();
    }
}

void UIScreen::SetFocus(UIPanel *pPanel) {
    if (pPanel == mFocusPanel) {
        return;
    }
    if (mFocusPanel != nullptr) {
        mFocusPanel->Unfocus();
    }
    mFocusPanel = pPanel;
    if (pPanel != nullptr) {
        pPanel->Focus();
    }
}

UIComponent *UIScreen::FindComponent(const char *pszName) {
    UIComponent *pFound = nullptr;
    UIPanel *pFoundPanel = nullptr;
    for (const auto &entry : mPanels) {
        UIComponent *pComponent = entry.second->FindComponent(pszName, true);
        if (pComponent == nullptr) {
            continue;
        }
        if (pFound != nullptr) {
            const String report = String("FindComponent within screen \"") + mName +
                                  "\" failed because there are at least two\ncomponents with the "
                                  "name \"" +
                                  pszName + "\".\nThey are found on panels:\n   \"" +
                                  pFoundPanel->mName + "\"\n   \"" + entry.first + "\"";
            DebugWarn(report.c_str());
        }
        pFound = pComponent;
        pFoundPanel = entry.second;
    }
    return pFound;
}

void UIScreen::Exit(UIScreen *pNextScreen, float fTime) {
    mNextScreen = pNextScreen;
    mPrevScreen = nullptr;
    for (const auto &entry : mPanels) {
        UIPanel *pPanel = entry.second;
        bool bShared = false;
        for (const auto &next : pNextScreen->mPanels) {
            if (next.second == pPanel) {
                bShared = true;
                break;
            }
        }
        if (!bShared) {
            pPanel->Exit(mForceEntryExit, fTime);
        } else if (pPanel->mState == UIPanel::kStateExiting) {
            pPanel->Enter(mForceEntryExit, fTime);
        }
    }
}

UIScreen::Transition *UIScreen::FindTransition(UIComponent *pComponent,
                                               [[maybe_unused]] bool bFail) {
    for (auto &transition : mTransitions) {
        if (transition.mComponent == pComponent->mName) {
            return &transition;
        }
    }
    return nullptr;
}

UIScreen::Transition *UIScreen::FindTransition(int nButton, [[maybe_unused]] bool bFail) {
    for (auto &transition : mTransitions) {
        if (transition.mButton == nButton) {
            return &transition;
        }
    }
    return nullptr;
}

void UIScreen::Print(PrnStream &stream) {
    stream << "{UIScreen " << mName << "\n";
    if (!mPanels.empty()) {
        stream << "   Panels:\n";
        for (const auto &entry : mPanels) {
            stream << "      " << entry.first;
            if (entry.second == mFocusPanel) {
                stream << " (has focus)";
            }
            stream << "\n";
        }
    }
    if (!mTransitions.empty()) {
        stream << "   Transitions:\n";
        for (const auto &transition : mTransitions) {
            stream << "      ";
            if (transition.mComponent != nullptr) {
                stream << " component " << transition.mComponent;
            } else {
                stream << " joypad button " << transition.mButton;
            }
            stream << " -> " << transition.mScreen << "\n";
        }
    }
    if (mNextScreen != nullptr) {
        stream << "   Transition in progress ";
        if (mPrevScreen != nullptr) {
            stream << " from ";
        } else {
            stream << " to ";
        }
        stream << mNextScreen->mName << "\n";
    }
    stream << "}\n";
}

void UIScreen::Load() {
    for (const auto &entry : mPanels) {
        entry.second->Load();
    }
}

void UIScreen::Unload() {
    for (const auto &entry : mPanels) {
        entry.second->Unload();
    }
}

bool UIScreen::IsLoaded() {
    for (const auto &entry : mPanels) {
        if (!entry.second->IsLoaded()) {
            return false;
        }
    }
    return true;
}
