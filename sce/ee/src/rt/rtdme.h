#ifndef RTDME_H
#define RTDME_H

#include <librtdme.h>

#include "rt/rtchannel.h"
#include "rt/rttime.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The engine of the peer-to-peer layer of the SCE-RT library. A console takes part in up to 4
 * connections, each with up to 256 clients. Messages are records of a class and a type with a field
 * list, sent to one client, a set of clients, or every client, over the channel of the connection
 * or as datagrams. Clients own ranges of shared objects, and the engine measures the latency of
 * each client.
 */

/** The number of connections. */
#define RT_DME_CONNECTION_COUNT 4

/** The number of clients of a connection. */
#define RT_DME_CLIENT_COUNT 256

/** The client index that stands for none. */
#define RT_DME_NO_CLIENT 256

/** The client index that stands for the server of a connection. */
#define RT_DME_SERVER_CLIENT 0xffff

/** The client index that stands for every client of a connection. */
#define RT_DME_ALL_CLIENTS (-1)

/** The number of addresses of a client. */
#define RT_DME_ADDRESS_COUNT 2

/** The number of message classes. */
#define RT_DME_MESSAGE_CLASS_COUNT 6

/** The number of message types of a class. */
#define RT_DME_MESSAGE_TYPE_COUNT 256

/** The number of fields of a message type. */
#define RT_DME_FIELD_COUNT 32

/** The number of shared objects. */
#define RT_DME_OBJECT_COUNT 4096

/** The number of object types. */
#define RT_DME_OBJECT_TYPE_COUNT 16

/** The number of object stream slots. */
#define RT_DME_STREAM_COUNT 2048

/** The number of outstanding latency probes. */
#define RT_DME_PING_COUNT 127

/** The number of pending listener polls. */
#define RT_DME_POLL_COUNT 256

/** The number of latency samples of a client. */
#define RT_DME_LATENCY_SAMPLES 100

/** The largest message, its 2-byte header excluded. */
#define RT_DME_MAX_MESSAGE_SIZE 510

/** The data of one fragment of a long message. */
#define RT_DME_FRAGMENT_DATA_SIZE 490

/** The size of the text of an address, the terminator included. */
#define RT_DME_ADDRESS_SIZE 16

/** The port of the peers when the settings give none. */
#define RT_DME_DEFAULT_PORT 10075

/** The results of the engine. */
#define RT_DME_RESULT_OUT_OF_RANGE 8    /*!< An index is out of range. */
#define RT_DME_RESULT_NO_RECORD 10      /*!< A record or an address is missing. */
#define RT_DME_RESULT_SEND_FAILED 13    /*!< The channel did not take a message. */
#define RT_DME_RESULT_BAD_CONNECTION 15 /*!< The connection index is out of range. */
#define RT_DME_RESULT_NOT_STARTED 17    /*!< The engine is not started. */
#define RT_DME_RESULT_STARTUP_FAILED 18 /*!< The channels did not start. */
#define RT_DME_RESULT_NO_MEMORY 22      /*!< An allocation failed. */
#define RT_DME_RESULT_INVALID 23        /*!< An argument is null or out of range. */
#define RT_DME_RESULT_SERVICE 24        /*!< The default for a failure of the channels. */
#define RT_DME_RESULT_MAPPING 30        /*!< The default for a failure of a port mapping. */
#define RT_DME_RESULT_TOO_LARGE 31      /*!< A message is too large. */
#define RT_DME_RESULT_BAD_VERSION 38    /*!< The settings have another version. */

/** The classes of messages. */
#define RT_DME_CLASS_SYSTEM 0      /*!< The messages of the engine. */
#define RT_DME_CLASS_APPLICATION 2 /*!< The messages of the game. */

/** The kinds of the addresses of a client. */
#define RT_DME_ADDRESS_PUBLIC 1  /*!< The address the other consoles reach. */
#define RT_DME_ADDRESS_PRIVATE 2 /*!< The address on the local network. */

/** The address indices of a client. */
#define RT_DME_ADDRESS_INDEX_PUBLIC 0     /*!< The public address works. */
#define RT_DME_ADDRESS_INDEX_PRIVATE 1    /*!< The private address works. */
#define RT_DME_ADDRESS_INDEX_UNRESOLVED 2 /*!< No address was tried yet. */
#define RT_DME_ADDRESS_INDEX_NONE 3       /*!< The client does not exist. */

/** The states of a connection. */
#define RT_DME_CONNECTION_IDLE 0       /*!< Not connected. */
#define RT_DME_CONNECTION_CONNECTED 1  /*!< Connected. */
#define RT_DME_CONNECTION_CONNECTING 2 /*!< The channel was opened. */
#define RT_DME_CONNECTION_FAILED 3     /*!< The connection failed. */

/** The roles of a client. */
#define RT_DME_ROLE_LOCAL 1 /*!< The client of this console. */
#define RT_DME_ROLE_HOST 2  /*!< The host of the session. */

/** The states of a shared object. */
#define RT_DME_OBJECT_FREE 0    /*!< The object is free. */
#define RT_DME_OBJECT_CLOSING 1 /*!< The object is being destroyed. */
#define RT_DME_OBJECT_USED 2    /*!< The object exists. */

/**
 * Receiver of a client that joined a connection.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param pszAddress The address the client is reached at.
 */
typedef void (*RtDmeClientJoinFunc)(int nConnection, int nClient, const char *pszAddress);

/**
 * Receiver of a client that left a connection.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param pszAddress The address the departure was reported for.
 */
typedef void (*RtDmeClientLeaveFunc)(int nConnection, int nClient, const char *pszAddress);

