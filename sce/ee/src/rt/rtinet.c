#include "rt/rtinet.h"

#include <stdint.h>
#include <string.h>

#include <libnet.h>
#include <msifrpc.h>

#include "rt/rttime.h"

// Error codes of the driver.
enum {
    kErrorFailed = 1,
    kErrorArgument = 2,
    kErrorSocketType = 3,
    kErrorStarted = 4,
    kErrorNotStarted = 5,
    kErrorStreamClosed = 13,
};

enum {
    // The arguments of sceLibnetInitialize().
    kClientBufferSize = 2048,
    kClientStackSize = 0x2000,
    kClientPriority = 32,
    kRpcAlign = 64,
    kMaxInterfaces = 32,
    // The interface control codes.
    kInterfaceControlGetFlags = 8,
    // Inferred from the second address the driver reads after the local one.
    kInterfaceControlGetBroadcastAddress = 11,
    // An interface carries traffic once both of these flags are set.
    kInterfaceFlagsReady = 3,
    // The milliseconds rtNetInetInit() waits for an interface, and the pause between two checks.
    kInterfaceTimeout = 95000,
    kInterfacePollInterval = 2,
    // sceLibnetWaitGetAddress() uses an interface already up with this flag.
    kWaitGetAddressUseUp = 2,
    // The timeouts of the connection calls.
    kOpenTimeout = 10000,
    kCloseTimeout = 500,
    kReceiveTimeout = 2,
    // The result of the module when a call timed out.
    kInetErrorTimeout = -500,
    // The protocol of a raw connection, ICMP.
    kRawProtocol = 1,
    // A connection control code applied with the value 1 to stream connections.
    kControlStreamOption = 3,
    kControlStreamOptionValue = 1,
    // The local port of a connection that connects, any free port.
    kAnyLocalPort = -1,
    kMaxReceiveSize = 4096,
    // Bit of the flags of a stream read set once the peer closed the stream.
    kStreamFlagClosed = 4,
};

// NTSC-U/C: 0x003c2550, PAL: 0x00441410
static bool g_bNetworkManagerInit;

// NTSC-U/C: 0x003c2558, PAL: 0x00441418
static char g_szNetworkLocalAddress[20];

// NTSC-U/C: 0x003c2570, PAL: 0x00441430
static char g_szNetworkBroadcastAddress[20];

// NTSC-U/C: 0x003c2584, PAL: 0x00441444
static int g_nNetworkInterfaceId = -1;

// NTSC-U/C: 0x00529080
static int g_anNetworkInterfaceIds[kMaxInterfaces];

// NTSC-U/C: 0x00529100
static sceSifMClientData g_NetworkClient __attribute__((aligned(kRpcAlign)));

// NTSC-U/C: 0x00529140
static int g_anNetworkRpcBuffer[kClientBufferSize / sizeof(int)]
    __attribute__((aligned(kRpcAlign)));

// NTSC-U/C: 0x002f8ef8, PAL: 0x00302e68
static int rtNetInetWaitForInterface(void) {
    RtTime start;
    unsigned int elapsed = 0;
    int flags;
    int count;
    int i;
    int interfaceId;
    bool found = false;

    count = sceInetGetInterfaceList(
        &g_NetworkClient, g_anNetworkRpcBuffer, g_anNetworkInterfaceIds, kMaxInterfaces);
    if (count <= 0) {
        return kErrorFailed;
    }
    if (RtReadTime(&start) != 0) {
        return kErrorFailed;
    }
    for (;;) {
        for (i = 0; i < count; ++i) {
            interfaceId = g_anNetworkInterfaceIds[i];
            if (sceInetInterfaceControl(&g_NetworkClient,
                                        g_anNetworkRpcBuffer,
                                        interfaceId,
                                        kInterfaceControlGetFlags,
                                        &flags,
                                        sizeof(flags)) != 0) {
                return kErrorFailed;
            }
            if ((flags & kInterfaceFlagsReady) == kInterfaceFlagsReady) {
                found = true;
                g_nNetworkInterfaceId = interfaceId;
                break;
            }
        }
        if (found) {
            return 0;
        }
        if (RtGetMillisecondsSince(&start, &elapsed) != 0) {
            return kErrorFailed;
        }
        if (elapsed > kInterfaceTimeout) {
            return kErrorFailed;
        }
        if (RtSpinMilliseconds(kInterfacePollInterval) != 0) {
            return kErrorFailed;
        }
    }
}

