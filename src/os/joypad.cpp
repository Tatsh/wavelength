#include "os/joypad.h"

#include "app/msgsource.h"
#include "msg/joypadanalogstickmsg.h"
#include "msg/joypadconnectionmsg.h"
#include "msg/joypadinputmsg.h"
#include "os/breugpad.h"
#include "os/debug.h"
#include "os/system.h"
#include "os/timer.h"
#include "script/dataarray.h"

/**
 * Sender of the controller messages.
 *
 * The RTTI includes the class name and records MsgSource as the base. The class adds no member.
 */
class JoypadMsgSource : public MsgSource {};

namespace {

// Multitap slots on each controller port.
constexpr int kSlotsPerPort = 4;

// Port index of slot 0 of the second controller port.
constexpr int kSecondPortFirstSlot = kSlotsPerPort;

// Pad bits of each stick in the button word, starting at kPadLeftStickUp.
constexpr int kStickButtonCount = 4;

// Pressure bytes of a pad report.
constexpr int kReportPressureCount = 16;

// Scale of a signed stick byte to the range -1 to 1.
constexpr float kStickScale = 1.0F / 128.0F;

// Scale of a pressure byte to the range 0 to 1.
constexpr float kPressureMax = 255.0F;

// Defaults of the `joypad` configuration block.
constexpr int kDefaultDeadzone = 24;
constexpr float kDefaultThreshold = 0.75F;

// The wait JoypadWait() makes before JoypadInit().
constexpr int kUninitializedWaitMs = 5000;

// Report pressure byte that feeds each button's pressure, in JoypadButton order. Buttons without a
// pressure sensor read the four bytes past the report's twelve, or byte 0.
// NTSC-U/C: 0x0041bc70
const int kPressureOrder[kJoypadNumButtons] = {10, 11, 8, 9, 4, 5, 6, 7, 12, 13, 14, 15,
                                               2,  0,  3, 1, 0, 0, 0, 0, 0,  0,  0,  0};

// Set once JoypadInit() has read the threshold.
// NTSC-U/C: 0x003b20e0
int gThresholdLoaded;

// NTSC-U/C: 0x003b20e4
int gStickMessages;

// Whether the menus receive each pad's input.
// NTSC-U/C: 0x003b20e8
int gMenuControl[kJoypadNumPads];

// NTSC-U/C: 0x0028adb0, PAL: 0x002945a8 (static initialiser)
// NTSC-U/C: 0x0028ae60, PAL: 0x00294658 (constructor call)
// NTSC-U/C: 0x0028ae80, PAL: 0x00294678 (destructor call)
// NTSC-U/C: 0x0047fe00
JoypadState gStates[kJoypadNumPorts];

// Destructor NTSC-U/C: 0x003a6fb0, PAL: 0x00415c90
// NTSC-U/C: 0x00480200
JoypadMsgSource gJoypadSource;

// The port each pad reads.
// NTSC-U/C: 0x00480210
int gPadPorts[kJoypadNumPorts];

// NTSC-U/C: 0x00480240
BreugPad gPads[kJoypadNumPorts];

// Read a pad's thresholds from the `joypad` configuration block.
// NTSC-U/C: 0x0028a670, PAL: 0x00293e68
void LoadThreshold(DataArray *pConfig) {
    float fThreshold = kDefaultThreshold;
    pConfig->FindFloat("threshold", &fThreshold, true);
    for (auto &state : gStates) {
        state.mThreshold = fThreshold;
    }
    gThresholdLoaded = 1;
}

// Give a port to a pad, or with nPad at kJoypadNoPad take the port from its pad.
// NTSC-U/C: 0x0028ad40, PAL: 0x00294538
void MapPad(int nPad, int nPort) {
    if (nPad == kJoypadNoPad) {
        gStates[nPort].mPad = kJoypadNoPad;
        return;
    }
    gPadPorts[nPad] = nPort;
    gStates[nPort].mPad = nPad;
}

// NTSC-U/C: 0x0028ad98, PAL: 0x00294590
int GetPort(int nPad) {
    return gPadPorts[nPad];
}

// Read a port's pad, and reorder the pressures to JoypadButton order.
// NTSC-U/C: 0x0028af80, PAL: 0x00294778
int ReadPad(int nPort,
            int *pButtons,
            unsigned char *pAxis0,
            unsigned char *pAxis1,
            unsigned char *pAxis2,
            unsigned char *pAxis3,
            unsigned char *pPressures,
            short *pPressureDeltas) {
    // Yes, the binary copies the four bytes past the twelve pressures uninitialised.
    unsigned char abReport[kReportPressureCount];
    const int nLevel =
        gPads[nPort].Read(pButtons, pAxis0, pAxis1, pAxis2, pAxis3, abReport, pPressureDeltas);
    if (pPressures != nullptr) {
        for (int i = 0; i < kJoypadNumButtons; ++i) {
            pPressures[i] = abReport[kPressureOrder[i]];
        }
    }
    return nLevel;
}

} // namespace

