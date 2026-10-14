#include <stdbool.h>
#include <stddef.h>

#include <librtdmedev.h>
#include <librtnet.h>

// Error codes of the socket layer.
enum {
    kErrorFailed = 1,
    kErrorArgument = 2,
    kErrorNotStarted = 4,
    kErrorNoMemory = 9,
    kErrorNullSocket = 10,
    kErrorBadSocket = 11,
    kErrorSocketType = 12,
};

enum {
    // The bytes of the IP and TCP headers the statistics add to each stream transfer.
    kStreamOverhead = 40,
    // The bytes of the IP and UDP headers the statistics add to each datagram.
    kDatagramOverhead = 28,
    // The milliseconds between two updates of the transfer rates.
    kStatsInterval = 1000,
};

static const float kMillisecondsPerSecond = 1000.0f;

// A transfer counter with its rate per second.
typedef struct {
    unsigned int count; // The transfers since the last update of the rate.
    float rate;         // The transfers per second over the last interval.
    float peak;         // The highest rate.
} rtNetRateCounter;

typedef struct {
    rtNetTime resetTime; // The start of the current interval.
    rtNetRateCounter packetsSent;
    rtNetRateCounter bytesSent;
    rtNetRateCounter packetsReceived;
    rtNetRateCounter bytesReceived;
} rtNetStats;

// NTSC-U/C: 0x003bfa50
static int g_nNetSocketUsers = 0;

// NTSC-U/C: 0x00528e18
static rtNetStats g_netStats;

// NTSC-U/C: 0x00528e50
static rtNetTime g_qwNetStatsLastSampleTime;

// NTSC-U/C: 0x002f0238, PAL: 0x002fa1a8
static int rtNetDriverInit(void) {
    return rtNetInetInit();
}

// NTSC-U/C: 0x002f0250, PAL: 0x002fa1c0
static int rtNetDriverShutdown(void) {
    return rtNetInetShutdown();
}

// NTSC-U/C: 0x002f0268, PAL: 0x002fa1d8
static int rtNetDriverOpen(LibnetAsyncHandle **connection,
                           const char *address,
                           unsigned short port,
                           const rtNetSocketOptions *options) {
    return rtNetInetOpen(connection, address, port, options);
}

// NTSC-U/C: 0x002f0280, PAL: 0x002fa1f0
static int rtNetDriverClose(LibnetAsyncHandle *connection) {
    return rtNetInetClose(connection);
}

// NTSC-U/C: 0x002f0298, PAL: 0x002fa208
static int rtNetDriverSendStream(LibnetAsyncHandle *connection,
                                 const void *data,
                                 unsigned short length,
                                 unsigned short *sent) {
    return rtNetInetSendStream(connection, data, length, sent);
}

// NTSC-U/C: 0x002f02b0, PAL: 0x002fa220
static int rtNetDriverSendTo(LibnetAsyncHandle *connection,
                             const char *address,
                             unsigned short port,
                             const void *data,
                             unsigned short length,
                             unsigned short *sent) {
    return rtNetInetSendTo(connection, address, port, data, length, sent);
}

// NTSC-U/C: 0x002f02d0, PAL: 0x002fa240
static int rtNetDriverListen(LibnetAsyncHandle *connection,
                             bool *listening,
                             const char *address,
                             unsigned short port) {
    return rtNetInetSendMarker(connection, listening, address, port);
}

// NTSC-U/C: 0x002f02e8, PAL: 0x002fa258
static int rtNetDriverReceiveStream(LibnetAsyncHandle *connection,
                                    void *buffer,
                                    unsigned short size,
                                    unsigned short *received) {
    return rtNetInetReceiveStream(connection, buffer, size, received);
}

// NTSC-U/C: 0x002f0300, PAL: 0x002fa270
static int rtNetDriverReceiveFrom(LibnetAsyncHandle *connection,
                                  char *address,
                                  unsigned short *port,
                                  void *buffer,
                                  unsigned short size,
                                  unsigned short *received) {
    return rtNetInetReceiveFrom(connection, address, port, buffer, size, received);
}

