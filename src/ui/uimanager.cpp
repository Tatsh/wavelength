#include "ui/uimanager.h"

#include <cstring>

#include "msg/keyboardkeymsg.h"
#include "os/debug.h"
#include "os/fileutil.h"
#include "os/joypad.h"
#include "os/system.h"
#include "rnd/manager.h"
#include "script/symbol.h"
#include "ui/lrbutton.h"
#include "ui/uibutton.h"
#include "ui/uilabel.h"
#include "ui/uilist.h"
#include "ui/uiscreenchangemsg.h"
#include "ui/uitextentry.h"

namespace {

// Index of the name of a description entry, and of the type that starts it.
constexpr int kTypeIndex = 0;
constexpr int kNameIndex = 1;

// The value of mRepeatPad while no button repeats.
constexpr int kNoRepeatPad = -1;

// The time LoadFile() lets the renderer spend on a screen it loads at once.
constexpr float kLoadAllBudgetMs = 1000000.0f;

// The load flags of a shared `.rnd` file, which loads before the call returns.
constexpr int kSharedLoadFlags = 0;

} // namespace

// The unit's static initialiser at NTSC-U/C: 0x0020e580, PAL: 0x00217398, its global
// constructor at NTSC-U/C: 0x0020e5c0, PAL: 0x002173d8, and its global destructor at
// NTSC-U/C: 0x0020e5e0, PAL: 0x002173f8, construct and destroy it.
UIManager TheUI;

float UIManager::sFirstRepeatDelay = 250.0f;
float UIManager::sRepeatDelay = 10.0f;

UIManager::Callback::Callback(DataArray *pConfig) {
    mAllowed = pConfig->FindArray("allowed_rnd_merges", true);
}

int UIManager::Callback::ShouldLoad(Rnd::Object *pExisting,
                                    const char *pszName,
                                    [[maybe_unused]] const char *pszClass) {
    const bool bMissing = pExisting == nullptr;
    if (!bMissing) {
        int i = 1;
        while (i < mAllowed->Size() && std::strcmp(pszName, mAllowed->Sym(i)) != 0) {
            ++i;
        }
        if (i == mAllowed->Size()) {
            mRejected.Printf("  %s\n", pszName);
        }
    }
    return bMissing;
}

void UIManager::Callback::Report() {
    if (mRejected.mLength != 0) {
        DebugNotify(
            "These objects won't be loaded because they\nalready exist, and are not\nin the "
            "allowed list:\n%s",
            mRejected.c_str());
    }
}

UIManager::UIManager() : mTime(1.0f), mEditMode(false), mRepeatPad(kNoRepeatPad) {
    mRepeatPending = true;
    mRepeatTime = 0.0f;
}

UIManager::~UIManager() {
}

void UIManager::Init() {
    mEditMode = false;
    mCurrentScreen = nullptr;
    mTime = 1.0f;
    DataArray *pConfig = SystemConfig()->FindArray("ui", true);
    mAllowableGroups = pConfig->FindArray("allowable_groups", true);

    DataArray *pRepeat = pConfig->FindArray("nav_repeat", false);
    if (pRepeat != nullptr) {
        pRepeat->FindFloat("max_delay_frames", &sFirstRepeatDelay, false);
        pRepeat->FindFloat("min_delay_frames", &sRepeatDelay, false);
    }

    int nEditMode = 0;
    pConfig->FindInt("editmode", &nEditMode, false);
    mEditMode = nEditMode != 0;

    DataArray *pShared = pConfig->FindArray("shared_rnd_files", false);
    if (pShared != nullptr) {
        mCallback = new Callback(pConfig);
        for (int i = 1; i < pShared->Size(); ++i) {
            RndLoader *pLoader =
                Rnd::TheManager.AddLoader(pShared->Sym(i), kSharedLoadFlags, mCallback, nullptr);
            for (Rnd::Object *pObject : pLoader->mObjects) {
                mSharedObjects.push_back(pObject);
            }
            delete pLoader;
        }
        mCallback->Report();
        delete mCallback;
        mCallback = nullptr;
    }

    UIPanel::Init(pConfig);
    UIComponent::Init(pConfig);
    RegisterPanelType(UIPanel::New, "panel");
    RegisterScreenType(UIScreen::New, "screen");
    RegisterStyleType(UIStyle::New, "style");
    RegisterComponentType(UIButton::New, "button_comp");
    RegisterComponentType(UILabel::New, "label_comp");
    RegisterComponentType(LRButton::New, "lrbutton_comp");
    RegisterComponentType(UIList::New, "list_comp");
    RegisterComponentType(UITextEntry::New, "text_entry_comp");
}