// NTSC-U/C: 0x002f9058, PAL: 0x00302fc8
static int rtNetInetReadLocalAddress(char *text) {
    sceInetAddress address;
    int interfaceId;

    if (text == NULL) {
        return kErrorArgument;
    }
    memset(&address, 0, sizeof(address));
    if (sceLibnetWaitGetAddress(&g_NetworkClient,
                                g_anNetworkRpcBuffer,
                                &interfaceId,
                                1,
                                &address,
                                kWaitGetAddressUseUp) < 0) {
        return kErrorFailed;
    }
    return LibnetAddressToString(text, &address) != 0;
}

// NTSC-U/C: 0x002f90d8, PAL: 0x00303048
static int rtNetInetReadBroadcastAddress(char *text) {
    sceInetAddress address;

    if (text == NULL) {
        return kErrorArgument;
    }
    if (g_nNetworkInterfaceId < 0) {
        return kErrorFailed;
    }
    memset(&address, 0, sizeof(address));
    if (sceInetInterfaceControl(&g_NetworkClient,
                                g_anNetworkRpcBuffer,
                                g_nNetworkInterfaceId,
                                kInterfaceControlGetBroadcastAddress,
                                &address,
                                sizeof(address)) != 0) {
        return kErrorFailed;
    }
    return LibnetAddressToString(text, &address) != 0;
}

int rtNetInetInit(void) {
    int result;

    if (g_bNetworkManagerInit) {
        return kErrorStarted;
    }
    sceSifMInitRpc(0);
    // Yes, the binary discards the result of the bind.
    sceLibnetInitialize(&g_NetworkClient, kClientBufferSize, kClientStackSize, kClientPriority);
    if (sceLibnetRegisterHandler(&g_NetworkClient, g_anNetworkRpcBuffer) < 0) {
        return kErrorFailed;
    }
    if ((result = rtNetInetWaitForInterface()) != 0) {
        return result;
    }
    if (g_nNetworkInterfaceId < 0) {
        return kErrorFailed;
    }
    if ((result = rtNetInetReadLocalAddress(g_szNetworkLocalAddress)) != 0) {
        return result;
    }
    if ((result = rtNetInetReadBroadcastAddress(g_szNetworkBroadcastAddress)) != 0) {
        return result;
    }
    g_bNetworkManagerInit = true;
    return 0;
}

int rtNetInetShutdown(void) {
    if (!g_bNetworkManagerInit) {
        return kErrorNotStarted;
    }
    if (sceLibnetUnregisterHandler(&g_NetworkClient, g_anNetworkRpcBuffer) < 0) {
        return kErrorFailed;
    }
    // Yes, the binary discards the result of the unbind.
    sceLibnetTerminate(&g_NetworkClient);
    g_bNetworkManagerInit = false;
    g_nNetworkInterfaceId = -1;
    return 0;
}