JoypadState::JoypadState() {
    mButtons = 0;
    mPad = kJoypadNoPad;
    mConnected = 0;
    for (int i = kJoypadNumButtons - 1; i >= 0; --i) {
        mPressures[i] = 0.0F;
    }
    for (auto &stick : mSticks) {
        stick.mX = 0.0F;
        stick.mY = 0.0F;
    }
}

void JoypadState::AddStickButtons(int *pButtons, bool bMenu) {
    for (int i = 0; i < kJoypadNumSticks; ++i) {
        const int nFirst = i * kStickButtonCount;
        if (mSticks[i].mX > mThreshold) {
            *pButtons |= 1 << (kPadLeftStickRight + nFirst);
        } else if (mSticks[i].mX < -mThreshold) {
            *pButtons |= 1 << (kPadLeftStickLeft + nFirst);
        }
        if (mSticks[i].mY > mThreshold) {
            *pButtons |= 1 << (kPadLeftStickDown + nFirst);
        } else if (mSticks[i].mY < -mThreshold) {
            *pButtons |= 1 << (kPadLeftStickUp + nFirst);
        }
    }
    if (!bMenu) {
        return;
    }
    const JoypadStick &stick = mSticks[0];
    if (stick.mX > mThreshold) {
        *pButtons |= 1 << kPadDRight;
    } else if (stick.mX < -mThreshold) {
        *pButtons |= 1 << kPadDLeft;
    }
    if (stick.mY > mThreshold) {
        *pButtons |= 1 << kPadDDown;
    } else if (stick.mY < -mThreshold) {
        *pButtons |= 1 << kPadDUp;
    }
}

int JoypadWait() {
    if (gThresholdLoaded == 0) {
        TimerSleep(kUninitializedWaitMs);
        return 0;
    }
    while (JoypadGetState(0)->mButtons != 0) {
        JoypadPoll();
    }
    while (JoypadGetState(0)->mButtons == 0) {
        JoypadPoll();
    }
    return JoypadGetState(0)->mButtons;
}

