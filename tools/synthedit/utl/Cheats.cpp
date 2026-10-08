#include "utl/Cheats.h"

#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/System.h"
#include "os/Timer.h"
#include "utl/DataFunc.h"

namespace {

// The quick-cheat lists, by modifier.
enum QuickCheatList {
    kQuickCheatsLeft = 0,
    kQuickCheatsRight = 1,
};

// The buttons held for each modifier.
const int kLeftModifierButtons = 0x205;
const int kRightModifierButtons = 0x40a;

// Buttons are numbered below this.
const int kNumButtons = 24;

// A long cheat has at most this many buttons, and the history keeps this many.
const int kMaxLongCheatButtons = 16;

// A longer pause between presses starts a new sequence.
const float kSequenceTimeoutMs = 750.0f;

// The nodes of a cheat entry.
enum CheatNode {
    kCheatButtons = 0,
    kCheatCommand = 1,
    kCheatFlag = 2,
};

// The node of the `controller` entry that receives the controller.
const int kControllerNode = 1;

} // namespace

// 0x100390a8
int gJoypadButtonMsgType = 100;

// 0x100c38f4
static CheatsManager *gCheatsManager;

namespace {

// Add the quick cheats of one modifier.
// 0x10013be0
void AddQuickCheats(DataArray *cheats, int list) {
    for (int i = 1; i < cheats->Size(); ++i) {
        DataArray *cheat = cheats->Array(i);
        if (cheat->Int(0) >= 0 && cheat->Int(0) < kNumButtons) {
            QuickCheat quick;
            quick.mButton = cheat->Int(0);
            quick.mCommand = cheat->Array(1);
            gCheatsManager->mQuickCheats[list].push_back(quick);
        } else {
            TheDebug.Printf("Error in quick-cheats: %s is not a valid button\n", cheat->Sym(0));
        }
    }
}

// Add the long cheats.
// 0x10013cc0
void AddLongCheats(DataArray *cheats) {
    for (int i = 1; i < cheats->Size(); ++i) {
        DataArray *cheat = cheats->Array(i);
        DataArray *buttons = cheat->Array(kCheatButtons);
        DataArray *command = cheat->Array(kCheatCommand);
        if (buttons->Size() > kMaxLongCheatButtons) {
            TheDebug.Printf("Too many buttons in long cheat, max %d\n", kMaxLongCheatButtons);
            continue;
        }
        std::vector<int> sequence;
        bool valid = true;
        for (int j = 0; j < buttons->Size(); ++j) {
            const int button = buttons->Int(j);
            if (button < 0 || button >= kNumButtons) {
                // Yes, the button is printed with %s.
                TheDebug.Printf("Error in long-cheats: %s is not a valid button\n", button);
                valid = false;
                break;
            }
            sequence.push_back(button);
        }
        if (!valid) {
            continue;
        }
        LongCheat longCheat;
        longCheat.mButtons = sequence;
        longCheat.mCommand = command;
        longCheat.mFlag = cheat->Int(kCheatFlag) != 0;
        gCheatsManager->mLongCheats.push_back(longCheat);
    }
}

// Print the arguments of the `test` command.
// 0x10013bb0
void DataTestCmd(DataArray *args, void * /*data*/) {
    PrnStream &out = TheDebug << "TestCmd called with data args:";
    args->Print(out);
    out << "\n";
}

} // namespace

CheatsManager::CheatsManager() : mLastTime(0.0f), mFlags(false) {
}

CheatsManager::~CheatsManager() {
}

bool CheatsManager::DispatchPriv(Message *msg) {
    if (msg->Type() != gJoypadButtonMsgType) {
        return false;
    }
    return OnButtonDown(static_cast<JoypadButtonMsg *>(msg));
}

void CheatsManager::ExecuteCheat(DataArray *command, int pad) {
    DataArray *controller = command->FindArray("controller", false);
    if (controller != NULL) {
        DataArray::DataNode value;
        value.i = pad;
        controller->SetNode(kControllerNode, value, DataArray::kDataInt);
    }
    DataExecute(command);
}

bool CheatsManager::OnButtonDown(JoypadButtonMsg *msg) {
    const int pad = msg->mPad;
    const int held = JoypadGetPlayerData(pad)->mButtons;
    const bool left = (held & kLeftModifierButtons) == kLeftModifierButtons;
    const bool right = (held & kRightModifierButtons) == kRightModifierButtons;
    if (!msg->mPressed) {
        return true;
    }
    const int button = msg->mButton;
    if (left || right) {
        std::vector<QuickCheat> &quick = mQuickCheats[left ? kQuickCheatsLeft : kQuickCheatsRight];
        std::vector<QuickCheat>::iterator it;
        for (it = quick.begin(); it != quick.end(); ++it) {
            if (it->mButton == button) {
                ExecuteCheat(it->mCommand, pad);
            }
        }
    }

    const float now = TimerUpdateMs();
    const float last = mLastTime;
    mLastTime = now;
    bool restarted = false;
    if (mButtons.size() != 0 && now - last > kSequenceTimeoutMs) {
        mButtons.clear();
        restarted = true;
    }
    mButtons.push_back(button);
    if (mButtons.size() > kMaxLongCheatButtons) {
        mButtons.pop_front();
    }
    if (restarted) {
        return true; // Yes, the button that starts a new sequence never completes a cheat.
    }

    std::vector<LongCheat>::iterator cheat;
    for (cheat = mLongCheats.begin(); cheat != mLongCheats.end(); ++cheat) {
        unsigned int i = 0;
        bool matched = true;
        while (i < cheat->mButtons.size()) {
            if (i >= mButtons.size() || mButtons[i] != cheat->mButtons[i]) {
                matched = false;
                break;
            }
            ++i;
        }
        if (matched) {
            ExecuteCheat(cheat->mCommand, pad);
            mButtons.clear();
            mFlags |= cheat->mFlag;
            break;
        }
    }
    return true;
}

void CheatsInit() {
    ASSERT(gCheatsManager == NULL);
    gCheatsManager = new CheatsManager;
    JoypadSubscribe(gCheatsManager);
    DataArray *quick = SystemConfig()->FindArray("quick-cheats", true);
    AddQuickCheats(quick->FindArray("left", true), kQuickCheatsLeft);
    AddQuickCheats(quick->FindArray("right", true), kQuickCheatsRight);
    AddLongCheats(SystemConfig()->FindArray("long-cheats", true));
    DataRegisterFunc(DataTestCmd, "test", NULL);
}

void CheatsTerminate() {
    ASSERT(gCheatsManager);
    JoypadUnsubscribe(gCheatsManager);
    delete gCheatsManager;
    gCheatsManager = NULL;
}
