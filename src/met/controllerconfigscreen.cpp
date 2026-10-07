#include "met/controllerconfigscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "game/playerprofile.h"
#include "met/controllerpanel.h"
#include "met/metagame.h"
#include "met/savefreqscreen.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "os/system.h"
#include "ui/uibutton.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"
#include "ui/uipanel.h"

namespace {

constexpr char kMetagameTag[] = "metagame";
constexpr char kChoiceListsTag[] = "choice_lists";
constexpr char kButtonEventsTag[] = "button_events";
constexpr char kStickEventsTag[] = "stick_events";

constexpr char kPanel[] = "o_controller";
constexpr char kMapPanel[] = "o_controller_map";
constexpr char kFirstComponent[] = "left_note_1";
constexpr char kSaveComponent[] = "save";
constexpr char kDefaultComponent[] = "default";
constexpr char kSaveScreen[] = "config_save_freq";
constexpr char kHelpToken[] = "o_controller_toggle_HELP";
constexpr char kButtonStyle[] = "controller_style";
constexpr char kStickStyle[] = "buttonStyle4";

// The label of an event that shares its choice with the focused event.
constexpr char kConflictLabel[] = "?";
constexpr char kNoLabel[] = "";

constexpr char kNoEventWarning[] = " Couldn't find event for component: %s\n";
constexpr char kNoSlotWarning[] = " Couldn't find button event: %s\n";
constexpr char kNoChoiceWarning[] = " Couldn't find choice: %d  in array %s\n";

constexpr char kLeftStickToken[] = "left_stick";
constexpr char kRightStickToken[] = "right_stick";

// The letters of the icons of the shoulder and face buttons, in the order of JoypadButton.
constexpr const char *kButtonLabels[] = {"a", "b", "c", "d", "e", "f", "g", "h"};

constexpr int kLeftStick = 0;
constexpr int kRightStick = 1;

// An event entry gives its name, its InputMap action, and the name of its choice list.
constexpr int kEventNameNode = 0;
constexpr int kEventActionNode = 1;
constexpr int kEventChoiceListNode = 2;

// A choice list gives its name and its kind before the choices.
constexpr int kFirstChoiceNode = 2;

// The column ChoiceColumn() reports when the choice list does not hold the value.
constexpr int kNoColumn = 0;

} // namespace

ControllerConfigScreen::ControllerConfigScreen(DataArray *pData)
    : FreqScreen(pData), mReturnScreen(nullptr), mActive(0), mButtonChoices(), mStickChoices() {
}

bool ControllerConfigScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return HandleFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void ControllerConfigScreen::Exit(UIScreen *pNextScreen, float fTime) {
    mActive = 0;
    FreqScreen::Exit(pNextScreen, fTime);
}

void ControllerConfigScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    mReturnScreen = pPrevScreen;
    DataArray *pConfig = SystemConfig()->FindArray(kMetagameTag, true);
    mChoiceLists = pConfig->FindArray(kChoiceListsTag, true);
    mButtonEvents = pConfig->FindArray(kButtonEventsTag, true);
    mStickEvents = pConfig->FindArray(kStickEventsTag, true);
    mButtonChoices.resize(mButtonEvents->Size() - 1, 0);
    mStickChoices.resize(mStickEvents->Size() - 1, 0);
    mFocusName = TheUI.FindPanel(kPanel, false)->FindComponent(kFirstComponent, false)->mName;
    LoadBindings(TheGameDb->GetProfile(0)->GetInputMap());
    mActive = 1;

    const char *pszFocus = TheUI.FindPanel(kPanel, false)->mFocus->mName;
    if (strcmp(pszFocus, kSaveComponent) != 0 && strcmp(pszFocus, kDefaultComponent) != 0) {
        ShowOnMap(pszFocus);
    }
}

