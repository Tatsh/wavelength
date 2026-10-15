#include <stdio.h>
#include <string.h>

#include "rt/rtdme.h"
#include "rt/rtmemory.h"

enum {
    // The flags the voice sends its system messages with.
    kVoiceSendFlags = RT_DME_SEND_ORDERED | RT_DME_SEND_GUARANTEED,
    // The result for a failure of the device.
    kVoiceDeviceError = 42,
    // The result for a voice that is missing or not open.
    kVoiceNotReady = 41,
    // The devices that can be open at once, and so the ports they take.
    kVoicePortCount = 4,
    // The kinds of the two addresses of an endpoint.
    kEndpointLocalKind = 2,
    kEndpointPublicKind = 1,
    // The kind of the gateway address that reaches the device through the gateway.
    kGatewayRelayed = 3,
    // The modes of the channels of the device.
    kChannelModeDefault = 0,
    kChannelModeAlternate = 3,
    // The values of the default settings.
    kDefaultVersion = 1,
    kDefaultChannelCount = 1,
    kDefaultBits = 16,
    kDefaultRate = 8000,
    kDefaultChannels = 1,
    kSlotStartActive = 1,
    // The bytes of an address, and the digits of a byte as text.
    kAddressBytes = 4,
    kByteBits = 8,
    kHundreds = 100,
    kTens = 10,
};

// The events the receiver of the events reports, as the device takes them.
enum {
    kVoiceEventStarted = 0,
    kVoiceEventStopped = 1,
    kVoiceEventSending = 2,
    kVoiceEventReceiving = 3,
    kVoiceEventIdle = 4,
};

// The codes of the device for the events.
enum {
    kDeviceCodeStarted = 33,
    kDeviceCodeSending = 34,
    kDeviceCodeIdle = 35,
    kDeviceCodeReceiving = 36,
    kDeviceCodeStopped = 63,
};

// The two addresses of the device of the voice, as text.
typedef struct {
    int nLocalKind;
    char szLocalAddress[RT_NET_ADDRESS_SIZE];
    int nLocalPort;
    int nKind;
    char szAddress[RT_NET_ADDRESS_SIZE];
    int nPort;
} RtDmeVoiceEndpoint;

// A client of the voice.
typedef struct {
    int nClient;
    unsigned char bActive;
    unsigned char bAdded;     // The device has the client as a peer.
    unsigned char bAnnounced; // The client was sent the voice address of this console.
    unsigned char uReserved07;
} RtDmeVoiceSlot;

// The voice of a connection. 340 bytes.
typedef struct {
    int nConnection;
    int nClientCount;
    unsigned char bOpen;
    unsigned char bStarted;
    unsigned char bLocalStarted;
    unsigned char uReserved0b;
    int nLocalClient;
    rtDmeVoiceSettings settings;
    RtDmeDeviceSettings device;
    RtDmeDevice *pDevice;
    unsigned char bStreaming;
    unsigned char bReady; // The device can reach the peers.
    unsigned char abReserved9e[2];
    RtDmeDevicePeer peer;
    RtDmeVoiceEndpoint endpoint;
    int anChannels[RT_DME_VOICE_CHANNEL_COUNT];
    unsigned char uReserved128; // Cleared when opened, and not read.
    unsigned char abReserved129[7];
    RtDmeVoiceSlot *pSlots;
    unsigned char abReserved134[32];
} RtDmeVoice;

// NTSC-U/C: 0x003b6be0, PAL: 0x00435aa0
static unsigned char g_bDmeVoiceStarted;
// NTSC-U/C: 0x003b6be2, PAL: 0x00435aa2
static short g_nDmeVoiceStarts;
// NTSC-U/C: 0x00519f10
static RtDmeVoice *g_apDmeVoices[RT_DME_CONNECTION_COUNT];

static inline int RtDmeVoiceIsStarted(void) {
    return g_nDmeVoiceStarts > 0 && g_bDmeVoiceStarted == 1;
}

// NTSC-U/C: 0x002af950, PAL: 0x002b98c0
static int RtDmeVoiceResult(int nResult, int nError) {
    return nResult != 0 ? nError : 0;
}

// NTSC-U/C: 0x002afa90, PAL: 0x002b9a00
static int RtDmeVoiceCreateDevice(RtDmeDevice **ppDevice, const RtDmeDeviceSettings *pSettings) {
    RtDmeDevice *pDevice;
    int nResult;

    pDevice = rtDmeDeviceCreate();
    nResult = rtDmeDeviceInit(pDevice, pSettings);
    *ppDevice = pDevice;
    return nResult;
}