void UIManager::Terminate() {
    for (const auto &entry : mStyles) {
        delete entry.second;
    }
    mStyles.clear();
    for (const auto &entry : mScreens) {
        delete entry.second;
    }
    mScreens.clear();
    for (const auto &entry : mPanels) {
        delete entry.second;
    }
    mPanels.clear();
    while (!mSharedObjects.empty()) {
        delete mSharedObjects.front();
        mSharedObjects.pop_front();
    }
}

void UIManager::AddScreen(UIScreen *pScreen) {
    (void)FindScreen(pScreen->mName, true); // Yes, the binary discards this lookup.
    mScreens[pScreen->mName] = pScreen;
}

void UIManager::AddPanel(UIPanel *pPanel) {
    (void)FindPanel(pPanel->mName, true); // Yes, the binary discards this lookup.
    mPanels[pPanel->mName] = pPanel;
}

void UIManager::AddStyle(UIStyle *pStyle) {
    (void)FindStyle(pStyle->mName, true); // Yes, the binary discards this lookup.
    mStyles[pStyle->mName] = pStyle;
}

bool UIManager::IsNavigationMsg(Message *pMsg) {
    return pMsg->Type() == g_nJoypadInputMsgType || pMsg->Type() == g_nKeyboardKeyMsgType;
}

bool UIManager::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return false;
}

bool UIManager::HandleJoypad(JoypadInputMsg *pMsg) {
    const int nButton = pMsg->mButton;
    if (nButton != kPadDUp && nButton != kPadDRight && nButton != kPadDDown &&
        nButton != kPadDLeft) {
        return false;
    }
    if (pMsg->mPressed == 0) {
        if (mRepeatTime != 0.0f && mRepeatButton == pMsg->mButton && mRepeatPad == pMsg->mPad) {
            mRepeatTime = 0.0f;
            mRepeatPad = kNoRepeatPad;
            mRepeatCount = 0;
        }
    } else if (pMsg->mPad == mRepeatPad || mRepeatPad == kNoRepeatPad) {
        mRepeatButton = pMsg->mButton;
        mRepeatPending = true;
        mRepeatPad = pMsg->mPad;
        mRepeatTime = RepeatDelay(mRepeatCount);
        ++mRepeatCount;
    }
    return false;
}

bool UIManager::Dispatch(Message *pMsg) {
    const bool bNavigation = TheUI.IsNavigationMsg(pMsg);
    if (!bNavigation && SendUntilHandled(pMsg)) {
        return true;
    }
    if (DispatchPriv(pMsg)) {
        return true;
    }
    if (!bNavigation) {
        return false;
    }
    return mCurrentScreen->Dispatch(pMsg);
}

void UIManager::Poll(float fTime) {
    mTime = fTime;
    if (mCurrentScreen != nullptr) {
        mCurrentScreen->Poll(fTime);
    }
    if (mRepeatTime == 0.0f) {
        return;
    }
    if (mRepeatPad == kNoRepeatPad) {
        mRepeatTime = 0.0f;
        return;
    }
    if (mRepeatPending) {
        const float fNextRepeat = fTime + mRepeatTime;
        mRepeatPending = false;
        mRepeatTime = fNextRepeat;
        return;
    }
    if (mRepeatTime < fTime) {
        ++mRepeatCount;
        JoypadInputMsg msg(mRepeatPad, mRepeatButton, 1);
        Dispatch(&msg);
    }
}

float UIManager::RepeatDelay(int nCount) {
    if (nCount == 0) {
        return sFirstRepeatDelay;
    }
    return sRepeatDelay;
}

void UIManager::Draw() {
    if (mCurrentScreen != nullptr) {
        mCurrentScreen->Draw();
    }
}

UIScreen *UIManager::FindScreen(const char *pszName, [[maybe_unused]] bool bFail) {
    const auto it = mScreens.find(pszName);
    return it == mScreens.end() ? nullptr : it->second;
}

UIPanel *UIManager::FindPanel(const char *pszName, [[maybe_unused]] bool bFail) {
    const auto it = mPanels.find(pszName);
    return it == mPanels.end() ? nullptr : it->second;
}

UIComponent *UIManager::FindComponent(const char *pszPanel, const char *pszComponent, bool bFail) {
    UIPanel *pPanel = FindPanel(pszPanel, bFail);
    if (pPanel == nullptr) {
        return nullptr;
    }
    return pPanel->FindComponent(pszComponent, bFail);
}

UIStyle *UIManager::FindStyle(const char *pszName, [[maybe_unused]] bool bFail) {
    const auto it = mStyles.find(pszName);
    return it == mStyles.end() ? nullptr : it->second;
}

void UIManager::GotoScreen(const char *pszName) {
    GotoScreen(TheUI.FindScreen(pszName, false));
}