void ControllerConfigScreen::LoadBindings(InputMap *pMap) {
    for (int nButton = 0; nButton < InputMap::kButtonCount; ++nButton) {
        DataArray *pEvent = FindButtonEvent(pMap->GetButtonAction(nButton), nButton);
        if (pEvent == nullptr) {
            continue;
        }
        ApplyLabel(EventName(pEvent), ButtonLabel(nButton), true);
        mButtonChoices[EventSlot(pEvent)] = ChoiceColumn(pEvent, nButton);
    }
    for (int nStick = 0; nStick < InputMap::kStickCount; ++nStick) {
        DataArray *pEvent = FindStickEvent(pMap->GetStickAction(nStick), nStick);
        if (pEvent == nullptr) {
            continue;
        }
        ApplyLabel(EventName(pEvent), StickLabel(nStick), false);
        mStickChoices[EventSlot(pEvent)] = ChoiceColumn(pEvent, nStick);
    }
}

void ControllerConfigScreen::SaveBindings() {
    InputMap map(*TheGameDb->GetProfile(0)->GetInputMap());
    for (int i = 1; i < mChoiceLists->Size(); ++i) {
        DataArray *pList = mChoiceLists->Array(i);
        for (int j = kFirstChoiceNode; j < pList->Size(); ++j) {
            map.SetButtonAction(pList->Int(j), 0);
        }
    }
    for (int i = 1; i < mButtonEvents->Size(); ++i) {
        DataArray *pEvent = mButtonEvents->Array(i);
        const int nButton = ChoiceList(pEvent)->Int(mButtonChoices[i - 1]);
        map.SetButtonAction(nButton, EventAction(pEvent));
    }
    for (int i = 1; i < mStickEvents->Size(); ++i) {
        DataArray *pEvent = mStickEvents->Array(i);
        const int nStick = ChoiceList(pEvent)->Int(mStickChoices[i - 1]);
        map.SetStickAction(nStick, EventAction(pEvent));
    }
    TheGameDb->GetProfile(0)->SetInputMap(&map);
}

DataArray *ControllerConfigScreen::FindButtonEvent(int nAction, int nButton) {
    for (int i = 1; i < mButtonEvents->Size(); ++i) {
        DataArray *pEvent = mButtonEvents->Array(i);
        DataArray *pList = ChoiceList(pEvent);
        int j = kFirstChoiceNode;
        while (j < pList->Size() && pList->Int(j) != nButton) {
            ++j;
        }
        const bool bListed = j < pList->Size();
        if (EventAction(pEvent) == nAction && bListed) {
            return pEvent;
        }
    }
    return nullptr;
}

DataArray *ControllerConfigScreen::FindStickEvent(int nAction, [[maybe_unused]] int nStick) {
    for (int i = 1; i < mStickEvents->Size(); ++i) {
        DataArray *pEvent = mStickEvents->Array(i);
        if (EventAction(pEvent) == nAction) {
            return pEvent;
        }
    }
    return nullptr;
}

DataArray *ControllerConfigScreen::FindEvent(const char *pszName, int *pnButton) {
    for (int i = 1; i < mButtonEvents->Size(); ++i) {
        if (pszName == EventName(mButtonEvents->Array(i))) {
            *pnButton = 1;
            return mButtonEvents->Array(i);
        }
    }
    for (int i = 1; i < mStickEvents->Size(); ++i) {
        if (pszName == EventName(mStickEvents->Array(i))) {
            *pnButton = 0;
            return mStickEvents->Array(i);
        }
    }
    DebugWarn(kNoEventWarning, pszName);
    return nullptr;
}

const char *ControllerConfigScreen::ButtonLabel(int nButton) const {
    if (static_cast<unsigned int>(nButton) < sizeof(kButtonLabels) / sizeof(kButtonLabels[0])) {
        return kButtonLabels[nButton];
    }
    return kNoLabel;
}

const char *ControllerConfigScreen::StickLabel(int nStick) const {
    if (nStick == kLeftStick) {
        return TheLocale.Localize(kLeftStickToken, true);
    }
    if (nStick == kRightStick) {
        return TheLocale.Localize(kRightStickToken, true);
    }
    return kNoLabel;
}