/**
 * Receiver of the result of a connection.
 *
 * @param nConnection The connection, or -1 when none was opened.
 * @param nState An RT_DME_CONNECTION_ value.
 * @param nStatus The status the server or the channels reported.
 * @param nResult The result of the last step, 0 for success.
 */
typedef void (*RtDmeConnectFunc)(int nConnection, int nState, int nStatus, int nResult);

/** The arguments of the receiver of a change of host. */
typedef struct {
    int nConnection; /*!< The connection, always 0. */
    int nHost;       /*!< The new host. */
    int nArgument;   /*!< The argument given with the receiver. */
} RtDmeHostChange;

/**
 * Receiver of a change of the host of connection 0.
 *
 * @param pChange The change.
 */
typedef void (*RtDmeHostChangeFunc)(const RtDmeHostChange *pChange);

/**
 * Receiver of a new shared object.
 *
 * @param nOwner The client that owns the object.
 * @param nObject The object.
 */
typedef void (*RtDmeObjectAddedFunc)(int nOwner, int nObject);

/**
 * Receiver of a change to a shared object.
 *
 * @param nOwner The client that owns the object.
 * @param nObject The object.
 * @param nField The field that changed, -1 for every field, or -2 for the object.
 */
typedef void (*RtDmeObjectChangedFunc)(int nOwner, int nObject, int nField);

/**
 * Receiver of a shared object about to be destroyed.
 *
 * @param nOwner The client that owns the object.
 * @param nObject The object.
 */
typedef void (*RtDmeObjectRemovedFunc)(int nOwner, int nObject);

/**
 * Receiver of the result of a latency probe.
 *
 * @param nResult The result.
 * @param pbAddress The 4 bytes of the address probed.
 * @param nLatency The milliseconds the answer took.
 */
typedef void (*RtDmePingFunc)(int nResult, const unsigned char *pbAddress, int nLatency);

/** One field of a message type. */
typedef struct {
    int nOffset;           /*!< The offset of the field in the record. */
    unsigned short uSize;  /*!< The size of an element in bytes. */
    unsigned short uCount; /*!< The number of elements. */
} RtDmeField;

/** A registered message type. */
typedef struct {
    int nFieldCount;                        /*!< The number of fields. */
    RtDmeField aFields[RT_DME_FIELD_COUNT]; /*!< The fields. */
    rtDmeMessageHandler pfnHandler;         /*!< Receives the messages of the type. */
} RtDmeMessageType;

/** A client of a connection. */
typedef struct {
    rtNetAddress addresses[RT_DME_ADDRESS_COUNT]; /*!< The public and the private address. */
    int nRole;                                    /*!< An RT_DME_ROLE_ value, or 0. */
    int anReserved34[2];                          /* +0x34, cleared when the local client is set */
    unsigned char abReserved3c[28];               /* +0x3c, not read by this part of the engine */
    unsigned char uAddressIndex;                  /*!< An RT_DME_ADDRESS_INDEX_ value. */
    unsigned char bJoined;                        /*!< Non-zero once the client joined. */
    RtTime lastHeard;          /*!< When the client joined or last sent a datagram. */
    unsigned char bCleared;    /*!< Set when the record is cleared. */
    unsigned char uReserved65; /* +0x65 */
    signed char nFlags;        /*!< The flags the client announces, bit 0 for datagrams. */
} RtDmeClient;

/** A connection. */
typedef struct {
    RtDmeClient *apClients[RT_DME_CLIENT_COUNT]; /*!< The clients, null where none. */
    int nLocalClient;                            /*!< The client of this console. */
    int nHostClient;                             /*!< The client that keeps the clock. */
    char szServerName[RT_DME_ADDRESS_SIZE];      /*!< The address of the server. */
    char szSessionName[RT_DME_ADDRESS_SIZE];     /*!< The name the server gave the session. */
    RtDmeClientJoinFunc pfnClientJoin;           /*!< Receives the clients that joined. */
    RtDmeClientLeaveFunc pfnClientLeave;         /*!< Receives the clients that left. */
    int nState;                                  /*!< An RT_DME_CONNECTION_ value. */
    int nWorldId;                                /*!< The session, or -1. */
    int nPendingJoins;                           /*!< The joins announced and not yet done. */
    int nMaxClients;                             /*!< The clients the connection takes. */
} RtDmeConnection;

/** The latency record of a client. */
typedef struct {
    unsigned int auSamples[RT_DME_LATENCY_SAMPLES]; /*!< The latencies in milliseconds. */
    int nSampleCount;                               /*!< The samples recorded. */
    int nReserved194;                               /* +0x194 */
    RtTime lastProbe;                               /*!< When the client was last probed. */
} RtDmeClientStats;

/** The objects a client owns. */
typedef struct {
    unsigned char abReserved00[6]; /* +0x00 */
    unsigned short uFirstObject;   /*!< The first object of the client. */
    unsigned short uObjectCount;   /*!< The number of objects of the client. */
} RtDmeObjectRange;

/** A shared object. */
typedef struct {
    unsigned char uType;              /*!< The type, which selects the receivers. */
    unsigned char abReserved01[7];    /* +0x01 */
    int nOwner;                       /*!< The client that owns the object. */
    char szName[RT_DME_ADDRESS_SIZE]; /*!< The name of the object. */
    unsigned char *pbData;            /*!< The data of the object. */
    unsigned char *pbShadow;          /*!< The copy of the data last sent. */
} RtDmeObject;

/** A slot of an object stream. */
typedef struct {
    unsigned char bUsed;            /*!< Non-zero while the slot is used. */
    unsigned char abReserved01[67]; /* +0x01 */
} RtDmeStream;

