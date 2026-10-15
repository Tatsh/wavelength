#include "rt/rtsocket.h"

#include <stdbool.h>
#include <stddef.h>

#include "rt/rtdmedevice.h"
#include "rt/rtinet.h"
#include "rt/rtmemory.h"
#include "rt/rttime.h"

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
} RtNetRateCounter;

typedef struct {
    RtTime resetTime; // The start of the current interval.
    RtNetRateCounter packetsSent;
    RtNetRateCounter bytesSent;
    RtNetRateCounter packetsReceived;
    RtNetRateCounter bytesReceived;
} RtNetStats;

// NTSC-U/C: 0x003bfa50, PAL: 0x0043e910
static int g_nNetSocketUsers = 0;

// NTSC-U/C: 0x00528e18
static RtNetStats g_netStats;

// NTSC-U/C: 0x00528e50
static RtTime g_qwNetStatsLastSampleTime;

// NTSC-U/C: 0x002f0238, PAL: 0x002fa1a8
static int RtNetDriverInit(void) {
    return rtNetInetInit();
}

// NTSC-U/C: 0x002f0250, PAL: 0x002fa1c0
static int RtNetDriverShutdown(void) {
    return rtNetInetShutdown();
}

// NTSC-U/C: 0x002f0268, PAL: 0x002fa1d8
static int RtNetDriverOpen(LibnetAsyncHandle **connection,
                           const char *address,
                           unsigned short port,
                           const RtSocketOptions *options) {
    return rtNetInetOpen(connection, address, port, options);
}

// NTSC-U/C: 0x002f0280, PAL: 0x002fa1f0
static int RtNetDriverClose(LibnetAsyncHandle *connection) {
    return rtNetInetClose(connection);
}

// NTSC-U/C: 0x002f0298, PAL: 0x002fa208
static int RtNetDriverWriteStream(LibnetAsyncHandle *connection,
                                  const void *data,
                                  unsigned short length,
                                  unsigned short *sent) {
    return rtNetInetSendStream(connection, data, length, sent);
}

// NTSC-U/C: 0x002f02b0, PAL: 0x002fa220
static int RtNetDriverSendTo(LibnetAsyncHandle *connection,
                             const char *address,
                             unsigned short port,
                             const void *data,
                             unsigned short length,
                             unsigned short *sent) {
    return rtNetInetSendTo(connection, address, port, data, length, sent);
}

// NTSC-U/C: 0x002f02d0, PAL: 0x002fa240
static int RtNetDriverListen(LibnetAsyncHandle *connection,
                             bool *listening,
                             const char *address,
                             unsigned short port) {
    return rtNetInetSendMarker(connection, listening, address, port);
}

// NTSC-U/C: 0x002f02e8, PAL: 0x002fa258
static int RtNetDriverReadStream(LibnetAsyncHandle *connection,
                                 void *buffer,
                                 unsigned short size,
                                 unsigned short *received) {
    return rtNetInetReceiveStream(connection, buffer, size, received);
}

// NTSC-U/C: 0x002f0300, PAL: 0x002fa270
static int RtNetDriverReceiveFrom(LibnetAsyncHandle *connection,
                                  char *address,
                                  unsigned short *port,
                                  void *buffer,
                                  unsigned short size,
                                  unsigned short *received) {
    return rtNetInetReceiveFrom(connection, address, port, buffer, size, received);
}

// NTSC-U/C: 0x002f0318, PAL: 0x002fa288
static int RtNetDriverAccept(LibnetAsyncHandle *connection,
                             bool *accepted,
                             char *address,
                             unsigned short *port) {
    return rtNetInetReceiveMarker(connection, accepted, address, port);
}

// NTSC-U/C: 0x002f0338, PAL: 0x002fa2a8
static int RtNetDriverCopyLocalAddress(char *address) {
    return rtNetInetCopyLocalAddress(address);
}