void JoypadPoll() {
    if (gThresholdLoaded == 0) {
        DebugNotify("Cannot call %s before initialization...", __func__);
        return;
    }
    int nButtons = 0;
    for (int i = 0; i < kJoypadNumPorts; ++i) {
        JoypadState &state = gStates[i];
        if (state.mPad == kJoypadNoPad) {
            continue;
        }
        unsigned char abAxes[kJoypadNumSticks * 2];
        unsigned char abPressures[kJoypadNumButtons];
        const int nLevel = ReadPad(
            i, &nButtons, &abAxes[0], &abAxes[1], &abAxes[2], &abAxes[3], abPressures, nullptr);
        if (nLevel == 0) {
            if (state.mConnected == 1) {
                JoypadConnectionMsg msg(state.mPad, 0);
                gJoypadSource.Send(&msg);
                state.mConnected = 0;
            }
            continue;
        }
        // A digital-only pad is ignored.
        if (nLevel < 2) {
            continue;
        }
        if (state.mConnected == 0) {
            JoypadConnectionMsg msg(state.mPad, 1);
            gJoypadSource.Send(&msg);
            state.mConnected = 1;
        }

        for (int j = 0; j < kJoypadNumSticks; ++j) {
            const float fX =
                static_cast<float>(static_cast<signed char>(abAxes[j * 2])) * kStickScale;
            const float fY =
                static_cast<float>(static_cast<signed char>(abAxes[j * 2 + 1])) * kStickScale;
            JoypadStick &stick = state.mSticks[j];
            if (gStickMessages != 0 && (stick.mX != fX || stick.mY != fY)) {
                JoypadAnalogStickMsg msg(state.mPad, j, fX, fY);
                gJoypadSource.Send(&msg);
            }
            stick.mX = fX;
            stick.mY = fY;
        }

        state.AddStickButtons(&nButtons, gMenuControl[state.mPad] != 0);
        const int nChanged = nButtons ^ state.mButtons;
        state.mButtons = nButtons;
        const int nReleased = nChanged & ~nButtons;
        const int nPressed = nChanged & nButtons;
        for (int j = 0; j < kJoypadNumButtons; ++j) {
            state.mPressures[j] = static_cast<float>(abPressures[j]) / kPressureMax;
        }
        for (int j = 0; j < kJoypadNumButtons; ++j) {
            const int nBit = 1 << j;
            if ((nReleased & nBit) != 0) {
                JoypadInputMsg msg(state.mPad, j, 0);
                gJoypadSource.Send(&msg);
            } else if ((nPressed & nBit) != 0) {
                JoypadInputMsg msg(state.mPad, j, 1);
                gJoypadSource.Send(&msg);
            }
        }
    }
}

JoypadState *JoypadGetState(int nPad) {
    return &gStates[gPadPorts[nPad]];
}

void JoypadSetMenuControl(int nPad, bool bMenu) {
    gMenuControl[nPad] = bMenu;
}

void JoypadSetStickMessages(bool bEnable) {
    gStickMessages = bEnable;
}

void JoypadAddSink(MsgSink *pSink) {
    gJoypadSource.AddSink(pSink);
}

void JoypadRemoveSink(MsgSink *pSink) {
    gJoypadSource.RemoveSink(pSink);
}

void JoypadInit() {
    DataArray *pConfig = SystemConfig()->FindArray("joypad", true);
    int nDeadzone = kDefaultDeadzone;
    pConfig->FindInt("deadzone", &nDeadzone, true);
    for (int i = 0; i < kJoypadNumPorts; ++i) {
        gPads[i].Init(i / kSlotsPerPort, i % kSlotsPerPort, nDeadzone);
    }
    JoypadMapDefault(false, false);
    LoadThreshold(pConfig);
}

void JoypadTerminate() {
}

void JoypadSetVibration(int nPad, int nSmallMotor, int nBigMotor) {
    gPads[GetPort(nPad)].SetActuators(nSmallMotor, static_cast<unsigned char>(nBigMotor));
}

void JoypadMapDefault(bool bPort0Multitap, bool bPort1Multitap) {
    if (bPort0Multitap) {
        for (int i = 0; i < kJoypadNumPads; ++i) {
            MapPad(i, i);
            MapPad(kJoypadNoPad, i | kSecondPortFirstSlot);
        }
        return;
    }
    MapPad(0, 0);
    if (!bPort1Multitap) {
        MapPad(1, kSecondPortFirstSlot);
    } else {
        MapPad(kJoypadNoPad, kSecondPortFirstSlot);
    }
    for (int nPort = 0; nPort < kJoypadNumPorts / kSlotsPerPort; ++nPort) {
        for (int nSlot = 1; nSlot < kSlotsPerPort; ++nSlot) {
            MapPad(kJoypadNoPad, (nPort * kSlotsPerPort) | nSlot);
        }
    }
}