// NTSC-U/C: 0x002afadc, PAL: 0x002b9a4c
static int RtDmeVoiceConnectDevice(RtDmeDevice *pDevice,
                                   const char *pszAddress,
                                   int nPort,
                                   RtDmeDeviceStreamFunc pfnStream) {
    return rtDmeDeviceMapPort(pDevice, pszAddress, nPort, pfnStream);
}

// NTSC-U/C: 0x002afaf8, PAL: 0x002b9a68
static int
RtDmeVoiceOpenChannel(RtDmeDevice *pDevice, int nMode, int *pnChannel, const char *pszName) {
    return rtDmeDeviceOpenConnection(pDevice, nMode, pnChannel, pszName);
}

// NTSC-U/C: 0x002afb14, PAL: 0x002b9a84
static void RtDmeVoiceStopDevice(RtDmeDevice *pDevice) {
    rtDmeDeviceStop(pDevice);
}

// NTSC-U/C: 0x002afb30, PAL: 0x002b9aa0
static int RtDmeVoiceSetDeviceClient(RtDmeDevice *pDevice, int nClient) {
    return rtDmeDeviceSetLocalId(pDevice, nClient);
}

// NTSC-U/C: 0x002afb4c, PAL: 0x002b9abc
static int RtDmeVoiceGetDevicePeer(RtDmeDevice *pDevice, RtDmeDevicePeer *pPeer) {
    return rtDmeDeviceGetLocalInfo(pDevice, pPeer);
}

// NTSC-U/C: 0x002afb68, PAL: 0x002b9ad8
static int RtDmeVoiceAddDevicePeer(RtDmeDevice *pDevice, const RtDmeDevicePeer *pPeer) {
    return rtDmeDeviceAddPeer(pDevice, pPeer) != 0 ? 0 : -1;
}

// NTSC-U/C: 0x002afb90, PAL: 0x002b9b00
static int RtDmeVoiceRemoveDevicePeer(RtDmeDevice *pDevice, int nClient) {
    return rtDmeDeviceRemovePeer(pDevice, nClient);
}

// NTSC-U/C: 0x002afbac, PAL: 0x002b9b1c
static int RtDmeVoiceSelectChannel(RtDmeDevice *pDevice, int nChannel) {
    return rtDmeDeviceDeselectConnection(pDevice, nChannel);
}

// NTSC-U/C: 0x002afbc8, PAL: 0x002b9b38
static int RtDmeVoiceGetDeviceStatus(RtDmeDevice *pDevice, RtDmeDeviceStatus *pStatus) {
    return rtDmeDeviceGetStatus(pDevice, pStatus);
}

// NTSC-U/C: 0x002afbe4, PAL: 0x002b9b54
static int RtDmeVoiceUpdateDevice(RtDmeDevice *pDevice) {
    return rtDmeDeviceUpdate(pDevice);
}

// NTSC-U/C: 0x002afc00, PAL: 0x002b9b70
static void RtDmeVoiceDestroyDevice(RtDmeDevice *pDevice) {
    if (pDevice != NULL) {
        rtDmeDeviceDestroy(pDevice);
    }
}

unsigned int rtNetParseAddress(const char *address) {
    unsigned int uValue;
    unsigned int uPart;
    char c;

    uValue = 0;
    uPart = 0;
    if (address == NULL) {
        return uValue;
    }
    c = *address++;
    while (c != '\0') {
        if (c == '.') {
            uValue = (uValue << kByteBits) + uPart;
            uPart = 0;
        } else if ((unsigned int)(c - '0') < kTens) {
            uPart = uPart * kTens + c - '0';
        }
        c = *address++;
    }
    return (uValue << kByteBits) + uPart;
}

void rtNetFormatAddress(unsigned int value, char *address) {
    unsigned int uByte;
    unsigned int i;

    if (address == NULL) {
        return;
    }
    for (i = 0; i < kAddressBytes; ++i) {
        if (i != 0) {
            *address++ = '.';
        }
        uByte = (value & 0xff000000U) >> 24;
        if (uByte >= kHundreds) {
            *address++ = uByte / kHundreds + '0';
            uByte %= kHundreds;
            *address++ = uByte / kTens + '0';
            *address++ = uByte % kTens + '0';
        } else if (uByte >= kTens) {
            *address++ = uByte / kTens + '0';
            *address++ = uByte % kTens + '0';
        } else {
            *address++ = uByte + '0';
        }
        value <<= kByteBits;
    }
    *address = '\0';
}