/** A set of clients, as a bitmap with its first and last members. */
typedef struct {
    unsigned char abBits[RT_DME_CLIENT_COUNT / 8]; /*!< One bit per client. */
    int nReserved20;                               /* +0x20 */
    int nFirst;                                    /*!< The lowest client of the set. */
    int nLast;                                     /*!< The highest client of the set. */
} RtDmeClientSet;

/** An outstanding latency probe. */
typedef struct {
    unsigned char abAddress[4]; /*!< The address probed. */
    unsigned short uPort;       /*!< The port probed. */
    int nReserved08;            /* +0x08, cleared when the probe is sent */
    RtDmePingFunc pfnDone;      /*!< Receives the result, null when the slot is free. */
    unsigned int uSentTime;     /*!< When the probe was sent, in milliseconds. */
    int nConnection;            /*!< The connection of the client probed. */
    int nClient;                /*!< The client probed, or RT_DME_NO_CLIENT. */
} RtDmePing;

/**
 * Receiver of the result of a listener poll.
 *
 * @param nResult The result.
 * @param pszAddress The address polled.
 * @param nValue The value of the poll.
 */
typedef void (*RtDmePollFunc)(int nResult, const char *pszAddress, int nValue);

/** A pending poll of the listener for a peer. */
typedef struct {
    unsigned char uState;                    /*!< The state of the poll, 0 when the slot is free. */
    char szAddress[RT_DME_ADDRESS_SIZE + 1]; /*!< The address polled. */
    unsigned short uPort;                    /*!< The port polled. */
    RtTime startTime;                        /*!< When the poll started. */
    int nValue;                              /*!< The value passed to the receiver. */
    RtDmePollFunc pfnDone;                   /*!< Receives the result. */
} RtDmePoll;

/** The result of a port mapping, as the receiver of rtDme mappings gets it. */
typedef struct {
    int bDone;                                    /*!< Non-zero once the mapping finished. */
    int nResult;                                  /*!< 0, or an error code. */
    rtNetAddress addresses[RT_DME_ADDRESS_COUNT]; /*!< The public and the private address. */
} RtDmeMapping;

/**
 * Receiver of a finished port mapping.
 *
 * @param pMapping The result.
 */
typedef void (*RtDmeMappingFunc)(const RtDmeMapping *pMapping);

/** Non-zero while the engine is started. */
extern int g_nDmeRunning;
/** The connection the engine serves. */
extern int g_nDmeActiveConnection;
/** The objects in use, as an upper bound of their indices. */
extern int g_nDmeObjectCount;
/** Receives the result of the next connection, then is cleared. */
extern RtDmeConnectFunc g_pfnDmeConnect;
/** Receives the changes of host of connection 0. */
extern RtDmeHostChangeFunc g_pfnDmeHostChange;
/** The argument given to g_pfnDmeHostChange. */
extern int g_nDmeHostChangeArgument;
/** Non-zero while the datagram service is started. */
extern unsigned char g_bDmeServiceStarted;
/** Set when a received message could not be dispatched. */
extern unsigned char g_bDmeReceiveError;
/** Set after a connection result was reported. */
extern unsigned char g_bDmeConnectReported;
/** Non-zero while the listener is open. */
extern unsigned char g_bDmeListening;
/** The object stream slots, allocated when first used. */
extern RtDmeStream *g_pDmeStreams;
/** The number of message types registered in each class. */
extern unsigned char g_auDmeMessageTypeCount[RT_DME_MESSAGE_CLASS_COUNT];
/** The time of the session in milliseconds, kept by its host. */
extern unsigned int g_uDmeSessionTime;
/** The host of connection 0, or RT_DME_NO_CLIENT. */
extern int g_nDmeHostClient;
/** The port of the peers. */
extern unsigned short g_uDmePort;
/** The port of the sender of the last datagram. */
extern unsigned short g_uDmeSenderPort;
/** Non-zero when g_abDmeFixedAddress replaces the private address. */
extern int g_bDmeUseFixedAddress;
/** The identifier of the game. */
extern int g_nDmeApplicationId;
/** The connections, allocated when opened. */
extern RtDmeConnection *g_apDmeConnections[RT_DME_CONNECTION_COUNT];
/** The latency records of the clients, allocated when first used. */
extern RtDmeClientStats *g_apDmeClientStats[RT_DME_CLIENT_COUNT];
/** The result of the last port mapping. */
extern RtDmeMapping g_dmeMapping;
/** The pending listener polls. */
extern RtDmePoll g_aDmePolls[RT_DME_POLL_COUNT];
/** The registered message types. */
extern RtDmeMessageType *g_apDmeMessageTypes[RT_DME_MESSAGE_CLASS_COUNT][RT_DME_MESSAGE_TYPE_COUNT];
/** The receivers of the objects that changed, by object type. */
extern RtDmeObjectRemovedFunc g_apfnDmeObjectRemoved[RT_DME_OBJECT_TYPE_COUNT];
/** The receivers of the new objects, by object type. */
extern RtDmeObjectAddedFunc g_apfnDmeObjectAdded[RT_DME_OBJECT_TYPE_COUNT];
/** The receivers of the changes to objects, by object type. */
extern RtDmeObjectChangedFunc g_apfnDmeObjectChanged[RT_DME_OBJECT_TYPE_COUNT];
/** The state of each object, an RT_DME_OBJECT_ value. */
extern signed char g_anDmeObjectState[RT_DME_OBJECT_COUNT];
/** The local addresses, public and private, as the other clients see this console. */
extern rtNetAddress g_dmeLocalAddresses[RT_DME_ADDRESS_COUNT];
/** The address of the sender of the last datagram. */
extern char g_szDmeSenderAddress[RT_DME_ADDRESS_SIZE];
/** The objects, allocated when created. */
extern RtDmeObject *g_apDmeObjects[RT_DME_OBJECT_COUNT];
/** The outstanding latency probes. */
extern RtDmePing g_aDmePings[RT_DME_PING_COUNT];

