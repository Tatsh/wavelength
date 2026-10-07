#include "ui/uipanel.h"

#include <cstring>

#include "os/joypad.h"
#include "os/locale.h"
#include "rnd/manager.h"
#include "ui/localizeerrors.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uimanager.h"
#include "ui/uiscreen.h"

namespace {

// The frame a panel reports while neither of its animations plays.
constexpr float kIdleFrame = 9999.999f;

// The frames of the entry and exit animation a panel starts with.
constexpr float kDefaultShownFrame = 0.0f;
constexpr float kDefaultHiddenFrame = 300.0f;
constexpr float kDefaultEnterMs = 150.0f;
constexpr float kDefaultExitMs = 150.0f;

// A start time of 0 means that no animation plays, so an animation that starts at time 0 starts
// here instead.
constexpr float kEarliestStart = 1.0e-6f;

// The load flags of a panel's file.
constexpr int kPanelLoadFlags = 5;

// The index of the first entry after the name in a panel's description.
constexpr int kFirstEntryIndex = 2;

// The index of a name in an entry.
constexpr int kEntryNameIndex = 1;

// The length of the `.txt` suffix a text object's name ends with.
constexpr int kTextSuffixLength = 4;

// Read the frames of a `panel_enter_exit` section.
inline void FindEnterExitFrames(DataArray *pFrames,
                                float *pfEnterStart,
                                float *pfEnterStop,
                                float *pfExitStart,
                                float *pfExitStop) {
    pFrames->FindFloat("enter_start_frame", pfEnterStart, false);
    pFrames->FindFloat("enter_stop_frame", pfEnterStop, false);
    pFrames->FindFloat("exit_start_frame", pfExitStart, false);
    pFrames->FindFloat("exit_stop_frame", pfExitStop, false);
}

} // namespace

float UIPanel::sEnterStartFrame = 0.0f;
float UIPanel::sEnterStopFrame = 150.0f;
float UIPanel::sExitStartFrame = 150.0f;
float UIPanel::sExitStopFrame = 300.0f;

void UIPanel::Init(DataArray *pConfig) {
    DataArray *pFrames = pConfig->FindArray("panel_enter_exit", false);
    if (pFrames != nullptr) {
        FindEnterExitFrames(
            pFrames, &sEnterStartFrame, &sEnterStopFrame, &sExitStartFrame, &sExitStopFrame);
    }
}

UIPanel::UIPanel(DataArray *pData, const char *pszDir)
    : mIdleFrame(kIdleFrame), mState(kStateHidden) {
    mHiddenFrame = kDefaultHiddenFrame;
    mExitMs = kDefaultExitMs;
    mView = nullptr;
    mFocusName = nullptr;
    mLoopStart = 0.0f;
    mLoopAnim = nullptr;
    mEnterExitAnim = nullptr;
    mTransitionStart = 0.0f;
    mShownFrame = kDefaultShownFrame;
    mEnterMs = kDefaultEnterMs;
    mLoadRefs = 0;
    mLoaded = false;
    mNavigator = nullptr;
    mFocus = nullptr;
    mLoader = nullptr;
    mName = pData->Sym(kEntryNameIndex);
    mFile.Printf("%s/%s.rnd", pszDir, mName);
    mData = pData;
    pData->AddRef();
}

void UIPanel::Load() {
    if (++mLoadRefs != 1) {
        return;
    }
    mState = kStateHidden;
    mView = nullptr;
    mFocus = nullptr;
    mNavigator = nullptr;
    if (mLoader == nullptr) {
        mLoader = Rnd::TheManager.AddLoader(mFile.c_str(), kPanelLoadFlags, nullptr, nullptr);
    }
}

void UIPanel::Unload() {
    if (--mLoadRefs != 0) {
        return;
    }
    if (mNavigator != nullptr) {
        delete mNavigator;
    }
    mNavigator = nullptr;
    if (mFocus != nullptr) {
        mFocusName = mFocus->mName;
    }
    for (const auto &entry : mComponents) {
        if (entry.second != nullptr) {
            delete entry.second;
        }
    }
    mComponents.clear();
    mFocus = nullptr;
    mTexts.clear();
    if (mLoader != nullptr) {
        delete mLoader;
    }
    mLoaded = false;
    mLoader = nullptr;
}

bool UIPanel::IsLoaded() {
    if (mLoaded) {
        return true;
    }
    if (mLoader == nullptr) {
        return false;
    }
    if (!mLoader->IsLoaded()) {
        return false;
    }
    FinishLoad();
    return true;
}