void ControllerConfigScreen::ApplyLabel(const char *pszComponent,
                                        const char *pszLabel,
                                        bool bButton) {
    UIButton *pButton = dynamic_cast<UIButton *>(TheUI.FindComponent(kPanel, pszComponent, false));
    if (bButton) {
        pButton->SetStyle(TheUI.FindStyle(kButtonStyle, false));
    } else {
        pButton->SetStyle(TheUI.FindStyle(kStickStyle, false));
    }
    pButton->SetText(pszLabel);
}

int ControllerConfigScreen::EventSlot(DataArray *pEvent) {
    const char *pszName = EventName(pEvent);
    for (int i = 1; i < mButtonEvents->Size(); ++i) {
        if (EventName(mButtonEvents->Array(i)) == pszName) {
            return i - 1;
        }
    }
    for (int i = 1; i < mStickEvents->Size(); ++i) {
        if (EventName(mStickEvents->Array(i)) == pszName) {
            return i - 1;
        }
    }
    DebugWarn(kNoSlotWarning, EventName(pEvent));
    return 0;
}

const char *ControllerConfigScreen::EventName(DataArray *pEvent) const {
    return pEvent->Sym(kEventNameNode);
}

int ControllerConfigScreen::EventAction(DataArray *pEvent) const {
    return pEvent->Int(kEventActionNode);
}

DataArray *ControllerConfigScreen::ChoiceList(DataArray *pEvent) {
    return mChoiceLists->FindArray(pEvent->Sym(kEventChoiceListNode), true);
}

int ControllerConfigScreen::ChoiceColumn(DataArray *pEvent, int nValue) {
    DataArray *pList = mChoiceLists->FindArray(pEvent->Sym(kEventChoiceListNode), true);
    for (int j = kFirstChoiceNode; j < pList->Size(); ++j) {
        if (pList->Int(j) == nValue) {
            return j;
        }
    }
    DebugWarn(kNoChoiceWarning, nValue, pEvent->Sym(kEventChoiceListNode));
    return kNoColumn;
}

bool ControllerConfigScreen::Validate() {
    int nFocusIsButton;
    DataArray *pFocus = FindEvent(mFocusName, &nFocusIsButton);
    const int nFocusSlot = EventSlot(pFocus);
    bool bValid = true;

    for (int i = 1; i < mButtonEvents->Size(); ++i) {
        DataArray *pEvent = mButtonEvents->Array(i);
        const char *pszName = EventName(pEvent);
        if (nFocusIsButton) {
            const int nFocusChoice = ChoiceList(pFocus)->Int(mButtonChoices[nFocusSlot]);
            if (nFocusChoice == ChoiceList(pEvent)->Int(mButtonChoices[i - 1]) &&
                pEvent != pFocus) {
                ApplyLabel(pszName, kConflictLabel, false);
            }
        }
        if (strcmp(TheUI.FindComponent(kPanel, pszName, false)->Text(), kConflictLabel) == 0) {
            bValid = false;
        }
    }

    for (int i = 1; i < mStickEvents->Size(); ++i) {
        DataArray *pEvent = mStickEvents->Array(i);
        const char *pszName = EventName(pEvent);
        if (!nFocusIsButton) {
            const int nFocusChoice = ChoiceList(pFocus)->Int(mStickChoices[nFocusSlot]);
            if (nFocusChoice == ChoiceList(pEvent)->Int(mStickChoices[i - 1]) && pEvent != pFocus) {
                ApplyLabel(pszName, kConflictLabel, false);
            }
        }
        if (strcmp(TheUI.FindComponent(kPanel, pszName, false)->Text(), kConflictLabel) == 0) {
            bValid = false;
        }
    }
    return bValid;
}

void ControllerConfigScreen::ResetBindings() {
    InputMap map;
    map.LoadDefaults();
    LoadBindings(&map);
}