/** The system message that announces the flags of a client. */
extern int g_nDmeClientFlagsMessage;
/** The system message that announces a client to its peers. */
extern int g_nDmeClientJoinMessage;
/** The system message that carries a fragment of a long message. */
extern int g_nDmeFragmentMessage;
/** The system message that announces the addresses of a client that left. */
extern int g_nDmeClientAddressesMessage;

/**
 * Report the version of the library.
 *
 * @param pszVersion Receives the version, RT_NET_VERSION_SIZE bytes.
 * @return 0, or RT_DME_RESULT_INVALID.
 * @ghidraAddress NTSC-U/C: 0x002a7bf0
 * @ghidraAddress PAL: 0x002b1b60
 */
int RtDmeGetVersion(char *pszVersion);

/**
 * Start the voice layer, counting the starts.
 *
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002ae070
 * @ghidraAddress PAL: 0x002b7fe0
 */
int RtDmeVoiceStartup(void);

/**
 * Undo one start of the voice layer.
 *
 * @return 0, or RT_DME_RESULT_NOT_STARTED.
 * @ghidraAddress NTSC-U/C: 0x002ae0d0
 * @ghidraAddress PAL: 0x002b8040
 */
int RtDmeVoiceShutdown(void);

/**
 * Close the voice of a connection.
 *
 * @param nConnection The connection.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ae728
 * @ghidraAddress PAL: 0x002b8698
 */
int RtDmeVoiceClose(int nConnection);

/**
 * Start the voice of the local client of a connection.
 *
 * @param nConnection The connection.
 * @param nClient The local client.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ae7d0
 * @ghidraAddress PAL: 0x002b8740
 */
int RtDmeVoiceStart(int nConnection, int nClient);

/**
 * Report whether the voice of a connection is open.
 *
 * @param nConnection The connection.
 * @return 1 when it is open.
 * @ghidraAddress NTSC-U/C: 0x002ae8f0
 * @ghidraAddress PAL: 0x002b8860
 */
int RtDmeVoiceIsOpen(int nConnection);

/**
 * Add a client to the voice of a connection.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002af040
 * @ghidraAddress PAL: 0x002b8fb0
 */
int RtDmeVoiceAddClient(int nConnection, int nClient);

/**
 * Remove a client from the voice of a connection.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002af140
 * @ghidraAddress PAL: 0x002b90b0
 */
int RtDmeVoiceRemoveClient(int nConnection, int nClient);

/**
 * Allocate the object ranges of the clients.
 *
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002afc20
 * @ghidraAddress PAL: 0x002b9b90
 */
int RtDmeInitObjectRanges(void);

/**
 * Look up the object range of a client.
 *
 * @param ppRange Receives the range, or null.
 * @param nConnection The connection.
 * @param nClient The client.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002afc68
 * @ghidraAddress PAL: 0x002b9bd8
 */
int RtDmeGetObjectRange(RtDmeObjectRange **ppRange, int nConnection, int nClient);

/**
 * Release the objects of a client.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b0290
 * @ghidraAddress PAL: 0x002ba200
 */
int RtDmeReleaseClientObjects(int nConnection, int nClient);

/**
 * Send the state of a client to the others.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b0700
 * @ghidraAddress PAL: 0x002ba670
 */
int RtDmeSendClientState(int nConnection, int nClient);

/**
 * Convert an index of a connection to the identifier of its channel.
 *
 * @param nConnection The connection.
 * @return The channel.
 * @ghidraAddress NTSC-U/C: 0x002b0ae8
 * @ghidraAddress PAL: 0x002baa58
 */
unsigned short RtDmeConnectionToChannel(int nConnection);

/**
 * Convert the identifier of a channel to the index of its connection.
 *
 * @param uChannel The channel.
 * @return The connection.
 * @ghidraAddress NTSC-U/C: 0x002b0b08
 * @ghidraAddress PAL: 0x002baa78
 */
int RtDmeChannelToConnection(unsigned short uChannel);

/**
 * Find the client that an address and a port belong to, in the active connection first.
 *
 * @param pnConnection Receives the connection.
 * @param pnClient Receives the client, or RT_DME_NO_CLIENT.
 * @param pszAddress The address.
 * @param uPort The port.
 * @ghidraAddress NTSC-U/C: 0x002b0b18
 * @ghidraAddress PAL: 0x002baa88
 */
void RtDmeFindClientByAddress(int *pnConnection,
                              int *pnClient,
                              const char *pszAddress,
                              unsigned short uPort);

/**
 * Report the latency of a client, the 5th percentile of its samples.
 *
 * @param puLatency Receives the latency in milliseconds.
 * @param uClient The client.
 * @return 0, RT_DME_RESULT_NO_MEMORY, or RT_DME_RESULT_INVALID.
 * @ghidraAddress NTSC-U/C: 0x002b0f40
 * @ghidraAddress PAL: 0x002baeb0
 */
int RtDmeGetClientLatency(unsigned int *puLatency, unsigned int uClient);

/**
 * Send the flags of the local client to a client, or to every client of connection 0.
 *
 * @param nConnection The connection.
 * @param nClient The client, or RT_DME_ALL_CLIENTS.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b1070
 * @ghidraAddress PAL: 0x002bafe0
 */
int RtDmeSendClientFlags(int nConnection, int nClient);

/**
 * Report the local client of a connection.
 *
 * @param pnClient Receives the client, or RT_DME_NO_CLIENT.
 * @param nConnection The connection.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b1138
 * @ghidraAddress PAL: 0x002bb0a8
 */
