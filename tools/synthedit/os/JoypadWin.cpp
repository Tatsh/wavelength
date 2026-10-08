#include <algorithm>
#include <vector>

#include <windows.h>

#include <dinput.h>

#include "os/DIJoypad.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/System.h"
#include "utl/Data.h"

namespace {

const int kNumButtons = 16;
const int kDefaultDeadzone = 24;

// 0x10040c70
std::vector<DIJoypad *> gDIJoypads;

// 0x10040c08
int gButtonMap[kNumButtons];

// 0x1000b9d0
void JoypadInitButtonMap() {
    const int buttonMap[] = { 4, 5, 6, 7, 0, 1, 2, 3, 8, 11, 10, 9, 12, 13, 14, 15 };
    for (int i = 0; i < kNumButtons; ++i) {
        gButtonMap[i] = buttonMap[i];
    }
}

// 0x1000ba80
bool DIJoypadLess(DIJoypad *a, DIJoypad *b) {
    return a == NULL || b == NULL || a->mJoystickId < b->mJoystickId;
}

// 0x1000bcb0
BOOL CALLBACK EnumJoypadsCallback(LPCDIDEVICEINSTANCE instance, LPVOID context) {
    DIJoypad *joypad = NULL;
    try {
        joypad = new DIJoypad(instance);
        gDIJoypads.push_back(joypad);
    } catch (...) {
        delete joypad;
    }
    return DIENUM_CONTINUE;
}

// 0x1000bab0
void EnumerateJoypads() {
    ASSERT(gDIJoypads.size() == 0);
    gLpdi->EnumDevices(DIDEVTYPE_JOYSTICK, EnumJoypadsCallback, NULL, DIEDFL_ATTACHEDONLY);
    std::sort(gDIJoypads.begin(), gDIJoypads.end(), DIJoypadLess);
    for (unsigned int i = 0; i < gDIJoypads.size(); ++i) {
        JoypadAssignPadToPlayer(i, i);
    }
}

// 0x1000c180
const char *DIErrorString(HRESULT hr) {
    switch (hr) {
    case DIERR_ALREADYINITIALIZED:
        return "ALREADYINITIALIZED\n";
    case DI_PROPNOEFFECT:
        return "PROPNOEFFECT\n";
    case DIERR_BETADIRECTINPUTVERSION:
        return "BETADIRECTINPUTVERSION\n";
    case DIERR_BADDRIVERVER:
        return "BADDRIVERVER\n";
    case DIERR_ACQUIRED:
        return "ACQUIRED\n";
    case DIERR_OLDDIRECTINPUTVERSION:
        return "OLDDIRECTINPUTVERSION\n";
    case DIERR_INVALIDPARAM:
        return "INVALIDPARAM\n";
    case DIERR_OUTOFMEMORY:
        return "OUTOFMEMORY\n";
    case DIERR_OTHERAPPHASPRIO:
        return "OTHERAPPHASPRIO\n";
    case DIERR_NOTINITIALIZED:
        return "NOTINITIALIZED\n";
    case DIERR_NOTFOUND:
        return "NOTFOUND\n";
    case DIERR_NOTACQUIRED:
        return "NOTACQUIRED\n";
    case DIERR_INPUTLOST:
        return "INPUTLOST\n";
    case DIERR_EFFECTPLAYING:
        return "EFFECTPLAYING\n";
    case DIERR_NOTEXCLUSIVEACQUIRED:
        return "NOTEXCLUSIVEACQUIRED\n";
    case DIERR_NOTDOWNLOADED:
        return "NOTDOWNLOADED\n";
    case DIERR_NOTBUFFERED:
        return "NOTBUFFERED\n";
    case DIERR_MOREDATA:
        return "MOREDATA\n";
    case DIERR_INCOMPLETEEFFECT:
        return "INCOMPLETEEFFECT\n";
    case DIERR_HASEFFECTS:
        return "HASEFFECTS\n";
    case DIERR_DEVICEFULL:
        return "DEVICEFULL\n";
    case DIERR_DEVICENOTREG:
        return "DEVICENOTREG\n";
    case DIERR_INSUFFICIENTPRIVS:
        return "INSUFFICIENTPRIVS\n";
    case DIERR_NOAGGREGATION:
        return "NOAGGREGATION\n";
    case DIERR_UNSUPPORTED:
        return "UNSUPPORTED\n";
    case DIERR_NOINTERFACE:
        return "NOINTERFACE\n";
    case DIERR_GENERIC:
        return "GENERIC\n";
    default:
        return "DI Error not found in table.";
    }
}

} // namespace

int gDeadzone = kDefaultDeadzone;

LPDIRECTINPUT gLpdi;

void JoypadInit() {
    DataArray *config = SystemConfig()->FindArray("joypad", true);
    JoypadConfigInit(config);
    gLpdi = NULL;
    if (FAILED(DirectInputCreateA(GetModuleHandleA(NULL), DIRECTINPUT_VERSION, &gLpdi, NULL))) {
        TheDebug.Fail("error in JoypadInit()");
    }
    JoypadInitButtonMap();
    gDeadzone = kDefaultDeadzone;
    config->FindInt("deadzone", &gDeadzone, true);
    EnumerateJoypads();
}

void JoypadTerminate() {
    ASSERT(gLpdi != NULL);
    std::vector<DIJoypad *>::iterator it;
    for (it = gDIJoypads.begin(); it != gDIJoypads.end(); ++it) {
        DIJoypad *joypad = *it;
        ASSERT(joypad != NULL);
        delete joypad;
    }
    gDIJoypads.clear();
    gLpdi->Release();
    gLpdi = NULL;
}

void PrintDIError(HRESULT hr) {
    TheDebug << "Hx DI Error: " << DIErrorString(hr) << "\n";
}