int rtNetInetOpen(LibnetAsyncHandle **connection,
                  const char *address,
                  unsigned short port,
                  const RtSocketOptions *options) {
    LibnetAsyncParam param;
    LibnetAsyncHandle *handle = NULL;
    int controlValue;
    int timeout;
    int result;

    if (connection == NULL) {
        return kErrorArgument;
    }
    *connection = NULL;
    if (!g_bNetworkManagerInit) {
        return kErrorNotStarted;
    }
    if (options == NULL) {
        return kErrorArgument;
    }
    if (options->nType == RT_SOCKET_STREAM && address == NULL) {
        return kErrorArgument;
    }
    if (options->nType != RT_SOCKET_LISTEN && port == 0) {
        return kErrorArgument;
    }
    memset(&param, 0, sizeof(param));
    if (options->nType == RT_SOCKET_STREAM) {
        if (address == NULL) {
            return kErrorArgument;
        }
        param.inet.localPort = kAnyLocalPort;
        param.inet.remotePort = port;
        param.inet.type = sceINETT_CONNECT;
        timeout = kOpenTimeout;
        if (sceInetName2Address(&g_NetworkClient,
                                g_anNetworkRpcBuffer,
                                0,
                                &param.inet.remoteAddress,
                                address,
                                0,
                                0) < 0) {
            return kErrorFailed;
        }
    } else if (options->nType == RT_SOCKET_DATAGRAM) {
        if (address != NULL) {
            return kErrorArgument;
        }
        timeout = kOpenTimeout;
        if (options->nBufferSize <= 0) {
            return kErrorArgument;
        }
        param.inet.localPort = port;
        param.maxPacketSize = options->nBufferSize;
        param.inet.remotePort = 0;
        param.inet.type = sceINETT_DGRAM;
    } else if (options->nType == RT_SOCKET_LISTEN) {
        if (address != NULL) {
            return kErrorArgument;
        }
        param.inet.localPort = kRawProtocol;
        param.inet.type = sceINETT_RAW;
        timeout = kOpenTimeout;
        param.inet.remotePort = 0;
        // Yes, the binary resolves a null name for a raw connection.
        if (sceInetName2Address(
                &g_NetworkClient, g_anNetworkRpcBuffer, 0, &param.inet.remoteAddress, NULL, 0, 0) <
            0) {
            return kErrorFailed;
        }
    } else if (options->nType == RT_SOCKET_STREAM_ACCEPTED) {
        if (address != NULL) {
            return kErrorArgument;
        }
        param.inet.localPort = port;
        param.inet.type = sceINETT_LISTEN;
        timeout = 0;
        param.inet.remotePort = 0;
        // Yes, the binary resolves a null name for a waiting stream connection.
        if (sceInetName2Address(
                &g_NetworkClient, g_anNetworkRpcBuffer, 0, &param.inet.remoteAddress, NULL, 0, 0) <
            0) {
            return kErrorFailed;
        }
    } else {
        return kErrorSocketType;
    }

    if (LibnetAsyncCreate(&g_NetworkClient, g_anNetworkRpcBuffer, &handle, &param) != 0) {
        return kErrorFailed;
    }
    result = LibnetAsyncOpen(&g_NetworkClient, g_anNetworkRpcBuffer, handle, timeout);
    if (result != 0 &&
        (result != kInetErrorTimeout || options->nType != RT_SOCKET_STREAM_ACCEPTED)) {
        LibnetAsyncClose(&g_NetworkClient, g_anNetworkRpcBuffer, handle, kCloseTimeout);
        return kErrorFailed;
    }
    if (options->nType == RT_SOCKET_STREAM || options->nType == RT_SOCKET_STREAM_ACCEPTED) {
        controlValue = kControlStreamOptionValue;
        if (LibnetAsyncControl(&g_NetworkClient,
                               g_anNetworkRpcBuffer,
                               handle,
                               kControlStreamOption,
                               &controlValue,
                               sizeof(controlValue)) != 0) {
            return kErrorFailed;
        }
    }
    *connection = handle;
    return 0;
}

int rtNetInetClose(LibnetAsyncHandle *connection) {
    if (!g_bNetworkManagerInit) {
        return kErrorNotStarted;
    }
    // Yes, the binary tests the connection pointer as a signed value.
    if ((intptr_t)connection < 0) {
        return kErrorArgument;
    }
    return LibnetAsyncClose(&g_NetworkClient, g_anNetworkRpcBuffer, connection, kCloseTimeout) != 0;
}

int rtNetInetSendStream(LibnetAsyncHandle *connection,
                        const void *data,
                        unsigned short length,
                        unsigned short *sent) {
    int result;

    if (!g_bNetworkManagerInit) {
        return kErrorNotStarted;
    }
    // Yes, the binary tests the connection pointer as a signed value.
    if ((intptr_t)connection < 0 || data == NULL || sent == NULL) {
        return kErrorArgument;
    }
    result =
        LibnetAsyncWriteStream(&g_NetworkClient, g_anNetworkRpcBuffer, connection, data, length);
    if (result < 0) {
        return kErrorFailed;
    }
    *sent = result;
    return 0;
}

int rtNetInetSendTo(LibnetAsyncHandle *connection,
                    const char *address,
                    unsigned short port,
                    const void *data,
                    unsigned short length,
                    unsigned short *sent) {
    sceInetAddress receiver;
    int flags = 0;
    int result;

    if (!g_bNetworkManagerInit) {
        return kErrorNotStarted;
    }
    // Yes, the binary tests the connection pointer as a signed value.
    if ((intptr_t)connection < 0 || address == NULL || port == 0 || data == NULL || sent == NULL) {
        return kErrorArgument;
    }
    *sent = 0;
    if (LibnetStringToAddress(&receiver, address) < 0) {
        return kErrorFailed;
    }
    result = LibnetAsyncSendTo(&g_NetworkClient,
                               g_anNetworkRpcBuffer,
                               connection,
                               data,
                               length,
                               &flags,
                               &receiver,
                               port,
                               0);
    if (result < 0) {
        return kErrorFailed;
    }
    *sent = result;
    return 0;
}