int RtDmeGetLocalClient(int *pnClient, int nConnection);

/**
 * Install the receiver of the changes of host.
 *
 * @param pfnHook The receiver.
 * @param nArgument The argument it receives.
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002b11b8
 * @ghidraAddress PAL: 0x002bb128
 */
int RtDmeSetHostChangeHook(RtDmeHostChangeFunc pfnHook, int nArgument);

/**
 * Set the time of the session.
 *
 * @param uTime The time in milliseconds.
 * @ghidraAddress NTSC-U/C: 0x002b1228
 * @ghidraAddress PAL: 0x002bb198
 */
void RtDmeSetSessionTime(unsigned int uTime);

/**
 * Allocate a zeroed block, reporting a failure.
 *
 * @param uSize The size in bytes.
 * @return The block, or null.
 * @ghidraAddress NTSC-U/C: 0x002b1238
 * @ghidraAddress PAL: 0x002bb1a8
 */
void *RtDmeAlloc(unsigned int uSize);

/**
 * Release a block and clear the pointer to it, reporting a failure.
 *
 * @param ppBlock The pointer to the block.
 * @ghidraAddress NTSC-U/C: 0x002b1278
 * @ghidraAddress PAL: 0x002bb1e8
 */
void RtDmeFree(void **ppBlock);

/**
 * Install the allocator of the library.
 *
 * @param pfnAlloc The allocator.
 * @param pfnFree The release.
 * @return 0, or RT_DME_RESULT_NO_MEMORY.
 * @ghidraAddress NTSC-U/C: 0x002b12c0
 * @ghidraAddress PAL: 0x002bb230
 */
int RtDmeSetMemoryFunctions(rtAllocFunc pfnAlloc, rtFreeFunc pfnFree);

/**
 * Parse a dotted address into its 4 bytes.
 *
 * @param pbAddress Receives the bytes.
 * @param pszAddress The text.
 * @return 0, RT_DME_RESULT_NO_RECORD, or RT_DME_RESULT_INVALID for empty text.
 * @ghidraAddress NTSC-U/C: 0x002b12e8
 * @ghidraAddress PAL: 0x002bb258
 */
int RtDmeParseAddress(unsigned char *pbAddress, const char *pszAddress);

/**
 * Write 4 address bytes as dotted text.
 *
 * @param pszAddress Receives the text.
 * @param pbAddress The bytes.
 * @return 0, or RT_DME_RESULT_NO_RECORD.
 * @ghidraAddress NTSC-U/C: 0x002b13a8
 * @ghidraAddress PAL: 0x002bb318
 */
int RtDmeFormatAddress(char *pszAddress, const unsigned char *pbAddress);

/**
 * Copy the public address of this console.
 *
 * @param pszAddress Receives the address, RT_DME_ADDRESS_SIZE bytes, cleared first.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b13f0
 * @ghidraAddress PAL: 0x002bb360
 */
int RtDmeGetLocalAddress(char *pszAddress);

/**
 * Register a client that joined and announce this console to it.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param pszAddress The address the client was reported at.
 * @ghidraAddress NTSC-U/C: 0x002b1458
 * @ghidraAddress PAL: 0x002bb3c8
 */
void RtDmeAddClient(int nConnection, int nClient, const char *pszAddress);

/**
 * Report the address of the sender of the last datagram.
 *
 * @param pbAddress Receives the 4 bytes of the address.
 * @return 0, or RT_DME_RESULT_INVALID.
 * @ghidraAddress NTSC-U/C: 0x002b1850
 * @ghidraAddress PAL: 0x002bb7c0
 */
int RtDmeGetSenderAddress(unsigned char *pbAddress);

/**
 * Look up a message type.
 *
 * @param nClass The class.
 * @param nType The type.
 * @return The type, or null.
 * @ghidraAddress NTSC-U/C: 0x002b1880
 * @ghidraAddress PAL: 0x002bb7f0
 */
RtDmeMessageType *RtDmeGetMessageType(unsigned int nClass, unsigned int nType);

/**
 * Look up an object.
 *
 * @param ppObject Receives the object, or null.
 * @param nObject The object.
 * @return 0, RT_DME_RESULT_OUT_OF_RANGE, or RT_DME_RESULT_INVALID.
 * @ghidraAddress NTSC-U/C: 0x002b18b8
 * @ghidraAddress PAL: 0x002bb828
 */
int RtDmeGetObject(RtDmeObject **ppObject, unsigned int nObject);

/**
 * Look up a slot of the object streams.
 *
 * @param ppStream Receives the slot. It is left unchanged when the lookup fails.
 * @param nStream The slot.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b1900
 * @ghidraAddress PAL: 0x002bb870
 */
int RtDmeFindStream(RtDmeStream **ppStream, unsigned int nStream);

/**
 * Look up a slot of the object streams, allocating the slots when first used.
 *
 * @param nStream The slot.
 * @param ppStream Receives the slot, or null.
 * @return 0, RT_DME_RESULT_OUT_OF_RANGE, RT_DME_RESULT_NO_MEMORY, or RT_DME_RESULT_INVALID.
 * @ghidraAddress NTSC-U/C: 0x002b1948
 * @ghidraAddress PAL: 0x002bb8b8
 */
int RtDmeGetStream(unsigned int nStream, RtDmeStream **ppStream);

/**
 * Report the time of this console in milliseconds.
 *
 * @param puTime Receives the time.
 * @return 0, or RT_DME_RESULT_INVALID.
 * @ghidraAddress NTSC-U/C: 0x002b1a18
 * @ghidraAddress PAL: 0x002bb988
 */
int RtDmeGetLocalTime(unsigned int *puTime);

