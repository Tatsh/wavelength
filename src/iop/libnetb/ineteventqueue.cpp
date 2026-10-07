#include "libnetb/ineteventqueue.h"

#include <stdio.h>

#include <inet.h>
#include <kernel.h>

#include "libnetb/libnetb.h"
#include "libnetb/libnetrecords.h"

namespace {

// Events the module reacts to besides INETCTL_EVENT_START. Their inetctl meanings are not
// identified.
constexpr int kInetCtlEvent1 = 1;
constexpr int kInetCtlEvent2 = 2;
constexpr int kInetCtlEvent3 = 3;
constexpr int kInetCtlEvent5 = 5;
constexpr int kInetCtlEvent7 = 7;
constexpr int kInetCtlEvent9 = 9;

constexpr int kEventRetries = 5;
constexpr int kRetryDelay = 300000;

constexpr char kEventMessage[] = "Libnet: type %d: id %d\n";
constexpr char kErrorMessage[] = "# ERR # %s: %d (%d)\n";
constexpr char kRetryMessage[] = "Libnet: retry %d\n";

} // namespace

// NTSC-U/C: 0x00005cc8
int InetEventQueue::sInterfaceIds[kInterfaceCount];

// NTSC-U/C: 0x00005cd0
int InetEventQueue::sLockSema;

// NTSC-U/C: 0x00005cd4
int InetEventQueue::sCountSema;

// NTSC-U/C: 0x00005cd8
int InetEventQueue::sReadIndex;

// NTSC-U/C: 0x00005cdc
int InetEventQueue::sWriteIndex;

// NTSC-U/C: 0x00005ce0
unsigned int InetEventQueue::sFlags;

// NTSC-U/C: 0x00005d00
sceInetCtlEventHandler InetEventQueue::sHandler;

// NTSC-U/C: 0x00005d10
int InetEventQueue::sInterfaceIdQueue[kQueueSize];

// NTSC-U/C: 0x00005d50
int InetEventQueue::sEventQueue[kQueueSize];

void InetEventQueue::EventHandler(int interfaceId, int event) {
    sFlags |= kFlagEventReceived;
    if (Libnet::sVerbose != 0) {
        printf(kEventMessage, event, interfaceId);
    }
    int result = WaitSema(sLockSema);
    if (result != KE_OK) {
        printf(kErrorMessage, __FILE__, __LINE__, result);
    }
    sInterfaceIdQueue[sWriteIndex] = interfaceId;
    sEventQueue[sWriteIndex] = event;
    sWriteIndex = (sWriteIndex + 1) % kQueueSize;
    result = SignalSema(sLockSema);
    if (result != KE_OK) {
        printf(kErrorMessage, __FILE__, __LINE__, result);
    }
    if (SignalSema(sCountSema) != KE_OK) {
        sFlags |= kFlagOverflow;
    }
}

void InetEventQueue::RecordInterface(int interfaceId) {
    unsigned int flags;
    sceInetInterfaceControl(interfaceId, INET_CONTROL_GET_FLAGS, &flags, sizeof(flags));
    if ((flags & INET_INTERFACE_PASSIVE) == 0) {
        sInterfaceIds[0] = interfaceId;
        return;
    }
    for (int i = 1; i < kInterfaceCount; ++i) {
        if (sInterfaceIds[i] == 0) {
            sInterfaceIds[i] = interfaceId;
            return;
        }
    }
}

int InetEventQueue::IsPrimaryInterface(int interfaceId) {
    unsigned int flags;
    sceInetInterfaceControl(interfaceId, INET_CONTROL_GET_FLAGS, &flags, sizeof(flags));
    if ((flags & INET_INTERFACE_PASSIVE) != 0) {
        return kInterfaceNone;
    }
    return sInterfaceIds[0] == interfaceId ? kInterfaceUp : kInterfaceNone;
}

int InetEventQueue::GetEvent(int *interfaceId, int *event) {
    *interfaceId = 0;
    *event = 0;
    if ((sFlags & kFlagEventReceived) == 0) {
        int retries = kEventRetries;
        do {
            if (retries <= 0) {
                return kLibnetErrorNoEvent;
            }
            --retries;
            if (Libnet::sVerbose != 0) {
                printf(kRetryMessage, retries);
            }
            DelayThread(kRetryDelay);
        } while ((sFlags & kFlagEventReceived) == 0);
    }
    if ((sFlags & kFlagOverflow) != 0) {
        *interfaceId = -1;
        return kLibnetErrorNoEvent;
    }
    if (WaitSema(sCountSema) != KE_OK) {
        return kLibnetErrorSemaphore;
    }
    if (WaitSema(sLockSema) != KE_OK) {
        return kLibnetErrorSemaphore;
    }
    *interfaceId = sInterfaceIdQueue[sReadIndex];
    *event = sEventQueue[sReadIndex];
    sReadIndex = (sReadIndex + 1) % kQueueSize;
    if (SignalSema(sLockSema) != KE_OK) {
        return kLibnetErrorSemaphore;
    }
    return 0;
}

int InetEventQueue::MapEvent(int interfaceId, int event) {
    switch (event) {
    case kInetCtlEvent1:
        return IsPrimaryInterface(interfaceId);
    case kInetCtlEvent2:
    case kInetCtlEvent5:
        return sInterfaceIds[0] == interfaceId ? kInterfaceLost : kInterfaceNone;
    case kInetCtlEvent3:
        return kInterfaceDown;
    case INETCTL_EVENT_START:
        RecordInterface(interfaceId);
        return kInterfaceNone;
    case kInetCtlEvent7:
        return sInterfaceIds[0] == 0 ? kInterfaceLost : kInterfaceNone;
    case kInetCtlEvent9:
        return kInterfaceLost;
    default:
        return kInterfaceNone;
    }
}