// NTSC-U/C: 0x002f0318, PAL: 0x002fa288
static int rtNetDriverAccept(LibnetAsyncHandle *connection,
                             bool *accepted,
                             char *address,
                             unsigned short *port) {
    return rtNetInetReceiveMarker(connection, accepted, address, port);
}

// NTSC-U/C: 0x002f0338, PAL: 0x002fa2a8
static int rtNetDriverCopyLocalAddress(char *address) {
    return rtNetInetCopyLocalAddress(address);
}

// NTSC-U/C: 0x002efe90, PAL: 0x002f9e00
static int rtNetSocketAlloc(rtNetSocket **socket) {
    rtNetSocket *block = NULL;

    if (socket == NULL) {
        return kErrorArgument;
    }
    *socket = NULL;
    if (rtDmeAllocZeroed((void **)&block, sizeof(rtNetSocket)) != 0) {
        return kErrorNoMemory;
    }
    if (block == NULL) {
        return kErrorNullSocket;
    }
    block->allocated = 1;
    *socket = block;
    return 0;
}

// NTSC-U/C: 0x002eff00, PAL: 0x002f9e70
static int rtNetSocketFree(rtNetSocket *socket) {
    void *block = socket;

    if (socket == NULL) {
        return kErrorArgument;
    }
    return rtDmeFree(&block) != 0;
}

// NTSC-U/C: 0x002eff30, PAL: 0x002f9ea0
static int rtNetSocketLookup(rtNetSocket *socket, rtNetSocket **entry) {
    if (entry == NULL) {
        return kErrorArgument;
    }
    *entry = socket;
    return 0;
}

// NTSC-U/C: 0x002eff98, PAL: 0x002f9f08
static bool rtNetSocketIsValid(const rtNetSocket *socket) {
    return socket != NULL && socket->allocated != 0 && socket->open != 0 &&
           (unsigned int)socket->options.type < RT_NET_SOCKET_TYPE_COUNT;
}

// NTSC-U/C: 0x002eff48, PAL: 0x002f9eb8
static int rtNetSocketResolve(rtNetSocket *socket, rtNetSocket **entry) {
    if (entry == NULL) {
        return kErrorArgument;
    }
    if (rtNetSocketLookup(socket, entry) != 0) {
        return kErrorBadSocket;
    }
    return rtNetSocketIsValid(*entry) ? 0 : kErrorBadSocket;
}

// NTSC-U/C: 0x002effc8, PAL: 0x002f9f38
static int rtNetSocketOptionsReset(rtNetSocketOptions *options) {
    if (options == NULL) {
        return kErrorArgument;
    }
    // Yes, the binary sets only the kind. The buffer size stays as the allocator cleared it.
    options->type = RT_NET_SOCKET_STREAM;
    return 0;
}

// NTSC-U/C: 0x002f0188, PAL: 0x002fa0f8
static int rtNetRateCounterUpdate(rtNetRateCounter *counter, unsigned int elapsed) {
    if (counter == NULL) {
        return kErrorArgument;
    }
    if (elapsed == 0) {
        counter->rate = 0.0f;
    } else {
        counter->rate = (float)counter->count / ((float)elapsed / kMillisecondsPerSecond);
    }
    if (counter->peak < counter->rate) {
        counter->peak = counter->rate;
    }
    counter->count = 0;
    return 0;
}

// NTSC-U/C: 0x002effe0, PAL: 0x002f9f50
static int rtNetStatsReset(void) {
    g_netStats.packetsSent.count = 0;
    g_netStats.packetsSent.rate = 0.0f;
    g_netStats.packetsSent.peak = 0.0f;
    g_netStats.bytesSent.count = 0;
    g_netStats.bytesSent.rate = 0.0f;
    g_netStats.bytesSent.peak = 0.0f;
    g_netStats.packetsReceived.count = 0;
    g_netStats.packetsReceived.rate = 0.0f;
    g_netStats.packetsReceived.peak = 0.0f;
    g_netStats.bytesReceived.count = 0;
    g_netStats.bytesReceived.rate = 0.0f;
    g_netStats.bytesReceived.peak = 0.0f;
    if (rtNetTimeNow(&g_netStats.resetTime) != 0) {
        return kErrorFailed;
    }
    g_qwNetStatsLastSampleTime = g_netStats.resetTime;
    return 0;
}