/**
 * Report the time of the session.
 *
 * @param puTime Receives the time in milliseconds.
 * @return 0, or RT_DME_RESULT_INVALID.
 * @ghidraAddress NTSC-U/C: 0x002b1a58
 * @ghidraAddress PAL: 0x002bb9c8
 */
int RtDmeGetSessionTime(unsigned int *puTime);

/**
 * Report the client that keeps the clock of a connection.
 *
 * @param pnClient Receives the client, or RT_DME_NO_CLIENT.
 * @param nConnection The connection.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b1a80
 * @ghidraAddress PAL: 0x002bb9f0
 */
int RtDmeGetHostClient(int *pnClient, int nConnection);

/**
 * Count the clients of a connection that joined.
 *
 * @param pnCount Receives the count.
 * @param nConnection The connection.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b1ae8
 * @ghidraAddress PAL: 0x002bba58
 */
int RtDmeCountJoinedClients(int *pnCount, int nConnection);

/**
 * Add a client to a set.
 *
 * @param pSet The set.
 * @param nClient The client.
 * @ghidraAddress NTSC-U/C: 0x002b1b90
 * @ghidraAddress PAL: 0x002bbb00
 */
void RtDmeAddToClientSet(RtDmeClientSet *pSet, int nClient);

/**
 * Send a message as a datagram to an address.
 *
 * @param nFlags The send flags.
 * @param pszAddress The address.
 * @param uPort The port.
 * @param nClass The class of the message.
 * @param nType The type of the message.
 * @param nSize The size of the message.
 * @param pData The message.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b1c90
 * @ghidraAddress PAL: 0x002bbc00
 */
int RtDmeSendDatagram(signed char nFlags,
                      const char *pszAddress,
                      unsigned short uPort,
                      unsigned int nClass,
                      unsigned int nType,
                      unsigned int nSize,
                      const void *pData);

/**
 * Send a message as a datagram to an address given as 4 bytes, on the port of the peers.
 *
 * @param nFlags The send flags.
 * @param pbAddress The address.
 * @param nClass The class of the message.
 * @param nType The type of the message.
 * @param nSize The size of the message.
 * @param pData The message.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b1dd8
 * @ghidraAddress PAL: 0x002bbd48
 */
int RtDmeSendDatagramToBytes(signed char nFlags,
                             const unsigned char *pbAddress,
                             unsigned int nClass,
                             unsigned int nType,
                             int nSize,
                             const void *pData);

/**
 * Send a message as a datagram to the clients of a connection.
 *
 * @param bIncludeSelf Non-zero to send it to the local client too.
 * @param bForce Non-zero to send it to the clients that do not take datagrams.
 * @param nFlags The send flags.
 * @param nConnection The connection.
 * @param nClass The class of the message.
 * @param nType The type of the message.
 * @param nSize The size of the message.
 * @param pData The message.
 * @return 0, or RT_DME_RESULT_SEND_FAILED when a send failed.
 * @ghidraAddress NTSC-U/C: 0x002b1e68
 * @ghidraAddress PAL: 0x002bbdd8
 */
int RtDmeBroadcastDatagram(unsigned char bIncludeSelf,
                           unsigned char bForce,
                           signed char nFlags,
                           int nConnection,
                           unsigned int nClass,
                           unsigned int nType,
                           unsigned int nSize,
                           const void *pData);

/**
 * Send a message of any length to a client, the server, or every client of a connection.
 *
 * @param nFlags The send flags.
 * @param nConnection The connection.
 * @param nClient The client, RT_DME_SERVER_CLIENT, or RT_DME_ALL_CLIENTS.
 * @param nClass The class of the message.
 * @param nType The type of the message.
 * @param nSize The size of the message.
 * @param pData The message.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b22d8
 * @ghidraAddress PAL: 0x002bc248
 */
int RtDmeSend(signed char nFlags,
              int nConnection,
              int nClient,
              unsigned int nClass,
              unsigned int nType,
              int nSize,
              const void *pData);

/**
 * Send a message to the clients of a set.
 *
 * @param nFlags The send flags.
 * @param nConnection The connection.
 * @param nClass The class of the message.
 * @param nType The type of the message.
 * @param nSize The size of the message.
 * @param pData The message.
 * @param pSet The clients.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b2508
 * @ghidraAddress PAL: 0x002bc478
 */
int RtDmeSendToClientSet(signed char nFlags,
                         int nConnection,
                         unsigned int nClass,
                         unsigned int nType,
                         int nSize,
                         const void *pData,
                         const RtDmeClientSet *pSet);

/**
 * Send a message of the game.
 *
 * @param nFlags The send flags.
 * @param nConnection The connection.
 * @param nClient The client, RT_DME_SERVER_CLIENT, or RT_DME_ALL_CLIENTS.
 * @param nType The type of the message.
 * @param nSize The size of the message.
 * @param pData The message.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b2688
 * @ghidraAddress PAL: 0x002bc5f8
 */
int RtDmeSendApplicationMessage(signed char nFlags,
                                int nConnection,
                                int nClient,
                                unsigned int nType,
                                int nSize,
                                const void *pData);

/**
 * Send a message of the engine.
 *
 * @param nFlags The send flags.
 * @param nConnection The connection.
 * @param nClient The client, RT_DME_SERVER_CLIENT, or RT_DME_ALL_CLIENTS.
 * @param nType The type of the message.
 * @param nSize The size of the message.
 * @param pData The message.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b26e0
 * @ghidraAddress PAL: 0x002bc650
 */
int RtDmeSendSystemMessage(signed char nFlags,
                           int nConnection,
                           int nClient,
                           unsigned int nType,
                           int nSize,
                           const void *pData);