void ControllerConfigScreen::CycleChoice(const char *pszComponent, int nButton) {
    mFocusName = pszComponent;
    int nIsButton;
    DataArray *pEvent = FindEvent(pszComponent, &nIsButton);
    const int nSlot = EventSlot(pEvent);
    DataArray *pList = ChoiceList(pEvent);
    std::vector<int> &choices = nIsButton ? mButtonChoices : mStickChoices;

    int nChoice = choices[nSlot];
    if (nButton == kPadDLeft) {
        nChoice = nChoice - 1 > kFirstChoiceNode - 1 ? nChoice - 1 : pList->Size() - 1;
    } else if (nButton == kPadDRight) {
        nChoice = nChoice + 1 < pList->Size() ? nChoice + 1 : kFirstChoiceNode;
    }

    const char *pszLabel;
    if (nIsButton) {
        pszLabel = ButtonLabel(pList->Int(nChoice));
    } else {
        pszLabel = StickLabel(pList->Int(nChoice));
    }
    choices[nSlot] = nChoice;
    ApplyLabel(pszComponent, pszLabel, nIsButton != 0);
    ShowOnMap(pszComponent);
}

void ControllerConfigScreen::ShowOnMap(const char *pszComponent) {
    if (!mActive) {
        return;
    }

    int nIsButton;
    DataArray *pEvent = FindEvent(pszComponent, &nIsButton);
    UIButton *pButton = dynamic_cast<UIButton *>(TheUI.FindComponent(kPanel, pszComponent, false));
    int nRegion = kPadNone;
    if (strcmp(pButton->Text(), kConflictLabel) != 0) {
        const int nSlot = EventSlot(pEvent);
        if (nIsButton) {
            nRegion = ChoiceList(pEvent)->Int(mButtonChoices[nSlot]);
        } else {
            const int nStick = ChoiceList(pEvent)->Int(mStickChoices[nSlot]);
            nRegion = nStick == kLeftStick ? kPadL3 : kPadR3;
        }
    }
    dynamic_cast<ControllerPanel *>(TheUI.FindPanel(kMapPanel, false))->SetRegion(nRegion);
}

bool ControllerConfigScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    const char *pszName = pMsg->mComponent->mName;
    if (strcmp(pszName, kDefaultComponent) == 0) {
        if (pMsg->mButton == kPadCross) {
            ResetBindings();
        }
    } else if (strcmp(pszName, kSaveComponent) == 0) {
        if (Validate()) {
            SaveBindings();
            if (TheMetagame.GetState() == Metagame::kStateFrontEnd) {
                SaveFreqScreen *pSave =
                    dynamic_cast<SaveFreqScreen *>(TheUI.FindScreen(kSaveScreen, false));
                pSave->SetDoneScreen(mReturnScreen->mName);
                pSave->mOverwriteStatus = 1;
                TheUI.GotoScreen(pSave);
            } else if (mReturnScreen != nullptr) {
                TheUI.GotoScreen(mReturnScreen);
            }
        }
    } else {
        if (pMsg->mButton == kPadDLeft || pMsg->mButton == kPadDRight) {
            CycleChoice(pszName, pMsg->mButton);
        }
        if (pMsg->mButton == kPadCross && Validate()) {
            UIPanel *pPanel = TheUI.FindPanel(kPanel, false);
            pPanel->SetFocus(pPanel->FindComponent(kSaveComponent, false), kPadNone);
        }
    }
    return FreqScreen::HandleSelectStart(pMsg);
}

bool ControllerConfigScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->mButton == kPadDUp || pMsg->mButton == kPadDDown) {
            Validate();
        } else if (pMsg->mButton == kPadTriangle && mReturnScreen != nullptr) {
            TheUI.GotoScreen(mReturnScreen);
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool ControllerConfigScreen::HandleFocusChange(UIComponentFocusChangeMsg *pMsg) {
    if (pMsg->mComponent == nullptr) {
        return false;
    }
    const char *pszName = pMsg->mComponent->mName;
    if (strcmp(pszName, kDefaultComponent) == 0 || strcmp(pszName, kSaveComponent) == 0) {
        return false;
    }
    ShowOnMap(pszName);
    const String help(FormatString(TheLocale.Localize(kHelpToken, true)));
    TheMetagame.SetHelpText(help.c_str());
    return false;
}