void UIPanel::FinishLoad() {
    LocalizeTexts(mFile.c_str(), mLoader->mObjects);
    mView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(FormatString("%s.view", mName)));
    mLoopAnim = dynamic_cast<Rnd::Animatable *>(
        Rnd::TheManager.Find(FormatString("%s_always.anim", mName)));
    mEnterExitAnim = dynamic_cast<Rnd::Animatable *>(
        Rnd::TheManager.Find(FormatString("%s_enterexit.anim", mName)));

    float fEnterStart = sEnterStartFrame;
    float fEnterStop = sEnterStopFrame;
    float fExitStart = sExitStartFrame;
    float fExitStop = sExitStopFrame;
    DataArray *pFrames = mData->FindArray("panel_enter_exit", false);
    if (pFrames != nullptr) {
        FindEnterExitFrames(pFrames, &fEnterStart, &fEnterStop, &fExitStart, &fExitStop);
    }
    mHiddenFrame = fExitStop;
    mShownFrame = fEnterStop;
    mEnterMs = __builtin_fabsf(fEnterStop - fEnterStart);
    mExitMs = __builtin_fabsf(fExitStop - fExitStart);
    mEnterReversed = fEnterStop < fEnterStart;
    mExitReversed = fExitStop < fExitStart;
    mEnterStartFrame = fEnterStart;
    mExitStartFrame = fExitStart;

    for (int i = kFirstEntryIndex; i < mData->Size(); ++i) {
        DataArray *pEntry = mData->Array(i);
        const char *pszType = pEntry->Sym(0);
        if (std::strstr(pszType, "comp") != nullptr) {
            AddComponent(TheUI.CreateComponent(pEntry, mName));
        } else if (std::strcmp("focus", pszType) == 0 && mFocusName == nullptr) {
            mFocus = FindComponent(pEntry->Sym(kEntryNameIndex), true);
        } else if (std::strcmp("navigator", pszType) == 0) {
            mNavigator = new UINavigator(pEntry, this);
        }
    }
    if (mFocusName != nullptr) {
        mFocus = FindComponent(mFocusName, true);
    }
    if (mFocus == nullptr && !mComponents.empty()) {
        mFocus = mComponents.begin()->second;
    }
    mLoaded = true;
}

UIPanel::~UIPanel() {
    mLoadRefs = 1;
    Unload();
    mData->Release();
}

void UIPanel::AddComponent(UIComponent *pComponent) {
    (void)FindComponent(pComponent->mName, true); // Yes, the binary discards the lookup.
    mComponents[pComponent->mName] = pComponent;
}

void UIPanel::SetTextsShowing(bool bShowing) {
    for (Rnd::Text *pText : mTexts) {
        pText->SetShowing(bShowing);
    }
}

void UIPanel::Enter(bool bForce, float fTime) {
    SetTextsShowing(true);
    if (mNavigator != nullptr) {
        mNavigator->mEnabled = true;
    }
    if (bForce) {
        if (mEnterExitAnim != nullptr) {
            mEnterExitAnim->SetFrame(mShownFrame);
        }
        UIComponent *pFocus = mFocus;
        mState = kStateShown;
        mFocus = nullptr;
        SetFocus(pFocus, kPadNone);
        return;
    }
    if (mState == kStateExiting) {
        mTransitionStart = fTime + mEnterMs * ((fTime - mTransitionStart) / mExitMs - 1.0f);
        if (mTransitionStart == 0.0f) {
            mTransitionStart = kEarliestStart;
        }
        mState = kStateEntering;
    } else if (mState == kStateHidden) {
        if (fTime == 0.0f) {
            mTransitionStart = kEarliestStart;
        } else {
            mTransitionStart = fTime;
        }
        mState = kStateEntering;
        UIComponent *pFocus = mFocus;
        mFocus = nullptr;
        SetFocus(pFocus, kPadNone);
    }
}

void UIPanel::Exit(bool bForce, float fTime) {
    if (bForce) {
        if (mEnterExitAnim != nullptr) {
            mEnterExitAnim->SetFrame(mHiddenFrame);
        }
        mState = kStateHidden;
        SetTextsShowing(false);
        return;
    }
    if (mState == kStateEntering) {
        mState = kStateExiting;
        mTransitionStart = fTime + mExitMs * ((fTime - mTransitionStart) / mEnterMs - 1.0f);
    } else if (mState == kStateShown) {
        mState = kStateExiting;
        mTransitionStart = fTime;
    }
}