// NTSC-U/C: 0x002aeb98, PAL: 0x002b8b08
static int RtDmeVoiceGet(int nConnection, RtDmeVoice **ppVoice) {
    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (RtDmeIsConnectionIndex(nConnection) != 1 || ppVoice == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *ppVoice = g_apDmeVoices[nConnection];
    return 0;
}

// NTSC-U/C: 0x002aec20, PAL: 0x002b8b90
static int RtDmeVoiceIsReady(const RtDmeVoice *pVoice) {
    if (pVoice == NULL || pVoice->bOpen != 1) {
        return 0;
    }
    return RtDmeIsConnectionIndex(pVoice->nConnection) == 1;
}

// NTSC-U/C: 0x002aec68, PAL: 0x002b8bd8
static int RtDmeVoiceGetReady(int nConnection, RtDmeVoice **ppVoice) {
    int nResult;

    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (RtDmeIsConnectionIndex(nConnection) != 1) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeVoiceGet(nConnection, ppVoice);
    if (nResult != 0) {
        return nResult;
    }
    return RtDmeVoiceIsReady(*ppVoice) == 1 ? 0 : kVoiceNotReady;
}

// NTSC-U/C: 0x002aed10, PAL: 0x002b8c80
static int RtDmeVoiceFindByDevice(const RtDmeDevice *pDevice, RtDmeVoice **ppVoice) {
    RtDmeVoice *pVoice;
    int nResult;
    int i;

    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (ppVoice == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *ppVoice = NULL;
    for (i = 0; i < RT_DME_CONNECTION_COUNT; ++i) {
        nResult = RtDmeVoiceGet(i, &pVoice);
        if (nResult != 0) {
            return nResult;
        }
        if (pVoice != NULL && pVoice->pDevice == pDevice) {
            *ppVoice = pVoice;
            break;
        }
    }
    return RtDmeVoiceIsReady(*ppVoice) == 1 ? 0 : kVoiceNotReady;
}

// NTSC-U/C: 0x002aedd0, PAL: 0x002b8d40
static int RtDmeVoiceGetSlot(RtDmeVoice *pVoice, int nClient, RtDmeVoiceSlot **ppSlot) {
    RtDmeVoiceSlot *pSlot;

    if (ppSlot == NULL || RtDmeVoiceIsReady(pVoice) != 1 || pVoice->pSlots == NULL ||
        nClient >= pVoice->nClientCount) {
        return RT_DME_RESULT_INVALID;
    }
    pSlot = &pVoice->pSlots[nClient];
    *ppSlot = pSlot;
    return pSlot != NULL ? 0 : kVoiceNotReady;
}

// NTSC-U/C: 0x002aeef0, PAL: 0x002b8e60
static int RtDmeVoiceIsPortUsed(int nPort, unsigned char *pbUsed) {
    RtDmeVoice *pVoice;
    int nResult;
    int i;

    if (pbUsed == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *pbUsed = 1;
    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    for (i = 0; i < RT_DME_CONNECTION_COUNT; ++i) {
        nResult = RtDmeVoiceGet(i, &pVoice);
        if (nResult != 0) {
            return nResult;
        }
        if (pVoice != NULL && pVoice->bOpen == 1 && nPort == pVoice->settings.port) {
            *pbUsed = pVoice->bOpen;
            return 0;
        }
    }
    *pbUsed = 0;
    return 0;
}

// NTSC-U/C: 0x002aee58, PAL: 0x002b8dc8
static int RtDmeVoiceAllocPort(int *pnPort) {
    unsigned char bUsed;
    int nResult;
    int nPort;

    bUsed = 0;
    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (pnPort == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    for (nPort = RT_DME_VOICE_FIRST_PORT; nPort < RT_DME_VOICE_FIRST_PORT + kVoicePortCount;
         ++nPort) {
        nResult = RtDmeVoiceIsPortUsed(nPort, &bUsed);
        if (nResult != 0) {
            return nResult;
        }
        if (!bUsed) {
            *pnPort = nPort;
            return 0;
        }
    }
    return kVoiceNotReady;
}

// NTSC-U/C: 0x002aefb8, PAL: 0x002b8f28
static int RtDmeVoiceMakeEndpoint(const RtDmeDevicePeer *pPeer, RtDmeVoiceEndpoint *pEndpoint) {
    if (pPeer == NULL || pEndpoint == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    memset(pEndpoint, 0, sizeof(RtDmeVoiceEndpoint));
    pEndpoint->nLocalKind = kEndpointLocalKind;
    pEndpoint->nLocalPort = pPeer->nLocalPort;
    rtNetFormatAddress(pPeer->uLocalAddress, pEndpoint->szLocalAddress);
    pEndpoint->nKind = kEndpointPublicKind;
    pEndpoint->nPort = pPeer->nPort;
    rtNetFormatAddress(pPeer->uAddress, pEndpoint->szAddress);
    return 0;
}

// NTSC-U/C: 0x002af338, PAL: 0x002b92a8
static int RtDmeVoiceEventToCode(int nEvent, int *pnCode) {
    if (pnCode == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    switch (nEvent) {
    case kVoiceEventStarted:
        *pnCode = kDeviceCodeStarted;
        break;
    case kVoiceEventStopped:
        *pnCode = kDeviceCodeStopped;
        break;
    case kVoiceEventSending:
        *pnCode = kDeviceCodeSending;
        break;
    case kVoiceEventReceiving:
        *pnCode = kDeviceCodeReceiving;
        break;
    case kVoiceEventIdle:
        *pnCode = kDeviceCodeIdle;
        break;
    default:
        return RT_DME_RESULT_INVALID;
    }
    return 0;
}

// NTSC-U/C: 0x002af3d0, PAL: 0x002b9340
static int RtDmeVoiceCodeToEvent(int nCode, int *pnEvent) {
    if (pnEvent == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    switch (nCode) {
    case kDeviceCodeStarted:
        *pnEvent = kVoiceEventStarted;
        break;
    case kDeviceCodeStopped:
        *pnEvent = kVoiceEventStopped;
        break;
    case kDeviceCodeSending:
        *pnEvent = kVoiceEventSending;
        break;
    case kDeviceCodeReceiving:
        *pnEvent = kVoiceEventReceiving;
        break;
    case kDeviceCodeIdle:
        *pnEvent = kVoiceEventIdle;
        break;
    default:
        return RT_DME_RESULT_INVALID;
    }
    return 0;
}

// NTSC-U/C: 0x002af460, PAL: 0x002b93d0
static int RtDmeVoiceMapMode(int nMode, int *pnMode) {
    if (pnMode == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    if (nMode == 0) {
        *pnMode = kChannelModeAlternate;
    } else if (nMode == 1) {
        *pnMode = kChannelModeDefault;
    } else {
        return RT_DME_RESULT_INVALID;
    }
    return 0;
}

// NTSC-U/C: 0x002af4a0, PAL: 0x002b9410
static int RtDmeVoiceAnnounce(RtDmeVoice *pVoice, RtDmeVoiceSlot *pSlot) {
    RtDmeVoicePeerMessage message;
    int nResult;

    if (RtDmeVoiceIsReady(pVoice) != 1 || pSlot == NULL || pVoice->bReady != 1) {
        return RT_DME_RESULT_INVALID;
    }
    message.nConnection = pVoice->nConnection;
    message.nClient = pVoice->nLocalClient;
    message.uLocalAddress = pVoice->peer.uLocalAddress;
    message.nLocalPort = pVoice->peer.nLocalPort;
    message.uAddress = pVoice->peer.uAddress;
    message.nPort = pVoice->peer.nPort;
    message.nReserved18 = pVoice->peer.nReserved10;
    nResult = RtDmeSendSystemMessage(kVoiceSendFlags,
                                     pVoice->nConnection,
                                     pSlot->nClient,
                                     g_nDmeVoicePeerMessage,
                                     sizeof(message),
                                     &message);
    if (nResult == 0) {
        pSlot->bAnnounced = pVoice->bReady;
    }
    return nResult;
}

// NTSC-U/C: 0x002af570, PAL: 0x002b94e0
static int RtDmeVoiceOnEvent(RtDmeDevice *pDevice, int *pnEvent, int nValue1, int nValue2) {
    RtDmeVoice *pVoice;
    rtDmeVoiceEvent event;

    pVoice = NULL;
    if (nValue1 == 0 || pnEvent == NULL) {
        return 0;
    }
    if (RtDmeVoiceFindByDevice(pDevice, &pVoice) != 0) {
        return 0;
    }
    event.event = 0;
    event.result = 0;
    if (pVoice->settings.onEvent != NULL && pVoice->bStarted == 1 && pVoice->bReady == 1) {
        event.value1 = nValue1;
        event.value2 = nValue2;
        event.argument = pVoice->settings.eventArgument;
        pVoice->settings.onEvent(&event);
    }
    if (RtDmeVoiceEventToCode(event.event, pnEvent) != 0) {
        return 0;
    }
    return event.result;
}

// NTSC-U/C: 0x002af630, PAL: 0x002b95a0
static int RtDmeVoiceOnState(RtDmeDevice *pDevice, int nState, int nValue1, int nValue2) {
    RtDmeVoice *pVoice;
    rtDmeVoiceState state;
    RtDmeDeviceStatus status;

    pVoice = NULL;
    if (nValue1 == 0) {
        return nValue2;
    }
    if (RtDmeVoiceFindByDevice(pDevice, &pVoice) != 0) {
        return nValue2;
    }
    state.result = nValue2;
    if (pVoice->settings.onState == NULL || pVoice->bStarted != 1 || pVoice->bReady != 1) {
        return state.result;
    }
    if (RtDmeVoiceCodeToEvent(nState, &state.state) != 0) {
        return nValue2;
    }
    // Yes, the binary discards the result of the status.
    (void)RtDmeVoiceGetDeviceStatus(pVoice->pDevice, &status);
    state.level = status.nLevel;
    state.value1 = nValue1;
    state.value2 = nValue2;
    state.argument = pVoice->settings.stateArgument;
    pVoice->settings.onState(&state);
    return state.result;
}

// NTSC-U/C: 0x002af700, PAL: 0x002b9670
static int RtDmeVoiceOnData(RtDmeDevice *pDevice, int nValue1, int nValue2, int nValue3) {
    RtDmeVoice *pVoice;
    rtDmeVoiceData data;

    pVoice = NULL;
    if (nValue1 == 0 || nValue2 == 0 || nValue3 == 0) {
        return -1;
    }
    if (RtDmeVoiceFindByDevice(pDevice, &pVoice) != 0) {
        return -1;
    }
    data.result = 0;
    if (pVoice->settings.onData != NULL && pVoice->bStarted == 1 && pVoice->bReady == 1) {
        data.value1 = nValue1;
        data.argument = pVoice->settings.dataArgument;
        data.value2 = nValue2;
        data.value3 = nValue3;
        pVoice->settings.onData(&data);
    }
    return data.result;
}

// NTSC-U/C: 0x002af7b0, PAL: 0x002b9720
static int RtDmeVoiceOnEnd(RtDmeDevice *pDevice, int nValue1, int nValue2) {
    RtDmeVoice *pVoice;
    rtDmeVoiceEnd end;

    pVoice = NULL;
    if (nValue2 == 0) {
        return -1;
    }
    if (RtDmeVoiceFindByDevice(pDevice, &pVoice) != 0) {
        return -1;
    }
    end.result = 0;
    if (pVoice->settings.onEnd != NULL && pVoice->bStarted == 1 && pVoice->bReady == 1) {
        end.argument = pVoice->settings.endArgument;
        end.value1 = nValue1;
        end.value2 = nValue2;
        pVoice->settings.onEnd(&end);
    }
    return end.result;
}

// NTSC-U/C: 0x002af848, PAL: 0x002b97b8
static int RtDmeVoiceOnStream(RtDmeDevice *pDevice, unsigned int uAddress, int nPort) {
    RtDmeVoice *pVoice;
    RtDmeVoiceSlot *pSlot;
    int nResult;
    int i;

    pVoice = NULL;
    if (RtDmeVoiceFindByDevice(pDevice, &pVoice) != 0) {
        return -1;
    }
    pVoice->endpoint.nPort = nPort;
    rtNetFormatAddress(uAddress, pVoice->endpoint.szAddress);
    if (RtDmeVoiceGetDevicePeer(pVoice->pDevice, &pVoice->peer) != 0) {
        return kVoiceDeviceError;
    }
    pVoice->bStreaming = 1;
    pVoice->bReady = 1;
    for (i = 0; i < pVoice->nClientCount; ++i) {
        nResult = RtDmeVoiceGetSlot(pVoice, i, &pSlot);
        if (nResult != 0) {
            return nResult;
        }
        // The active flag set and the announced flag clear, read as one word.
        if (pSlot->bActive == kSlotStartActive && pSlot->bAnnounced == 0) {
            nResult = RtDmeVoiceAnnounce(pVoice, pSlot);
            if (nResult != 0) {
                return nResult;
            }
        }
    }
    return 0;
}

// NTSC-U/C: 0x002ae990, PAL: 0x002b8900
static int RtDmeVoiceAlloc(int nConnection, unsigned int nClientCount) {
    RtDmeVoice **ppVoice;
    RtDmeVoice *pVoice;

    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (RtDmeIsConnectionIndex(nConnection) != 1 || nConnection >= RT_DME_CONNECTION_COUNT ||
        nClientCount > RT_DME_CLIENT_COUNT || nClientCount == 0) {
        return RT_DME_RESULT_INVALID;
    }
    ppVoice = &g_apDmeVoices[nConnection];
    if (*ppVoice == NULL && RtAllocZeroedBlock((void **)ppVoice, sizeof(RtDmeVoice)) != 0) {
        return kVoiceNotReady;
    }
    pVoice = *ppVoice;
    if (pVoice == NULL) {
        return kVoiceNotReady;
    }
    if (pVoice->pSlots != NULL) {
        RtFreeBlock((void **)&pVoice->pSlots);
    }
    if (RtAllocZeroedBlock((void **)&pVoice->pSlots, nClientCount * sizeof(RtDmeVoiceSlot)) != 0) {
        RtFreeBlock((void **)ppVoice);
        *ppVoice = NULL;
        return kVoiceNotReady;
    }
    if (pVoice->pSlots == NULL) {
        return kVoiceNotReady;
    }
    pVoice->nClientCount = nClientCount;
    pVoice->nConnection = nConnection;
    pVoice->nLocalClient = RT_DME_NO_CLIENT;
    pVoice->bOpen = 0;
    pVoice->bLocalStarted = 0;
    return 0;
}

// NTSC-U/C: 0x002aeae8, PAL: 0x002b8a58
static int RtDmeVoiceFree(int nConnection) {
    RtDmeVoice **ppVoice;

    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (RtDmeIsConnectionIndex(nConnection) != 1 || nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    ppVoice = &g_apDmeVoices[nConnection];
    if (*ppVoice == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    if ((*ppVoice)->pSlots != NULL) {
        RtFreeBlock((void **)&(*ppVoice)->pSlots);
    }
    RtFreeBlock((void **)ppVoice);
    *ppVoice = NULL;
    return 0;
}

int RtDmeVoiceStartup(void) {
    if (g_nDmeVoiceStarts == 0) {
        memset(g_apDmeVoices, 0, sizeof(g_apDmeVoices));
        g_bDmeVoiceStarted = 1;
    }
    ++g_nDmeVoiceStarts;
    return 0;
}

int RtDmeVoiceShutdown(void) {
    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (g_nDmeVoiceStarts == g_bDmeVoiceStarted) {
        g_bDmeVoiceStarted = 0;
    }
    --g_nDmeVoiceStarts;
    return 0;
}

int RtDmeVoiceInitSettings(rtDmeVoiceSettings *pSettings) {
    int nResult;

    if (pSettings == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    memset(pSettings, 0, sizeof(rtDmeVoiceSettings));
    pSettings->enabled = 0;
    nResult = RtDmeVoiceAllocPort(&pSettings->port);
    if (nResult != 0) {
        return nResult;
    }
    pSettings->version = kDefaultVersion;
    pSettings->format[4] = kDefaultBits;
    pSettings->format[5] = kDefaultRate;
    pSettings->channelCount = kDefaultChannelCount;
    pSettings->mode = 0;
    pSettings->format[0] = kDefaultChannels;
    pSettings->format[1] = kDefaultBits;
    pSettings->format[2] = kDefaultRate;
    pSettings->format[3] = kDefaultChannels;
    pSettings->onEvent = NULL;
    pSettings->eventArgument = 0;
    pSettings->onState = NULL;
    pSettings->stateArgument = 0;
    pSettings->device[0] = 0;
    pSettings->device[1] = 0;
    pSettings->onData = NULL;
    pSettings->dataArgument = 0;
    pSettings->onEnd = NULL;
    pSettings->endArgument = 0;
    return 0;
}

int RtDmeVoiceUpdate(int nConnection, unsigned char bStarted) {
    RtDmeVoice *pVoice;
    unsigned char bLayerStarted;
    int nResult;

    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    bLayerStarted = g_bDmeVoiceStarted;
    nResult = RtDmeVoiceGetReady(nConnection, &pVoice);
    if (nResult != 0) {
        return nResult;
    }
    pVoice->bStarted = bStarted;
    nResult = RtDmeVoiceUpdateDevice(pVoice->pDevice);
    // Yes, the binary restores the started flag from the flag of the layer, which is 1 here.
    pVoice->bStarted = bLayerStarted;
    return RtDmeVoiceResult(nResult, kVoiceDeviceError);
}

int RtDmeVoiceOpen(int nConnection, int nMaxClients, const rtDmeVoiceSettings *pSettings) {
    RtDmeVoice *pVoice;
    rtNetAddress *pGateway;
    char szName[RT_NET_ADDRESS_SIZE];
    int nMode;
    int nChannel;
    int nResult;
    unsigned int i;
    int j;

    pGateway = NULL;
    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (pSettings == NULL || (unsigned int)pSettings->channelCount > RT_DME_VOICE_CHANNEL_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeVoiceAlloc(nConnection, nMaxClients);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtDmeVoiceGet(nConnection, &pVoice);
    if (nResult != 0) {
        return nResult;
    }
    if (pVoice == NULL) {
        return kVoiceNotReady;
    }
    if (pVoice->bOpen) {
        return nResult;
    }
    pVoice->pDevice = NULL;
    pVoice->nConnection = nConnection;
    pVoice->bLocalStarted = 0;
    pVoice->nLocalClient = RT_DME_NO_CLIENT;
    pVoice->settings = *pSettings;
    pVoice->bStreaming = 0;
    pVoice->bReady = 0;
    pVoice->uReserved128 = 0;
    pVoice->bStarted = 0;
    pVoice->device.nPort = pVoice->settings.port;
    pVoice->device.anFormat[0] = pVoice->settings.format[0];
    pVoice->device.anFormat[4] = pVoice->settings.format[4];
    pVoice->device.anFormat[1] = pVoice->settings.format[1];
    pVoice->device.anFormat[5] = pVoice->settings.format[5];
    pVoice->device.anFormat[2] = pVoice->settings.format[2];
    pVoice->device.anFormat[3] = pVoice->settings.format[3];
    pVoice->device.anDevice[0] = pSettings->device[0];
    pVoice->device.anDevice[1] = pSettings->device[1];
    if (pSettings->onState != NULL || pSettings->onEvent != NULL) {
        pVoice->device.pfnEvent = RtDmeVoiceOnEvent;
        pVoice->device.pfnState = RtDmeVoiceOnState;
    } else {
        pVoice->device.pfnEvent = NULL;
        pVoice->device.pfnState = NULL;
    }
    if (pSettings->onData != NULL || pSettings->onEnd != NULL) {
        pVoice->device.pfnData = RtDmeVoiceOnData;
        pVoice->device.pfnEnd = RtDmeVoiceOnEnd;
    } else {
        pVoice->device.pfnData = NULL;
        pVoice->device.pfnEnd = NULL;
    }
    if (pVoice->pSlots == NULL) {
        return kVoiceNotReady;
    }
    for (j = 0; j < pVoice->nClientCount; ++j) {
        pVoice->pSlots[j].nClient = j;
        pVoice->pSlots[j].bActive = 0;
        pVoice->pSlots[j].bAdded = 0;
        pVoice->pSlots[j].bAnnounced = 0;
    }
    nResult = RtDmeVoiceResult(RtDmeVoiceCreateDevice(&pVoice->pDevice, &pVoice->device),
                               kVoiceDeviceError);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtDmeVoiceMapMode(pVoice->settings.mode, &nMode);
    if (nResult != 0) {
        return nResult;
    }
    for (i = 0; i < (unsigned int)pVoice->settings.channelCount; ++i) {
        sprintf(szName, "%d", i);
        nResult = RtDmeVoiceResult(RtDmeVoiceOpenChannel(pVoice->pDevice, nMode, &nChannel, szName),
                                   kVoiceDeviceError);
        if (nResult != 0) {
            return nResult;
        }
        pVoice->anChannels[i] = nChannel;
    }
    // Yes, the binary selects the last channel opened, which is unset when none was.
    nResult =
        RtDmeVoiceResult(RtDmeVoiceSelectChannel(pVoice->pDevice, nChannel), kVoiceDeviceError);
    if (nResult != 0) {
        return nResult;
    }
    pVoice->bOpen = 1;
    nResult = RtDmeGetGatewayAddress(&pGateway);
    if (nResult != 0) {
        return nResult;
    }
    if (pGateway == NULL) {
        return RT_DME_RESULT_NO_RECORD;
    }
    if (pGateway->type == kGatewayRelayed) {
        pVoice->bStreaming = 0;
        nResult = RtDmeVoiceResult(
            RtDmeVoiceConnectDevice(
                pVoice->pDevice, pGateway->address, pGateway->port, RtDmeVoiceOnStream),
            kVoiceDeviceError);
        if (nResult != 0) {
            return nResult;
        }
    } else {
        pVoice->bReady = 1;
    }
    nResult = RtDmeVoiceResult(RtDmeVoiceGetDevicePeer(pVoice->pDevice, &pVoice->peer),
                               kVoiceDeviceError);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtDmeVoiceMakeEndpoint(&pVoice->peer, &pVoice->endpoint);
    if (nResult != 0) {
        return nResult;
    }
    pVoice->bStarted = 1;
    return nResult;
}

int RtDmeVoiceClose(int nConnection) {
    RtDmeVoice *pVoice;
    int nResult;

    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    nResult = RtDmeVoiceGetReady(nConnection, &pVoice);
    if (nResult != 0) {
        return nResult;
    }
    RtDmeVoiceStopDevice(pVoice->pDevice);
    RtDmeVoiceDestroyDevice(pVoice->pDevice);
    pVoice->bOpen = 0;
    pVoice->bLocalStarted = 0;
    pVoice->nLocalClient = RT_DME_NO_CLIENT;
    pVoice->pDevice = NULL;
    pVoice->bStarted = 0;
    // Yes, the binary discards the result of the release.
    (void)RtDmeVoiceFree(nConnection);
    return 0;
}

int RtDmeVoiceStart(int nConnection, int nClient) {
    RtDmeVoice *pVoice;
    RtDmeVoiceSlot *pSlot;
    int bIsClient;
    int nResult;

    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    bIsClient = RtDmeIsClientIndex(nClient);
    if (bIsClient != 1) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeVoiceGetReady(nConnection, &pVoice);
    if (nResult != 0) {
        return nResult;
    }
    nResult =
        RtDmeVoiceResult(RtDmeVoiceSetDeviceClient(pVoice->pDevice, nClient), kVoiceDeviceError);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtDmeVoiceResult(RtDmeVoiceGetDevicePeer(pVoice->pDevice, &pVoice->peer),
                               kVoiceDeviceError);
    if (nResult != 0) {
        return nResult;
    }
    if (pVoice->peer.nClient != nClient) {
        return kVoiceNotReady;
    }
    pVoice->bLocalStarted = bIsClient;
    pVoice->nLocalClient = nClient;
    nResult = RtDmeVoiceGetSlot(pVoice, nClient, &pSlot);
    if (nResult != 0) {
        return nResult;
    }
    pSlot->bActive = bIsClient;
    pSlot->bAdded = bIsClient;
    pSlot->bAnnounced = bIsClient;
    return nResult;
}

int RtDmeVoiceIsOpen(int nConnection) {
    RtDmeVoice *pVoice;
    int bIsConnection;

    if (!RtDmeVoiceIsStarted()) {
        return 0;
    }
    bIsConnection = RtDmeIsConnectionIndex(nConnection);
    if (bIsConnection != 1) {
        return 0;
    }
    if (RtDmeVoiceGet(nConnection, &pVoice) != 0 || pVoice == NULL) {
        return 0;
    }
    return pVoice->bOpen == bIsConnection;
}

int RtDmeVoiceAddClient(int nConnection, int nClient) {
    RtDmeVoice *pVoice;
    RtDmeVoiceSlot *pSlot;
    int bIsClient;
    int nResult;

    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    bIsClient = RtDmeIsClientIndex(nClient);
    if (bIsClient != 1) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeVoiceGetReady(nConnection, &pVoice);
    if (nResult != 0) {
        return nResult;
    }
    if (pVoice->bLocalStarted != bIsClient) {
        return kVoiceNotReady;
    }
    nResult = RtDmeVoiceResult(RtDmeVoiceGetDevicePeer(pVoice->pDevice, &pVoice->peer),
                               kVoiceDeviceError);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtDmeVoiceGetSlot(pVoice, nClient, &pSlot);
    if (nResult != 0) {
        return nResult;
    }
    pSlot->bActive = 1;
    if (pVoice->bReady != 1) {
        return 0;
    }
    nResult = RtDmeVoiceAnnounce(pVoice, pSlot);
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

int RtDmeVoiceRemoveClient(int nConnection, int nClient) {
    RtDmeVoice *pVoice;
    RtDmeVoiceSlot *pSlot;
    int bIsClient;
    int nResult;

    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    bIsClient = RtDmeIsClientIndex(nClient);
    if (bIsClient != 1) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeVoiceGetReady(nConnection, &pVoice);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtDmeVoiceGetSlot(pVoice, nClient, &pSlot);
    if (nResult != 0) {
        return nResult;
    }
    if (pSlot->bAdded == bIsClient) {
        // Yes, the binary discards the result of removing the peer.
        (void)RtDmeVoiceRemoveDevicePeer(pVoice->pDevice, nClient);
        pSlot->bAdded = 0;
    }
    pSlot->bActive = 0;
    pSlot->bAnnounced = 0;
    return 0;
}

int RtDmeVoiceAddPeer(const RtDmeVoicePeerMessage *pMessage) {
    RtDmeVoice *pVoice;
    RtDmeVoiceSlot *pSlot;
    RtDmeDevicePeer peer;
    RtDmeVoiceEndpoint endpoint;
    int bIsClient;
    int nResult;

    if (!RtDmeVoiceIsStarted()) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (pMessage == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeVoiceGetReady(pMessage->nConnection, &pVoice);
    if (nResult != 0) {
        return nResult;
    }
    bIsClient = RtDmeIsClientIndex(pMessage->nClient);
    if (bIsClient != 1) {
        return kVoiceNotReady;
    }
    nResult = RtDmeVoiceGetSlot(pVoice, pMessage->nClient, &pSlot);
    if (nResult != 0) {
        return nResult;
    }
    memset(&peer, 0, sizeof(peer));
    peer.uLocalAddress = pMessage->uLocalAddress;
    peer.nLocalPort = pMessage->nLocalPort;
    peer.uAddress = pMessage->uAddress;
    peer.nPort = pMessage->nPort;
    peer.nReserved10 = pMessage->nReserved18;
    peer.nClient = pMessage->nClient;
    nResult = RtDmeVoiceMakeEndpoint(&peer, &endpoint);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtDmeVoiceResult(RtDmeVoiceAddDevicePeer(pVoice->pDevice, &peer), kVoiceDeviceError);
    if (nResult != 0) {
        return nResult;
    }
    pSlot->bAdded = bIsClient;
    return nResult;
}