// NTSC-U/C: 0x002efe90, PAL: 0x002f9e00
static int RtSocketRecordAlloc(RtSocket **socket) {
    RtSocket *block = NULL;

    if (socket == NULL) {
        return kErrorArgument;
    }
    *socket = NULL;
    if (RtAllocZeroedBlock((void **)&block, sizeof(RtSocket)) != 0) {
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
static int RtSocketRecordFree(RtSocket *socket) {
    void *block = socket;

    if (socket == NULL) {
        return kErrorArgument;
    }
    return RtFreeBlock(&block) != 0;
}

// NTSC-U/C: 0x002eff30, PAL: 0x002f9ea0
static int RtSocketRecordLookup(RtSocket *socket, RtSocket **entry) {
    if (entry == NULL) {
        return kErrorArgument;
    }
    *entry = socket;
    return 0;
}

// NTSC-U/C: 0x002eff98, PAL: 0x002f9f08
static bool RtSocketRecordIsValid(const RtSocket *socket) {
    return socket != NULL && socket->allocated != 0 && socket->open != 0 &&
           (unsigned int)socket->options.nType < RT_SOCKET_TYPE_COUNT;
}

// NTSC-U/C: 0x002eff48, PAL: 0x002f9eb8
static int RtSocketRecordResolve(RtSocket *socket, RtSocket **entry) {
    if (entry == NULL) {
        return kErrorArgument;
    }
    if (RtSocketRecordLookup(socket, entry) != 0) {
        return kErrorBadSocket;
    }
    return RtSocketRecordIsValid(*entry) ? 0 : kErrorBadSocket;
}

// NTSC-U/C: 0x002effc8, PAL: 0x002f9f38
static int RtSocketOptionsReset(RtSocketOptions *options) {
    if (options == NULL) {
        return kErrorArgument;
    }
    // Yes, the binary sets only the kind. The buffer size stays as the allocator cleared it.
    options->nType = RT_SOCKET_STREAM;
    return 0;
}

// NTSC-U/C: 0x002f0188, PAL: 0x002fa0f8
static int RtNetRateCounterUpdate(RtNetRateCounter *counter, unsigned int elapsed) {
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
static int RtNetStatsReset(void) {
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
    if (RtReadTime(&g_netStats.resetTime) != 0) {
        return kErrorFailed;
    }
    g_qwNetStatsLastSampleTime = g_netStats.resetTime;
    return 0;
}

// NTSC-U/C: 0x002f00e0, PAL: 0x002fa050
static int RtNetStatsRefreshRates(void) {
    unsigned int elapsed;
    int result;

    if (RtMeasureMillisecondsSince(&g_netStats.resetTime, &elapsed) != 0) {
        return kErrorFailed;
    }
    if (elapsed <= kStatsInterval) {
        return 0;
    }
    if ((result = RtNetRateCounterUpdate(&g_netStats.packetsSent, elapsed)) != 0) {
        return result;
    }
    if ((result = RtNetRateCounterUpdate(&g_netStats.bytesSent, elapsed)) != 0) {
        return result;
    }
    if ((result = RtNetRateCounterUpdate(&g_netStats.packetsReceived, elapsed)) != 0) {
        return result;
    }
    if ((result = RtNetRateCounterUpdate(&g_netStats.bytesReceived, elapsed)) != 0) {
        return result;
    }
    if (RtReadTime(&g_netStats.resetTime) != 0) {
        return kErrorFailed;
    }
    return 0;
}

// NTSC-U/C: 0x002f0068, PAL: 0x002f9fd8
static int RtNetStatsRecord(unsigned int sent, unsigned int received, unsigned int overhead) {
    if (sent != 0) {
        ++g_netStats.packetsSent.count;
        g_netStats.bytesSent.count += sent + overhead;
    }
    if (received != 0) {
        ++g_netStats.packetsReceived.count;
        g_netStats.bytesReceived.count += received + overhead;
    }
    return RtNetStatsRefreshRates();
}

int RtSocketStartup(void) {
    int result;

    if (g_nNetSocketUsers == 0) {
        if ((result = RtNetDriverInit()) != 0) {
            return result;
        }
        if ((result = RtNetStatsReset()) != 0) {
            return result;
        }
    }
    ++g_nNetSocketUsers;
    return 0;
}

int RtSocketShutdown(void) {
    if (g_nNetSocketUsers <= 0) {
        return kErrorNotStarted;
    }
    if (--g_nNetSocketUsers != 0) {
        return 0;
    }
    return RtNetDriverShutdown();
}

int RtSocketCreate(RtSocket **socket,
                   const char *address,
                   unsigned short port,
                   const RtSocketOptions *options) {
    RtSocket *created;
    RtSocket *entry = NULL;
    int result;

    if (socket == NULL) {
        return kErrorArgument;
    }
    *socket = NULL;
    if ((result = RtSocketRecordAlloc(&created)) != 0) {
        return result;
    }
    if ((result = RtSocketRecordLookup(created, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (options != NULL) {
        entry->options = *options;
    } else if ((result = RtSocketOptionsReset(&entry->options)) != 0) {
        return result;
    }
    if ((result = RtNetDriverOpen(&entry->connection, address, port, &entry->options)) != 0) {
        return result;
    }
    entry->port = port;
    entry->open = 1;
    *socket = created;
    return 0;
}

int RtSocketClose(RtSocket *socket) {
    RtSocket *entry = NULL;
    int result;

    if ((result = RtSocketRecordResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if ((result = RtNetDriverClose(entry->connection)) != 0) {
        return result;
    }
    entry->open = 0;
    entry->allocated = 0;
    return RtSocketRecordFree(socket);
}

int RtSocketSend(RtSocket *socket, const void *data, unsigned short length, unsigned short *sent) {
    RtSocket *entry = NULL;
    int result;

    if (sent == NULL) {
        return kErrorArgument;
    }
    *sent = 0;
    if (data == NULL) {
        return kErrorArgument;
    }
    if ((result = RtSocketRecordResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (entry->options.nType != RT_SOCKET_STREAM &&
        entry->options.nType != RT_SOCKET_STREAM_ACCEPTED) {
        return kErrorSocketType;
    }
    if ((result = RtNetDriverWriteStream(entry->connection, data, length, sent)) != 0) {
        return result;
    }
    return RtNetStatsRecord(*sent, 0, kStreamOverhead);
}

int RtSocketSendTo(RtSocket *socket,
                   const char *address,
                   unsigned short port,
                   const void *data,
                   unsigned short length,
                   unsigned short *sent) {
    RtSocket *entry = NULL;
    int result;

    if (address == NULL || sent == NULL) {
        return kErrorArgument;
    }
    *sent = 0;
    if (data == NULL) {
        return kErrorArgument;
    }
    if ((result = RtSocketRecordResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (entry->options.nType != RT_SOCKET_DATAGRAM) {
        return kErrorSocketType;
    }
    if ((result = RtNetDriverSendTo(entry->connection, address, port, data, length, sent)) != 0) {
        return result;
    }
    return RtNetStatsRecord(*sent, 0, kDatagramOverhead);
}

int RtSocketPollListener(RtSocket *socket,
                         bool *listening,
                         const char *address,
                         unsigned short port) {
    RtSocket *entry = NULL;
    int result;

    if (listening == NULL) {
        return kErrorArgument;
    }
    *listening = false;
    if (address == NULL) {
        return kErrorArgument;
    }
    if ((result = RtSocketRecordResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (entry->options.nType != RT_SOCKET_LISTEN) {
        return kErrorSocketType;
    }
    if ((result = RtNetDriverListen(entry->connection, listening, address, port)) != 0) {
        return result;
    }
    return RtNetStatsRecord(kDatagramOverhead, 0, 0);
}

int RtSocketReceive(RtSocket *socket, void *buffer, unsigned short size, unsigned short *received) {
    RtSocket *entry = NULL;
    int result;

    if (received == NULL) {
        return kErrorArgument;
    }
    *received = 0;
    if (buffer == NULL) {
        return kErrorArgument;
    }
    if ((result = RtSocketRecordResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (entry->options.nType != RT_SOCKET_STREAM &&
        entry->options.nType != RT_SOCKET_STREAM_ACCEPTED) {
        return kErrorSocketType;
    }
    if ((result = RtNetDriverReadStream(entry->connection, buffer, size, received)) != 0) {
        return result;
    }
    return RtNetStatsRecord(0, *received, kStreamOverhead);
}

int RtSocketReceiveFrom(RtSocket *socket,
                        char *address,
                        unsigned short *port,
                        void *buffer,
                        unsigned short size,
                        unsigned short *received) {
    RtSocket *entry = NULL;
    int result;

    if (address == NULL || received == NULL) {
        return kErrorArgument;
    }
    *received = 0;
    if (buffer == NULL) {
        return kErrorArgument;
    }
    if ((result = RtSocketRecordResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (entry->options.nType != RT_SOCKET_DATAGRAM) {
        return kErrorSocketType;
    }
    if ((result = RtNetDriverReceiveFrom(
             entry->connection, address, port, buffer, size, received)) != 0) {
        return result;
    }
    return RtNetStatsRecord(0, *received, kDatagramOverhead);
}

int RtSocketAccept(RtSocket *socket, bool *accepted, char *address, unsigned short *port) {
    RtSocket *entry = NULL;
    int result;

    if (accepted == NULL) {
        return kErrorArgument;
    }
    *accepted = false;
    if (address == NULL || port == NULL) {
        return kErrorArgument;
    }
    if ((result = RtSocketRecordResolve(socket, &entry)) != 0) {
        return result;
    }
    if (entry == NULL) {
        return kErrorNullSocket;
    }
    if (entry->options.nType != RT_SOCKET_LISTEN) {
        return kErrorSocketType;
    }
    if ((result = RtNetDriverAccept(entry->connection, accepted, address, port)) != 0) {
        return result;
    }
    if (!*accepted) {
        return 0;
    }
    return RtNetStatsRecord(0, kDatagramOverhead, 0);
}

int RtSocketGetLocalAddress(char *address) {
    return RtNetDriverCopyLocalAddress(address);
}

rtDmeEndpointNode *rtDmeEndpointNodeCreate(rtDmePeer *peer) {
    rtDmeEndpointNode *node = NULL;

    RtAllocZeroedBlock((void **)&node, sizeof(rtDmeEndpointNode));
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
        RtFreeBlock(&block);
    }
}