bool UIPanel::Dispatch(Message *pMsg) {
    const bool bNavigation = TheUI.IsNavigationMsg(pMsg);
    if (!bNavigation && TheUI.mCurrentScreen != nullptr && TheUI.mCurrentScreen->Dispatch(pMsg)) {
        return true;
    }
    if (DispatchPriv(pMsg)) {
        return true;
    }
    if (mNavigator != nullptr && mNavigator->Dispatch(pMsg)) {
        return true;
    }
    if (!bNavigation) {
        return false;
    }
    return mFocus != nullptr && mFocus->Dispatch(pMsg);
}

bool UIPanel::DispatchPriv(Message *pMsg) {
    (void)pMsg->Type(); // Yes, the binary discards the type.
    return false;
}

float UIPanel::ComputeFrame(float fTime) {
    if (mTransitionStart == 0.0f) {
        return mIdleFrame;
    }
    const float fElapsed = fTime - mTransitionStart;
    if (mState == kStateEntering) {
        if (mEnterMs < fElapsed) {
            mTransitionStart = 0.0f;
            mState = kStateShown;
            return mShownFrame;
        }
        if (mEnterReversed) {
            return mEnterStartFrame - fElapsed;
        }
        return fElapsed + mEnterStartFrame;
    }
    if (mExitMs < fElapsed) {
        mTransitionStart = 0.0f;
        mState = kStateHidden;
        const float fFrame = mHiddenFrame;
        SetTextsShowing(false);
        return fFrame;
    }
    if (mExitReversed) {
        return mExitStartFrame - fElapsed;
    }
    return fElapsed + mExitStartFrame;
}

void UIPanel::Poll(float fTime) {
    if (!mLoaded) {
        return;
    }
    if (mLoopAnim != nullptr) {
        mLoopAnim->SetFrame(fTime - mLoopStart);
    }
    const float fFrame = ComputeFrame(fTime);
    mFrame = fFrame;
    if (mEnterExitAnim != nullptr && fFrame != mIdleFrame) {
        mEnterExitAnim->SetFrame(fFrame);
    }
    for (const auto &entry : mComponents) {
        entry.second->Poll(fTime);
    }
    mView->UpdateWorldXfm(nullptr, 0);
}

void UIPanel::Draw() {
    if (mLoaded && mState != kStateHidden) {
        mView->Draw();
    }
}

void UIPanel::SetShowing(bool bShowing) {
    mView->SetShowing(bShowing);
}

void UIPanel::Focus() {
    if (mNavigator != nullptr) {
        mNavigator->mEnabled = true;
    }
}

void UIPanel::SetFocus(UIComponent *pComponent, [[maybe_unused]] int nButton) {
    if (pComponent == mFocus) {
        return;
    }
    UIComponentFocusChangeMsg msg(pComponent, mFocus, this, TheUI.mCurrentScreen);
    if (Dispatch(&msg)) {
        return;
    }
    if (mFocus != nullptr) {
        mFocus->Unfocus();
    }
    mFocus = pComponent;
    if (pComponent != nullptr) {
        pComponent->Focus();
    }
}

UIComponent *UIPanel::FindComponent(const char *pszName, [[maybe_unused]] bool bFail) {
    const auto it = mComponents.find(pszName);
    if (it == mComponents.end()) {
        return nullptr;
    }
    return it->second;
}

void UIPanel::LocalizeTexts(const char *pszFile, std::list<Rnd::Object *> &objects) {
    LocalizeErrors errors(pszFile);
    for (Rnd::Object *pObject : objects) {
        Rnd::Text *pText = pObject != nullptr ? dynamic_cast<Rnd::Text *>(pObject) : nullptr;
        if (pText == nullptr) {
            continue;
        }
        mTexts.push_back(pText);
        pText->SetShowing(false);
        String token(pText->mName.mStr);
        if (token.Compare(token.mLength - kTextSuffixLength, kTextSuffixLength, ".txt") == 0) {
            token.Erase(token.mLength - kTextSuffixLength, kTextSuffixLength);
        }
        const char *pszLocalized = TheLocale.Localize(token.c_str(), false);
        if (TheUI.mEditMode) {
            LocalizeErrors::CountToken(token.c_str(), 1);
        }
        if (pszLocalized != token.c_str()) {
            pText->SetText(pszLocalized);
        } else if (TheUI.mEditMode) {
            errors.mErrors.push_back(LocalizeErrors::SingleError{String(pText->mName.mStr),
                                                                 String(pText->mPreWrapText.mStr)});
        }
        pText->UpdateCursors();
    }
    if (errors.mErrors.size() != 0) {
        errors.Store();
    }
}