int rtNetInetSendMarker(LibnetAsyncHandle *connection,
                        bool *sent,
                        const char *address,
                        unsigned short marker) {
    sceInetAddress receiver;
    int result;

    if (sent == NULL) {
        return kErrorArgument;
    }
    *sent = false;
    if (!g_bNetworkManagerInit) {
        return kErrorNotStarted;
    }
    // Yes, the binary tests the connection pointer as a signed value.
    if ((intptr_t)connection < 0 || address == NULL) {
        return kErrorArgument;
    }
    if (LibnetStringToAddress(&receiver, address) < 0) {
        return kErrorFailed;
    }
    result = LibnetAsyncSendMarker(
        &g_NetworkClient, g_anNetworkRpcBuffer, connection, marker, &receiver);
    if (result < 0) {
        return kErrorFailed;
    }
    if (result > 0) {
        *sent = true;
    }
    return 0;
}

int rtNetInetReceiveStream(LibnetAsyncHandle *connection,
                           void *buffer,
                           unsigned short size,
                           unsigned short *received) {
    int flags = 0;
    int result;

    if (!g_bNetworkManagerInit) {
        return kErrorNotStarted;
    }
    // Yes, the binary tests the connection pointer as a signed value.
    if ((intptr_t)connection < 0 || buffer == NULL || received == NULL) {
        return kErrorArgument;
    }
    result = LibnetAsyncReadStream(&g_NetworkClient,
                                   g_anNetworkRpcBuffer,
                                   connection,
                                   buffer,
                                   size > kMaxReceiveSize ? kMaxReceiveSize : size,
                                   &flags);
    if (result < 0) {
        return kErrorFailed;
    }
    if ((flags & kStreamFlagClosed) != 0) {
        return kErrorStreamClosed;
    }
    *received = result;
    return 0;
}

int rtNetInetReceiveFrom(LibnetAsyncHandle *connection,
                         char *address,
                         unsigned short *port,
                         void *buffer,
                         unsigned short size,
                         unsigned short *received) {
    sceInetAddress sender;
    int flags = 0;
    int senderPort;
    int result;

    if (!g_bNetworkManagerInit) {
        return kErrorNotStarted;
    }
    // Yes, the binary tests the connection pointer as a signed value.
    if ((intptr_t)connection < 0 || address == NULL || port == NULL || buffer == NULL ||
        received == NULL) {
        return kErrorArgument;
    }
    *received = 0;
    result = LibnetAsyncRecvFrom(&g_NetworkClient,
                                 g_anNetworkRpcBuffer,
                                 connection,
                                 buffer,
                                 size > kMaxReceiveSize ? kMaxReceiveSize : size,
                                 &flags,
                                 &sender,
                                 &senderPort,
                                 kReceiveTimeout);
    if (result < 0) {
        if (result != kInetErrorTimeout) {
            return kErrorFailed;
        }
        result = 0;
    }
    if (result > 0) {
        *port = senderPort;
        if (LibnetAddressToString(address, &sender) != 0) {
            return kErrorFailed;
        }
    }
    *received = result;
    return 0;
}

int rtNetInetReceiveMarker(LibnetAsyncHandle *connection,
                           bool *received,
                           char *address,
                           unsigned short *marker) {
    sceInetAddress sender;
    unsigned short value;
    int result;

    if (received == NULL) {
        return kErrorArgument;
    }
    *received = false;
    if (!g_bNetworkManagerInit) {
        return kErrorNotStarted;
    }
    // Yes, the binary tests the connection pointer as a signed value.
    if ((intptr_t)connection < 0 || address == NULL) {
        return kErrorArgument;
    }
    result =
        LibnetAsyncRecvMarker(&g_NetworkClient, g_anNetworkRpcBuffer, connection, &value, &sender);
    if (result < 0) {
        if (result != kInetErrorTimeout) {
            return kErrorFailed;
        }
        result = 0;
    }
    if (result <= 0) {
        return 0;
    }
    *received = true;
    *marker = value;
    return LibnetAddressToString(address, &sender) != 0;
}

int rtNetInetCopyLocalAddress(char *address) {
    if (!g_bNetworkManagerInit) {
        return kErrorNotStarted;
    }
    if (address == NULL) {
        return kErrorArgument;
    }
    strcpy(address, g_szNetworkLocalAddress);
    return 0;
}