// NTSC-U/C: 0x002f00e0, PAL: 0x002fa050
static int rtNetStatsUpdate(void) {
    unsigned int elapsed;
    int result;

    if (rtNetTimeElapsedMs(&g_netStats.resetTime, &elapsed) != 0) {
        return kErrorFailed;
    }
    if (elapsed <= kStatsInterval) {
        return 0;
    }
    if ((result = rtNetRateCounterUpdate(&g_netStats.packetsSent, elapsed)) != 0) {
        return result;
    }
    if ((result = rtNetRateCounterUpdate(&g_netStats.bytesSent, elapsed)) != 0) {
        return result;
    }
    if ((result = rtNetRateCounterUpdate(&g_netStats.packetsReceived, elapsed)) != 0) {
        return result;
    }
    if ((result = rtNetRateCounterUpdate(&g_netStats.bytesReceived, elapsed)) != 0) {
        return result;
    }
    if (rtNetTimeNow(&g_netStats.resetTime) != 0) {
        return kErrorFailed;
    }
    return 0;
}

// NTSC-U/C: 0x002f0068, PAL: 0x002f9fd8
static int rtNetStatsRecord(unsigned int sent, unsigned int received, unsigned int overhead) {
    if (sent != 0) {
        ++g_netStats.packetsSent.count;
        g_netStats.bytesSent.count += sent + overhead;
    }
    if (received != 0) {
        ++g_netStats.packetsReceived.count;
        g_netStats.bytesReceived.count += received + overhead;
    }
    return rtNetStatsUpdate();
}

int rtNetSocketInit(void) {
    int result;

    if (g_nNetSocketUsers == 0) {
        if ((result = rtNetDriverInit()) != 0) {
            return result;
        }
        if ((result = rtNetStatsReset()) != 0) {
            return result;
        }
    }
    ++g_nNetSocketUsers;
    return 0;
}

int rtNetSocketShutdown(void) {
    if (g_nNetSocketUsers <= 0) {
        return kErrorNotStarted;
    }
    if (--g_nNetSocketUsers != 0) {
        return 0;
    }
    return rtNetDriverShutdown();
}