/**
 * Report a new object to the receiver of its type.
 *
 * @param nOwner The client that owns the object.
 * @param nObject The object.
 * @ghidraAddress NTSC-U/C: 0x002b2738
 * @ghidraAddress PAL: 0x002bc6a8
 */
void RtDmeNotifyObjectAdded(unsigned int nOwner, unsigned int nObject);

/**
 * Report a change to an object to the receiver of its type.
 *
 * @param nOwner The client that owns the object.
 * @param nObject The object.
 * @param nField The field, below 64, -1 for every field, or -2 for the object.
 * @ghidraAddress NTSC-U/C: 0x002b27b0
 * @ghidraAddress PAL: 0x002bc720
 */
void RtDmeNotifyObjectChanged(int nOwner, unsigned int nObject, int nField);

/**
 * Destroy an object of the active connection.
 *
 * @param nObject The object.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b28c8
 * @ghidraAddress PAL: 0x002bc838
 */
int RtDmeDestroyObject(unsigned int nObject);

/**
 * Find an object of a client by its name.
 *
 * @param pszName The name.
 * @param nConnection The connection.
 * @param nClient The client.
 * @return The object, or -1.
 * @ghidraAddress NTSC-U/C: 0x002b2a18
 * @ghidraAddress PAL: 0x002bc988
 */
int RtDmeFindObjectByName(const char *pszName, unsigned int nConnection, unsigned int nClient);

/**
 * Find a free object of the local client of the active connection.
 *
 * @return The object, or -1.
 * @ghidraAddress NTSC-U/C: 0x002b2b28
 * @ghidraAddress PAL: 0x002bca98
 */
int RtDmeFindFreeObject(void);

/**
 * Unregister a client, report its departure, and pick a new host when it was the host.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param pszAddress The address reported with the departure.
 * @param bAnnounce Non-zero to announce the departure to the other clients of connection 0.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b2f00
 * @ghidraAddress PAL: 0x002bce70
 */
int RtDmeRemoveClient(int nConnection,
                      int nClient,
                      const char *pszAddress,
                      unsigned char bAnnounce);

/**
 * Start the engine and its channels.
 *
 * @param pSettings The settings.
 * @param pStatus Receives the status.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b35b8
 * @ghidraAddress PAL: 0x002bd528
 */
int RtDmeStartup(const rtNetSettings *pSettings, rtNetInitStatus *pStatus);

/**
 * Start the datagram service of the peers, once.
 *
 * @param uSendSize The size of the send queue.
 * @param uReceiveSize The size of the receive queue.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b38e8
 * @ghidraAddress PAL: 0x002bd858
 */
int RtDmeStartDatagramService(unsigned int uSendSize, unsigned int uReceiveSize);

/**
 * Destroy every object.
 *
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002b39a8
 * @ghidraAddress PAL: 0x002bd918
 */
int RtDmeDestroyAllObjects(void);

/**
 * Convert a status of the channels to a status of the engine.
 *
 * @param pnStatus Receives the status.
 * @param nChannelStatus The status of the channels, below 10.
 * @return 0, or RT_DME_RESULT_INVALID.
 * @ghidraAddress NTSC-U/C: 0x002b3ab8
 * @ghidraAddress PAL: 0x002bda28
 */
int RtDmeConvertStatus(int *pnStatus, unsigned int nChannelStatus);

/**
 * Record the result of connecting: the local client, or the failure.
 *
 * @param nConnection The connection.
 * @param nClient The local client the server gave.
 * @param bConnected Non-zero when the connection succeeded.
 * @param nReason The status reported with the result.
 * @ghidraAddress NTSC-U/C: 0x002b3b98
 * @ghidraAddress PAL: 0x002bdb08
 */
void RtDmeSetConnectResult(int nConnection, int nClient, unsigned char bConnected, int nReason);

/**
 * Convert a result of the channels to a result of the engine.
 *
 * @param nResult The result of the channels.
 * @param nDefault The result for a failure without its own result.
 * @return The result.
 * @ghidraAddress NTSC-U/C: 0x002b7d48
 * @ghidraAddress PAL: 0x002c1cb8
 */
int RtDmeConvertResult(int nResult, int nDefault);

/**
 * Report which address of a client works.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @return An RT_DME_ADDRESS_INDEX_ value.
 * @ghidraAddress NTSC-U/C: 0x002b7e08
 * @ghidraAddress PAL: 0x002c1d78
 */
int RtDmeGetAddressIndex(int nConnection, int nClient);

/**
 * Set which address of a client works.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param uIndex RT_DME_ADDRESS_INDEX_PUBLIC or RT_DME_ADDRESS_INDEX_PRIVATE.
 * @ghidraAddress NTSC-U/C: 0x002b7e88
 * @ghidraAddress PAL: 0x002c1df8
 */
void RtDmeSetAddressIndex(int nConnection, int nClient, unsigned char uIndex);

/**
 * Report an address of a client.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param uIndex The address.
 * @return The text of the address, or null.
 * @ghidraAddress NTSC-U/C: 0x002b7f10
 * @ghidraAddress PAL: 0x002c1e80
 */
char *RtDmeGetClientAddress(int nConnection, int nClient, unsigned char uIndex);

/**
 * Set an address of a client.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param uIndex The address.
 * @param pszAddress The text of the address.
 * @ghidraAddress NTSC-U/C: 0x002b7fb8
 * @ghidraAddress PAL: 0x002c1f28
 */
void RtDmeSetClientAddress(int nConnection,
                           int nClient,
                           unsigned char uIndex,
                           const char *pszAddress);

/**
 * Report the kind of an address of a client.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param uIndex The address.
 * @return An RT_DME_ADDRESS_ kind, or 0 when it is not set.
 * @ghidraAddress NTSC-U/C: 0x002b8070
 * @ghidraAddress PAL: 0x002c1fe0
 */
