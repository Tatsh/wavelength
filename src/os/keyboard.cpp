#include "os/keyboard.h"

#include <libusbkb.h>

#include "app/msgsource.h"
#include "msg/keyboardkeymsg.h"

namespace {

// The keyboard the layer reads.
constexpr unsigned int kKeyboard = 0;

// Settings KeyboardInit() gives the keyboard.
constexpr int kArrangement = 0;
constexpr int kCodeType = 1;
constexpr int kReadMode = 0;
constexpr int kLedMode = 1;
constexpr int kRepeatStart = 40;
constexpr int kRepeatInterval = 7;

// sceUsbKbSync() mode that returns at once.
constexpr int kSyncPoll = 1;

// Values of gRetry besides a countdown.
constexpr int kRetryAttached = -1;
constexpr int kRetryQuery = 0;
constexpr int kRetryPolls = 100;

// The request sceUsbKbSync() finishes.
enum Request { kRequestNone = 0, kRequestRead = 1, kRequestInfo = 2 };

// Bits of a raw key code that hold the USB usage.
constexpr unsigned int kUsageMask = 0xfff;

// USB usages Translate() recognises.
enum Usage {
    kUsageEscape = 0x29,
    kUsageCapsLock = 0x39,
    kUsageF1 = 0x3a,
    kUsagePrintScreen = 0x46,
    kUsageScrollLock = 0x47,
    kUsagePause = 0x48,
    kUsageInsert = 0x49,
    kUsageHome = 0x4a,
    kUsagePageUp = 0x4b,
    kUsageDelete = 0x4c,
    kUsageEnd = 0x4d,
    kUsagePageDown = 0x4e,
    kUsageRight = 0x4f,
    kUsageLeft = 0x50,
    kUsageDown = 0x51,
    kUsageUp = 0x52,
    kUsageNumLock = 0x53
};

// Function keys F1 to F12.
constexpr unsigned int kFunctionKeyCount = 12;

// Keys KeyboardKeyMsg reports for keys without a character.
enum Key {
    kKeyNone = 0,
    kKeyCapsLock = 0x122,
    kKeyNumLock = 0x123,
    kKeyScrollLock = 0x124,
    kKeyPrintScreen = 0x12c,
    kKeyPause = 0x12d,
    kKeyEscape = 0x12e,
    kKeyInsert = 0x136,
    kKeyDelete = 0x137,
    kKeyHome = 0x138,
    kKeyEnd = 0x139,
    kKeyPageUp = 0x13a,
    kKeyPageDown = 0x13b,
    kKeyLeft = 0x140,
    kKeyRight = 0x141,
    kKeyUp = 0x142,
    kKeyDown = 0x143,
    kKeyF1 = 0x191
};

// NTSC-U/C: 0x003b2100
int gActive;

// NTSC-U/C: 0x0028b4b0, PAL: 0x00294ca8 (static initialiser)
// NTSC-U/C: 0x0028b518, PAL: 0x00294d10 (constructor call)
// NTSC-U/C: 0x0028b538, PAL: 0x00294d30 (destructor call)
// NTSC-U/C: 0x00480e40
MsgSource gKeyboardSource;

// NTSC-U/C: 0x00480e80
USBKBINFO_t gInfo;

// NTSC-U/C: 0x00480f40
USBKBDATA_t gData;

// NTSC-U/C: 0x00480fc8
int gRequest;

// kRetryAttached while a keyboard is attached, kRetryQuery to query, and otherwise the polls
// before the next query.
// NTSC-U/C: 0x00480fcc
int gRetry;

// Map a key code to the key KeyboardKeyMsg reports, or kKeyNone for a key without one.
// NTSC-U/C: 0x0028b3d0, PAL: 0x00294bc8
int Translate(unsigned int nCode) {
    const unsigned int nUsage = nCode & kUsageMask;
    if ((nCode & USBKB_RAWDAT) == 0) {
        return static_cast<int>(nUsage);
    }
    if (nUsage - kUsageF1 < kFunctionKeyCount) {
        return static_cast<int>(nUsage - kUsageF1) + kKeyF1;
    }
    switch (nUsage) {
    case kUsageEscape:
        return kKeyEscape;
    case kUsageCapsLock:
        return kKeyCapsLock;
    case kUsagePrintScreen:
        return kKeyPrintScreen;
    case kUsageScrollLock:
        return kKeyScrollLock;
    case kUsagePause:
        return kKeyPause;
    case kUsageInsert:
        return kKeyInsert;
    case kUsageHome:
        return kKeyHome;
    case kUsagePageUp:
        return kKeyPageUp;
    case kUsageDelete:
        return kKeyDelete;
    case kUsageEnd:
        return kKeyEnd;
    case kUsagePageDown:
        return kKeyPageDown;
    case kUsageRight:
        return kKeyRight;
    case kUsageLeft:
        return kKeyLeft;
    case kUsageDown:
        return kKeyDown;
    case kUsageUp:
        return kKeyUp;
    case kUsageNumLock:
        return kKeyNumLock;
    default:
        return kKeyNone;
    }
}

// NTSC-U/C: 0x0028b318, PAL: 0x00294b10
void OnRead(bool bSucceeded) {
    if (!bSucceeded) {
        gRetry = kRetryPolls;
        return;
    }
    if (gData.len == 0 || gData.keycode[0] == 0) {
        return;
    }
    const int nKey = Translate(gData.keycode[0]);
    if (nKey != kKeyNone) {
        KeyboardKeyMsg msg(nKey);
        gKeyboardSource.Send(&msg);
    }
}

// NTSC-U/C: 0x0028b3a8, PAL: 0x00294ba0
void OnInfo() {
    gRetry = gInfo.now_connect != 0 ? kRetryAttached : kRetryPolls;
}

} // namespace

void KeyboardInit() {
    gActive = 1;
    gRetry = kRetryQuery;
    gRequest = kRequestNone;
    int nMaxConnect;
    sceUsbKbInit(&nMaxConnect);
    sceUsbKbSetArrangement(kKeyboard, kArrangement);
    sceUsbKbSetCodeType(kKeyboard, kCodeType);
    sceUsbKbSetReadMode(kKeyboard, kReadMode);
    sceUsbKbSetLEDMode(kKeyboard, kLedMode);
    sceUsbKbSetRepeat(kKeyboard, kRepeatStart, kRepeatInterval);
    sceUsbKbClearRbuf(kKeyboard);
}

void KeyboardTerminate() {
    sceUsbKbEnd();
    gActive = 0;
}

void KeyboardPoll() {
    if (gRequest == kRequestNone) {
        if (gRetry == kRetryAttached) {
            sceUsbKbRead(kKeyboard, &gData);
            gRequest = kRequestRead;
        } else if (gRetry == kRetryQuery) {
            sceUsbKbGetInfo(&gInfo);
            gRequest = kRequestInfo;
        } else {
            --gRetry;
        }
    }
    if (gRequest == kRequestNone) {
        return;
    }
    int nResult;
    if (sceUsbKbSync(kSyncPoll, &nResult) != 0) {
        return;
    }
    if (gRequest == kRequestRead) {
        OnRead(nResult == 0);
    } else if (gRequest == kRequestInfo) {
        OnInfo();
    }
    gRequest = kRequestNone;
}

void KeyboardAddSink(MsgSink *pSink) {
    gKeyboardSource.AddSink(pSink);
}

void KeyboardRemoveSink(MsgSink *pSink) {
    gKeyboardSource.RemoveSink(pSink);
}