int rtNetSocketCreate(rtNetSocket **socket,
                      const char *address,
                      unsigned short port,
                      const rtNetSocketOptions *options) {
    rtNetSocket *created;
    rtNetSocket *entry = NULL;
    int result;

    if (socket == NULL) {
        return kErrorArgument;
    }
    *socket = NULL;
    if ((result = rtNetSocketAlloc(&created)) != 0) {
        return result;
    }
    if ((result = rtNetSocketLookup(created, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (options != NULL) {
        entry->options = *options;
    } else if ((result = rtNetSocketOptionsReset(&entry->options)) != 0) {
        return result;
    }
    if ((result = rtNetDriverOpen(&entry->connection, address, port, &entry->options)) != 0) {
        return result;
    }
    entry->port = port;
    entry->open = 1;
    *socket = created;
    return 0;
}

int rtNetSocketClose(rtNetSocket *socket) {
    rtNetSocket *entry = NULL;
    int result;

    if ((result = rtNetSocketResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if ((result = rtNetDriverClose(entry->connection)) != 0) {
        return result;
    }
    entry->open = 0;
    entry->allocated = 0;
    return rtNetSocketFree(socket);
}

int rtNetSocketSendStream(rtNetSocket *socket,
                          const void *data,
                          unsigned short length,
                          unsigned short *sent) {
    rtNetSocket *entry = NULL;
    int result;

    if (sent == NULL) {
        return kErrorArgument;
    }
    *sent = 0;
    if (data == NULL) {
        return kErrorArgument;
    }
    if ((result = rtNetSocketResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (entry->options.type != RT_NET_SOCKET_STREAM &&
        entry->options.type != RT_NET_SOCKET_STREAM_ACCEPTED) {
        return kErrorSocketType;
    }
    if ((result = rtNetDriverSendStream(entry->connection, data, length, sent)) != 0) {
        return result;
    }
    return rtNetStatsRecord(*sent, 0, kStreamOverhead);
}

int rtNetSocketSendTo(rtNetSocket *socket,
                      const char *address,
                      unsigned short port,
                      const void *data,
                      unsigned short length,
                      unsigned short *sent) {
    rtNetSocket *entry = NULL;
    int result;

    if (address == NULL || sent == NULL) {
        return kErrorArgument;
    }
    *sent = 0;
    if (data == NULL) {
        return kErrorArgument;
    }
    if ((result = rtNetSocketResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (entry->options.type != RT_NET_SOCKET_DATAGRAM) {
        return kErrorSocketType;
    }
    if ((result = rtNetDriverSendTo(entry->connection, address, port, data, length, sent)) != 0) {
        return result;
    }
    return rtNetStatsRecord(*sent, 0, kDatagramOverhead);
}

int rtNetSocketListen(rtNetSocket *socket,
                      bool *listening,
                      const char *address,
                      unsigned short port) {
    rtNetSocket *entry = NULL;
    int result;

    if (listening == NULL) {
        return kErrorArgument;
    }
    *listening = false;
    if (address == NULL) {
        return kErrorArgument;
    }
    if ((result = rtNetSocketResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (entry->options.type != RT_NET_SOCKET_LISTEN) {
        return kErrorSocketType;
    }
    if ((result = rtNetDriverListen(entry->connection, listening, address, port)) != 0) {
        return result;
    }
    return rtNetStatsRecord(kDatagramOverhead, 0, 0);
}

int rtNetSocketReceiveStream(rtNetSocket *socket,
                             void *buffer,
                             unsigned short size,
                             unsigned short *received) {
    rtNetSocket *entry = NULL;
    int result;

    if (received == NULL) {
        return kErrorArgument;
    }
    *received = 0;
    if (buffer == NULL) {
        return kErrorArgument;
    }
    if ((result = rtNetSocketResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (entry->options.type != RT_NET_SOCKET_STREAM &&
        entry->options.type != RT_NET_SOCKET_STREAM_ACCEPTED) {
        return kErrorSocketType;
    }
    if ((result = rtNetDriverReceiveStream(entry->connection, buffer, size, received)) != 0) {
        return result;
    }
    return rtNetStatsRecord(0, *received, kStreamOverhead);
}

int rtNetSocketReceiveFrom(rtNetSocket *socket,
                           char *address,
                           unsigned short *port,
                           void *buffer,
                           unsigned short size,
                           unsigned short *received) {
    rtNetSocket *entry = NULL;
    int result;

    if (address == NULL || received == NULL) {
        return kErrorArgument;
    }
    *received = 0;
    if (buffer == NULL) {
        return kErrorArgument;
    }
    if ((result = rtNetSocketResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (entry->options.type != RT_NET_SOCKET_DATAGRAM) {
        return kErrorSocketType;
    }
    if ((result = rtNetDriverReceiveFrom(
             entry->connection, address, port, buffer, size, received)) != 0) {
        return result;
    }
    return rtNetStatsRecord(0, *received, kDatagramOverhead);
}

int rtNetSocketAccept(rtNetSocket *socket, bool *accepted, char *address, unsigned short *port) {
    rtNetSocket *entry = NULL;
    int result;

    if (accepted == NULL) {
        return kErrorArgument;
    }
    *accepted = false;
    if (address == NULL || port == NULL) {
        return kErrorArgument;
    }
    if ((result = rtNetSocketResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (entry->options.type != RT_NET_SOCKET_LISTEN) {
        return kErrorSocketType;
    }
    if ((result = rtNetDriverAccept(entry->connection, accepted, address, port)) != 0) {
        return result;
    }
    if (!*accepted) {
        return 0;
    }
    return rtNetStatsRecord(0, kDatagramOverhead, 0);
}

int rtNetGetLocalAddress(char *address) {
    return rtNetDriverCopyLocalAddress(address);
}

rtDmeEndpointNode *rtDmeEndpointNodeCreate(rtDmePeer *peer) {
    rtDmeEndpointNode *node = NULL;

    rtDmeAllocZeroed((void **)&node, sizeof(rtDmeEndpointNode));
    if (node == NULL) {
        return NULL;
    }
    node->peer = peer;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

void rtDmeEndpointNodeDestroy(rtDmeEndpointNode *node) {
    void *block = node;

    if (node != NULL) {
        rtDmeFree(&block);
    }
}
