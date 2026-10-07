#include "met/freqscreen.h"

#include "game/gamedb.h"
#include "met/freqpanel.h"
#include "met/metagame.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "os/system.h"
#include "synth/fxmidi.h"
#include "ui/uicomponentselectmsg.h"

namespace {

// Index of the component name in a `shortcuts` entry.
constexpr int kShortcutComponentIndex = 1;

} // namespace

FreqScreen::FreqScreen(DataArray *pData)
    : UIScreen(pData), mNoGizmo(false), mNoProjectorSfx(false), mNeedsTransitionSfx(false) {
    pData->FindVector("gizmoOrig", &mGizmoOrig, false);
    pData->FindBool("no_projector_sfx", &mNoProjectorSfx, false);
    Vector3 rotation{0.0f, 0.0f, 0.0f};
    pData->FindVector("gizmoRot", &rotation, false);
    mGizmoRot = EulerAnglesToQuat(&rotation.x);
    pData->FindBool("needs_transition_sfx", &mNeedsTransitionSfx, false);
    pData->FindBool("no_gizmo", &mNoGizmo, false);
}

FreqScreen::~FreqScreen() {
}

void FreqScreen::Draw() {
    for (const auto &entry : mPanels) {
        entry.second->DrawGizmo();
    }
    UIScreen::Draw();
}

void FreqScreen::Exit(UIScreen *pNextScreen, float fTime) {
    FreqScreen *pNext = nullptr;
    if (pNextScreen != nullptr) {
        pNext = dynamic_cast<FreqScreen *>(pNextScreen);
    }
    if (pNext != nullptr) {
        bool bJump = mNoGizmo;
        if (TheMetagame.GetState() != Metagame::kStateFrontEnd) {
            bJump = true;
        }
        if (!bJump) {
            TheMetagame.mGizmo->MoveTo(pNext->mGizmoOrig, pNext->mGizmoRot, fTime);
        } else {
            TheMetagame.mGizmo->SetTo(pNext->mGizmoOrig, pNext->mGizmoRot);
        }
    }
    UIScreen::Exit(pNextScreen, fTime);
    if (mNeedsTransitionSfx) {
        FxMidi::StopTransition();
    }
}

void FreqScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    const int nState = TheMetagame.GetState();
    if (mNeedsTransitionSfx) {
        if (nState == Metagame::kStateFrontEnd) {
            FxMidi::PlayTransition();
        }
    } else if (!mNoProjectorSfx) {
        FxMidi::PlayProjector();
    }

    if (!TheMetagame.mGizmo->IsShowing()) {
        TheMetagame.mGizmo->SetTo(mGizmoOrig, mGizmoRot);
    }
    bool bHide = mNoGizmo;
    if (TheMetagame.GetState() != Metagame::kStateFrontEnd) {
        bHide = true;
    }
    TheMetagame.mGizmo->SetShowing(!bHide);

    for (const auto &entry : mPanels) {
        UIPanel *pPanel = entry.second;
        FreqPanel *pFreqPanel = nullptr;
        if (pPanel != nullptr) {
            pFreqPanel = dynamic_cast<FreqPanel *>(pPanel);
        }
        if (pFreqPanel != nullptr) {
            pFreqPanel->ShowGizmoMesh(!bHide);
        }
    }
    UIScreen::Enter(pPrevScreen, fTime);
}

const char *FreqScreen::Title() {
    return TheLocale.Localize(FormatString("%s_TITLE", mName), false);
}

bool FreqScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return UIScreen::DispatchPriv(pMsg);
}

void FreqScreen::PlayButtonSound(int nButton) {
    switch (nButton) {
    case kPadCross:
        FxMidi::PlayMenuSelect();
        break;
    case kPadDUp:
        FxMidi::PlayMenuUp();
        break;
    case kPadDRight:
        FxMidi::PlayMenuRight();
        break;
    case kPadDDown:
        FxMidi::PlayMenuDown();
        break;
    case kPadDLeft:
        FxMidi::PlayMenuLeft();
        break;
    default:
        break;
    }
}

bool FreqScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    PlayButtonSound(pMsg->mButton);
    return false;
}

bool FreqScreen::HandleTransitionComplete([[maybe_unused]] UITransitionCompleteMsg *pMsg) {
    DataArray *pShortcut = SystemConfig()
                               ->FindArray("metagame", false)
                               ->FindArray("shortcuts", false)
                               ->FindArray(mName, false);
    if (pShortcut != nullptr) {
        UIComponent *pComponent = FindComponent(pShortcut->Sym(kShortcutComponentIndex));
        UIComponentSelectMsg msg(pComponent, mFocusPanel, this, kPadCross);
        Dispatch(&msg);
    }
    return false;
}

bool FreqScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPad < TheGameDb->GetNumPads()) {
        return UIScreen::HandleJoypad(pMsg);
    }
    return true;
}