unsigned char RtDmeGetClientAddressKind(int nConnection, int nClient, unsigned char uIndex);

/**
 * Set the kind of an address of a client.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param uIndex The address.
 * @param uKind An RT_DME_ADDRESS_ kind.
 * @ghidraAddress NTSC-U/C: 0x002b8110
 * @ghidraAddress PAL: 0x002c2080
 */
void RtDmeSetClientAddressKind(int nConnection,
                               int nClient,
                               unsigned char uIndex,
                               unsigned char uKind);

/**
 * Set the port of an address of a client.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param uIndex The address.
 * @param uPort The port.
 * @ghidraAddress NTSC-U/C: 0x002b81b8
 * @ghidraAddress PAL: 0x002c2128
 */
void RtDmeSetClientPort(int nConnection, int nClient, unsigned char uIndex, unsigned short uPort);

/**
 * Report the port of an address of a client.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param uIndex The address.
 * @return The port, or 0.
 * @ghidraAddress NTSC-U/C: 0x002b8260
 * @ghidraAddress PAL: 0x002c21d0
 */
unsigned short RtDmeGetClientPort(int nConnection, int nClient, unsigned char uIndex);

/**
 * Look up the latency record of a client, allocating it when first used.
 *
 * @param uClient The client.
 * @param ppStats Receives the record, or null.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b8308
 * @ghidraAddress PAL: 0x002c2278
 */
int RtDmeGetClientStats(unsigned int uClient, RtDmeClientStats **ppStats);

/**
 * Look up a connection.
 *
 * @param nConnection The connection.
 * @param ppConnection Receives the connection, null when it is not open.
 * @return 0, RT_DME_RESULT_BAD_CONNECTION, or RT_DME_RESULT_INVALID.
 * @ghidraAddress NTSC-U/C: 0x002b83f8
 * @ghidraAddress PAL: 0x002c2368
 */
int RtDmeGetConnection(unsigned int nConnection, RtDmeConnection **ppConnection);

/**
 * Look up a client.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param ppClient Receives the client, or null.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b8518
 * @ghidraAddress PAL: 0x002c2488
 */
int RtDmeGetClient(unsigned int nConnection, unsigned int nClient, RtDmeClient **ppClient);

/**
 * Look up a client, allocating its record when it has none.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param ppClient Receives the client, or null.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b85a0
 * @ghidraAddress PAL: 0x002c2510
 */
int RtDmeCreateClient(unsigned int nConnection, unsigned int nClient, RtDmeClient **ppClient);

/**
 * Report the address of the server of a connection.
 *
 * @param nConnection The connection.
 * @return The address, or null.
 * @ghidraAddress NTSC-U/C: 0x002b86d8
 * @ghidraAddress PAL: 0x002c2648
 */
char *RtDmeGetServerName(unsigned int nConnection);

/**
 * Report the flags a client announced.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param pnFlags Receives the flags.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b8788
 * @ghidraAddress PAL: 0x002c26f8
 */
int RtDmeGetClientFlags(unsigned int nConnection, int nClient, signed char *pnFlags);

/**
 * Set the client that keeps the clock of a connection.
 *
 * @param nConnection The connection.
 * @param nClient The client, or RT_DME_NO_CLIENT.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b88c0
 * @ghidraAddress PAL: 0x002c2830
 */
int RtDmeSetHostClient(unsigned int nConnection, int nClient);

/**
 * Drop a client of connection 0 that went silent.
 *
 * @param nClient The client.
 * @param uTimeout The milliseconds of silence that drop it.
 * @param bForce Non-zero to drop it at once.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b8ad8
 * @ghidraAddress PAL: 0x002c2a48
 */
int RtDmeExpireClient(int nClient, unsigned int uTimeout, unsigned char bForce);

/**
 * Finish the departure of a client of connection 0.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b95b8
 * @ghidraAddress PAL: 0x002c3528
 */
int RtDmeFinishDeparture(int nConnection, int nClient);

/**
 * Drop the peer connections of a client.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b96c0
 * @ghidraAddress PAL: 0x002c3630
 */
int RtDmeDropClientPeers(int nConnection, int nClient);

/**
 * Check a client index.
 *
 * @param nClient The client.
 * @return 1 when the index is below RT_DME_CLIENT_COUNT.
 * @ghidraAddress NTSC-U/C: 0x002b9b30
 * @ghidraAddress PAL: 0x002c3aa0
 */
int RtDmeIsClientIndex(unsigned int nClient);

/**
 * Narrow a client index to the byte the messages carry.
 *
 * @param nClient The client.
 * @return The byte.
 * @ghidraAddress NTSC-U/C: 0x002b9b38
 * @ghidraAddress PAL: 0x002c3aa8
 */
unsigned char RtDmeClientToByte(int nClient);

/**
 * Pick a new host for a connection.
 *
 * @param nConnection The connection.
 * @param nArgument1 The first argument of the election.
 * @param nArgument2 The second argument of the election.
 * @param nArgument3 The third argument of the election.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002bc930
 * @ghidraAddress PAL: 0x002c68a0
 */
int RtDmeElectHost(int nConnection, int nArgument1, int nArgument2, int nArgument3);

/**
 * Register the message types of the engine.
 *
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002bcbe8
 * @ghidraAddress PAL: 0x002c6b58
 */
int RtDmeRegisterSystemMessages(void);

/**
 * Clear the reassembly table of the long messages.
 *
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002be218
 * @ghidraAddress PAL: 0x002c8188
 */
int RtDmeClearReassembly(void);

#ifdef __cplusplus
}
#endif

#endif