void UIManager::GotoScreen(UIScreen *pScreen) {
    if (mCurrentScreen != nullptr) {
        if (mCurrentScreen == pScreen || mCurrentScreen->mNextScreen == pScreen) {
            return;
        }
        UIScreenChangeMsg msg(pScreen, mCurrentScreen);
        if (Dispatch(&msg)) {
            return;
        }
    }
    pScreen->Load();
    if (mCurrentScreen != nullptr) {
        mCurrentScreen->Exit(pScreen, mTime);
    } else {
        pScreen->Enter(nullptr, mTime);
    }
}

void UIManager::SetCurrentScreen(UIScreen *pScreen) {
    mCurrentScreen = pScreen;
}

UIPanel *UIManager::FocusPanel() {
    return mCurrentScreen->mFocusPanel;
}

void UIManager::LoadGroup(const char *pszGroup) {
    const char *pszSymbol = LookupSymbol(pszGroup);
    for (const auto &entry : mScreens) {
        if (entry.second->mGroup == pszSymbol) {
            entry.second->Load();
        }
    }
}

void UIManager::UnloadGroup(const char *pszGroup) {
    const char *pszSymbol = LookupSymbol(pszGroup);
    for (const auto &entry : mScreens) {
        if (entry.second->mGroup == pszSymbol) {
            entry.second->Unload();
        }
    }
}

bool UIManager::IsGroupLoaded(const char *pszGroup) {
    const char *pszSymbol = LookupSymbol(pszGroup);
    for (const auto &entry : mScreens) {
        if (entry.second->mGroup == pszSymbol && !entry.second->IsLoaded()) {
            return false;
        }
    }
    return true;
}

void UIManager::RegisterPanelType(PanelFactory pfnFactory, const char *pszType) {
    mPanelFactories[String(pszType)] = pfnFactory;
}

void UIManager::RegisterScreenType(ScreenFactory pfnFactory, const char *pszType) {
    mScreenFactories[String(pszType)] = pfnFactory;
}

void UIManager::RegisterStyleType(StyleFactory pfnFactory, const char *pszType) {
    mStyleFactories[String(pszType)] = pfnFactory;
}

void UIManager::RegisterComponentType(ComponentFactory pfnFactory, const char *pszType) {
    mComponentFactories[String(pszType)] = pfnFactory;
}

UIPanel *UIManager::CreatePanel(DataArray *pData, const char *pszDir) {
    const auto it = mPanelFactories.find(String(pData->Sym(kTypeIndex)));
    return it->second(pData, pszDir);
}

UIScreen *UIManager::CreateScreen(DataArray *pData) {
    const auto it = mScreenFactories.find(String(pData->Sym(kTypeIndex)));
    return it->second(pData);
}

UIStyle *UIManager::CreateStyle(DataArray *pData) {
    const auto it = mStyleFactories.find(String(pData->Sym(kTypeIndex)));
    return it->second(pData);
}

UIComponent *UIManager::CreateComponent(DataArray *pData, const char *pszDir) {
    const auto it = mComponentFactories.find(String(pData->Sym(kTypeIndex)));
    return it->second(pData, pszDir);
}

void UIManager::LoadFile(const char *pszFile, bool bLoadAll) {
    if (pszFile == nullptr) {
        return;
    }

    const bool bUsingCD = UsingCD();
    if (g_bHostConfig) {
        SetUsingCD(false);
    }
    if (bUsingCD && g_bHostConfig) {
        DebugPrint("WARNING: reading UI files from host0\n");
    }
    DataArray *pFile = DataArray::Read(pszFile, nullptr);
    SetUsingCD(bUsingCD);

    for (int i = 0; i < pFile->Size(); ++i) {
        DataArray *pEntry = pFile->Array(i);
        const char *pszType = pEntry->Sym(kTypeIndex);
        if (std::strstr(pszType, "style") != nullptr) {
            AddStyle(CreateStyle(pEntry));
        } else if (std::strstr(pszType, "screen") != nullptr) {
            AddScreen(CreateScreen(pEntry));
        } else if (std::strstr(pszType, "panel") != nullptr) {
            AddPanel(CreatePanel(pEntry, FileGetPath(pEntry->mFile)));
        } else if (std::strcmp("focus", pszType) == 0) {
            GotoScreen(FindScreen(pEntry->Sym(kNameIndex), false));
        }
    }
    pFile->Release();

    if (!bLoadAll) {
        return;
    }
    for (const auto &entry : mScreens) {
        entry.second->Load();
        Rnd::TheManager.PollLoaders(kLoadAllBudgetMs);
        (void)entry.second->IsLoaded(); // Yes, the binary discards the result.
    }
}

void UIManager::VerifyGroup(const char **ppszGroup) {
    if (*ppszGroup == nullptr) {
        *ppszGroup = mAllowableGroups->Sym(1);
        return;
    }
    for (int i = 1; i < mAllowableGroups->Size(); ++i) {
        if (mAllowableGroups->Sym(i) == *ppszGroup) {
            return;
        }
    }
    DebugWarn("Couldn't verify group %s", *ppszGroup);
}
