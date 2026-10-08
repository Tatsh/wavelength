#include "os/cheatsmanager.h"

#include "os/debug.h"
#include "os/joypad.h"
#include "os/system.h"
#include "script/scriptfunction.h"

namespace {

// Buttons held together that select each table of mComboCheats.
constexpr int kComboLeftMask = (1 << kPadL2) | (1 << kPadL1) | (1 << kPadL3);
constexpr int kComboRightMask = (1 << kPadR2) | (1 << kPadR1) | (1 << kPadR3);

// The longest pause between the presses of one sequence.
constexpr float kMaxPressGapMs = 750.0F;

// Nodes of a `long-cheats` entry and of a cheat's `controller` entry.
enum EntryNode { kEntryButtons = 0, kEntryAction = 1, kEntryMarksEntered = 2 };
constexpr int kControllerValue = 1;

// NTSC-U/C: 0x003b235c
CheatsManager *gCheatsManager;

} // namespace

CheatsManager::CheatsManager() {
    mReserved60 = 0;
    mLastPressMs = 0.0F;
    mCheatEntered = 0;
}

void CheatsManager::RunAction(DataArray *pAction, int nPad) {
    DataArray *pController = pAction->FindArray("controller", false);
    if (pController != nullptr) {
        pController->SetNode(kControllerValue, nPad, DataArray::kNodeInt);
    }
    ScriptFunction::Dispatch(pAction);
}

bool CheatsManager::HandleButton(JoypadInputMsg *pMsg) {
    const int nPad = pMsg->mPad;
    const int nHeld = JoypadGetState(nPad)->mButtons;
    const bool bComboLeft = (nHeld & kComboLeftMask) == kComboLeftMask;
    const bool bComboRight = (nHeld & kComboRightMask) == kComboRightMask;
    if (pMsg->mPressed == 0) {
        return true;
    }
    const int nButton = pMsg->mButton;

    if (bComboLeft || bComboRight) {
        for (const auto &cheat : mComboCheats[bComboLeft ? kComboLeft : kComboRight]) {
            if (cheat.mButton == nButton) {
                RunAction(cheat.mAction, nPad);
            }
        }
    }

    const float fLastMs = mLastPressMs;
    mLastPressMs = SystemMs();
    bool bRestarted = false;
    if (!mHistory.empty() && mLastPressMs - fLastMs > kMaxPressGapMs) {
        bRestarted = true;
        mHistory.clear();
    }
    mHistory.push_back(nButton);
    if (mHistory.size() > kMaxHistory) {
        mHistory.pop_front();
    }
    if (bRestarted) {
        return true;
    }

    for (const auto &cheat : mLongCheats) {
        bool bMatch = true;
        for (unsigned int i = 0; i < cheat.mButtons.size(); ++i) {
            if (i >= mHistory.size() || mHistory[i] != cheat.mButtons[i]) {
                bMatch = false;
                break;
            }
        }
        if (bMatch) {
            RunAction(cheat.mAction, nPad);
            mHistory.clear();
            mCheatEntered = mCheatEntered != 0 || cheat.mMarksEntered != 0;
            return true;
        }
    }
    return true;
}

bool CheatsManager::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() != g_nJoypadInputMsgType) {
        return false;
    }
    return HandleButton(static_cast<JoypadInputMsg *>(pMsg));
}

void CheatsManager::TestCmd(DataArray *pCommand, void *pUserData) {
    (void)pUserData;
    PrnStream &stream = TheDebug << "TestCmd called with data args:";
    pCommand->Print(stream);
    stream << "\n";
}

void CheatsManager::LoadLongCheats(DataArray *pCheats) {
    for (int i = 1; i < pCheats->Size(); ++i) {
        DataArray *pEntry = pCheats->Array(i);
        DataArray *pButtons = pEntry->Array(kEntryButtons);
        DataArray *pAction = pEntry->Array(kEntryAction);
        if (pButtons->Size() > kMaxCheatButtons) {
            DebugPrint("Too many buttons in long cheat, max %d\n", kMaxCheatButtons);
            continue;
        }
        LongCheat cheat;
        bool bValid = true;
        for (int j = 0; j < pButtons->Size(); ++j) {
            const int nButton = pButtons->Int(j);
            if (static_cast<unsigned int>(nButton) >= kJoypadNumButtons) {
                bValid = false;
                // Yes, the button number is passed for the %s.
                DebugPrint("Error in long-cheats: %s is not a valid button\n", nButton);
                break;
            }
            cheat.mButtons.push_back(nButton);
        }
        if (bValid) {
            cheat.mAction = pAction;
            cheat.mMarksEntered = pEntry->Int(kEntryMarksEntered) != 0;
            gCheatsManager->mLongCheats.push_back(cheat);
        }
    }
}

int CheatsManager::IsCheatEntered() {
    return gCheatsManager->mCheatEntered;
}

void CheatsManager::Init() {
    gCheatsManager = new CheatsManager;
    JoypadAddSink(gCheatsManager);
    LoadLongCheats(SystemConfig()->FindArray("long-cheats", true));
    ScriptFunction::Register(TestCmd, "test", nullptr);
}

void CheatsManager::Terminate() {
    JoypadRemoveSink(gCheatsManager);
    delete gCheatsManager;
    gCheatsManager = nullptr;
}
