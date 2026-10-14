#ifndef RTDME_H
#define RTDME_H

#include <librtdme.h>

#include "rt/rtchannel.h"
#include "rt/rtringbuffer.h"
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
#define RT_DME_RESULT_FAILED 3          /*!< The connection failed. */
#define RT_DME_RESULT_OUT_OF_RANGE 8    /*!< An index is out of range. */
#define RT_DME_RESULT_NO_RECORD 10      /*!< A record or an address is missing. */
#define RT_DME_RESULT_ERROR_RECEIVED 12 /*!< A message received could not be dispatched. */
#define RT_DME_RESULT_SEND_FAILED 13    /*!< The channel did not take a message. */
#define RT_DME_RESULT_TIMED_OUT 14      /*!< An answer did not come in time. */
#define RT_DME_RESULT_BAD_CONNECTION 15 /*!< The connection index is out of range. */
#define RT_DME_RESULT_NOT_STARTED 17    /*!< The engine is not started. */
#define RT_DME_RESULT_STARTUP_FAILED 18 /*!< The channels did not start. */
#define RT_DME_RESULT_NO_MEMORY 22      /*!< An allocation failed. */
#define RT_DME_RESULT_INVALID 23        /*!< An argument is null or out of range. */
#define RT_DME_RESULT_SERVICE 24        /*!< The default for a failure of the channels. */
#define RT_DME_RESULT_NO_PROBE 25       /*!< Every latency probe is outstanding. */
#define RT_DME_RESULT_NO_SERVICE 26     /*!< The datagram service is not started. */
#define RT_DME_RESULT_FULL 28           /*!< The session has no room for this console. */
#define RT_DME_RESULT_HOST_FAILED 29    /*!< The default result of hosting. */
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
#define RT_DME_ADDRESS_INDEX_NONE 3       /*!< No address of the client responded. */

/** The flags a client announces. */
#define RT_DME_CLIENT_FLAG_DATAGRAMS 1 /*!< The client takes datagrams. */

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
 * Receiver of the result of a latency probe of the game.
 *
 * @param nResult The result, 0 or RT_DME_RESULT_TIMED_OUT.
 * @param pszAddress The address probed, in a buffer the receiver may write.
 * @param nLatency The milliseconds the answer took.
 * @param uRemoteTime The session time the answer carried.
 */
typedef void (*RtDmePingFunc)(int nResult,
                              char *pszAddress,
                              int nLatency,
                              unsigned int uRemoteTime);

/**
 * Receiver of the result of a latency probe of a client.
 *
 * @param nResult The result, 0 or RT_DME_RESULT_TIMED_OUT.
 * @param pszAddress The address probed, in a buffer the receiver may write.
 * @param nLatency The milliseconds the answer took.
 * @param uRemoteTime The session time the answer carried.
 * @param nConnection The connection of the client.
 * @param nClient The client.
 */
typedef void (*RtDmeClientPingFunc)(int nResult,
                                    char *pszAddress,
                                    int nLatency,
                                    unsigned int uRemoteTime,
                                    int nConnection,
                                    int nClient);

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
    int anState[2];                               /*!< The two state values the client announces. */
    unsigned short uObjectCount;   /*!< The persistent objects the client announced. */
    unsigned char abReserved3e[2]; /* +0x3e */
    int nHostRank;                 /*!< The rank of a candidate host, lowest first, 0 for none. */
    RtTime lastProbe;              /*!< When the addresses of the client were last probed. */
    int nProbeCount;               /*!< The probes of the addresses not answered. */
    unsigned char abReserved50[8]; /* +0x50 */
    unsigned char uAddressIndex;   /*!< An RT_DME_ADDRESS_INDEX_ value. */
    unsigned char bJoined;         /*!< Non-zero once the client joined. */
    RtTime lastHeard;              /*!< When the client joined or last sent a datagram. */
    unsigned char bCleared;        /*!< Set when the record is cleared. */
    unsigned char bObjectsJoined;  /*!< Set once the objects of the client were claimed. */
    signed char nFlags;            /*!< The flags the client announces, bit 0 for datagrams. */
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
    int nNextSample;                                /*!< The sample the next result replaces. */
    RtTime lastProbe;                               /*!< When the client was last probed. */
} RtDmeClientStats;

/** The size of the name of an object range, the terminator included. */
#define RT_DME_RANGE_NAME_SIZE 14

/** The objects and the stream slots a client owns. 28 bytes. */
typedef struct {
    unsigned int uJoinTime;              /*!< The session time the client claimed them at. */
    unsigned short uObjectLimit;         /*!< The objects of the session, set when opened. */
    unsigned short uFirstObject;         /*!< The first object of the client. */
    unsigned short uObjectCount;         /*!< The number of objects of the client. */
    unsigned short uFirstStream;         /*!< The first stream slot of the client. */
    unsigned short uStreamCount;         /*!< The number of stream slots of the client. */
    char szName[RT_DME_RANGE_NAME_SIZE]; /*!< The name of the range. */
} RtDmeObjectRange;

/** The lifetimes of an object that outlive its owner. */
#define RT_DME_OBJECT_LIFETIME_SESSION 3 /*!< The object passes to the host. */
#define RT_DME_OBJECT_LIFETIME_HOST 4    /*!< The object passes to the host. */

/** The number of fields of an object type. */
#define RT_DME_OBJECT_FIELD_COUNT 64

/** A shared object. */
typedef struct {
    unsigned char uType;              /*!< The type, which selects the receivers. */
    unsigned char uFilter;            /*!< The filter of the clients that see the object, or 0. */
    unsigned char uLifetime;          /*!< What happens to the object when its owner leaves. */
    unsigned char uFlags;             /*!< A value sent with the object and not read. */
    int nLockClient;                  /*!< The client that holds the object, or -1. */
    int nOwner;                       /*!< The client that owns the object. */
    char szName[RT_DME_ADDRESS_SIZE]; /*!< The name of the object. */
    unsigned char *pbData;            /*!< The data of the object. */
    unsigned char *pbShadow;          /*!< The copy of the data last sent. */
    int nReserved24;                  /* +0x24 */
    unsigned int uFullInterval;       /*!< The milliseconds between full sends, or 0. */
    unsigned int uLastFullTime;       /*!< The session time of the last full send, 0 for none. */
    /** The session time the object was last sent whole to each client. */
    unsigned int auClientSendTime[RT_DME_CLIENT_COUNT];
    /** The session time each field was last sent. */
    unsigned int auFieldSendTime[RT_DME_OBJECT_FIELD_COUNT];
    int nReserved530; /* +0x530 */
} RtDmeObject;

/** The arguments of the test of a field that the game provides. */
typedef struct {
    int nObject;                /*!< The object. */
    int nField;                 /*!< The field. */
    int nElementSize;           /*!< The size of an element of the field. */
    int nElementCount;          /*!< The elements of the field. */
    const unsigned char *pbNew; /*!< The current value. */
    const unsigned char *pbOld; /*!< The value last sent. */
} RtDmeFieldTest;

/**
 * Test of a field that the game provides.
 *
 * @param pTest The field.
 * @return Non-zero in its low byte when the field changed enough to be sent.
 */
typedef int (*RtDmeFieldTestFunc)(const RtDmeFieldTest *pTest);

/** How the values of a field are compared, an index of RtDmeObjectField::nCompare. */
#define RT_DME_COMPARE_S8 0         /*!< A signed byte. */
#define RT_DME_COMPARE_U8 1         /*!< An unsigned byte. */
#define RT_DME_COMPARE_S16 2        /*!< A signed halfword. */
#define RT_DME_COMPARE_U16 3        /*!< An unsigned halfword. */
#define RT_DME_COMPARE_S32 4        /*!< A signed word. */
#define RT_DME_COMPARE_U32 5        /*!< An unsigned word. */
#define RT_DME_COMPARE_FLOAT 6      /*!< A float. */
#define RT_DME_COMPARE_DOUBLE 7     /*!< A double. */
#define RT_DME_COMPARE_S16_2D 8     /*!< A point of 2 halfwords, by distance. */
#define RT_DME_COMPARE_S16_3D 9     /*!< A point of 3 halfwords, by distance. */
#define RT_DME_COMPARE_S32_2D 10    /*!< A point of 2 words, by distance. */
#define RT_DME_COMPARE_S32_3D 11    /*!< A point of 3 words, by distance. */
#define RT_DME_COMPARE_FLOAT_2D 12  /*!< A point of 2 floats, by distance. */
#define RT_DME_COMPARE_FLOAT_3D 13  /*!< A point of 3 floats, by distance. */
#define RT_DME_COMPARE_DOUBLE_2D 14 /*!< A point of 2 doubles, by distance. */
#define RT_DME_COMPARE_DOUBLE_3D 15 /*!< A point of 3 doubles, by distance. */
#define RT_DME_COMPARE_COUNT 16     /*!< The number of comparisons. */

/** When a field is sent, the values of RtDmeObjectField::nMode. */
#define RT_DME_FIELD_NEVER 0   /*!< The field is not sent. */
#define RT_DME_FIELD_CHANGED 1 /*!< The field is sent when it changed at all. */
#define RT_DME_FIELD_CUSTOM 5  /*!< The test of the game decides. */

/** A field of an object type. */
typedef struct {
    int nReserved00;        /* +0x00 */
    int nElementSize;       /*!< The size of an element in bytes. */
    int nElementCount;      /*!< The elements of the field. */
    int nCompare;           /*!< An RT_DME_COMPARE_ value. */
    unsigned int uInterval; /*!< The milliseconds between sends of the field. */
    int nMode;              /*!< An RT_DME_FIELD_ value, or another for a threshold. */
    union {
        double dThreshold;          /*!< The change that sends the field. */
        RtDmeFieldTestFunc pfnTest; /*!< The test of RT_DME_FIELD_CUSTOM. */
    } test;                         /*!< The test of the field. */
    signed char nSendFlags;         /*!< The send flags the field adds to an update. */
} RtDmeObjectField;

/** An object type. 552 bytes. */
typedef struct {
    int nDataSize;                  /*!< The size of the data of an object. */
    int nFieldCount;                /*!< The fields. */
    unsigned char abReserved08[32]; /* +0x08 */
    RtDmeObjectField *apFields[RT_DME_OBJECT_FIELD_COUNT]; /*!< The fields, null where none. */
    int anFieldOffsets[RT_DME_OBJECT_FIELD_COUNT];         /*!< The offsets of the fields. */
} RtDmeObjectType;

/**
 * Filter of the clients that see an object.
 *
 * @param nClient The client.
 * @param nObject The object.
 * @return Non-zero when the client sees the object.
 */
typedef int (*RtDmeObjectFilterFunc)(int nClient, int nObject);

/** The states of a stream slot. */
#define RT_DME_STREAM_FREE 0     /*!< The slot is free. */
#define RT_DME_STREAM_RESERVED 1 /*!< The slot belongs to a client. */
#define RT_DME_STREAM_OPEN 2     /*!< The slot carries a stream. */

/** The number of kinds of stream. */
#define RT_DME_STREAM_KIND_COUNT 16

/** A slot of an object stream. 68 bytes. */
typedef struct {
    unsigned char uState;          /*!< An RT_DME_STREAM_ value. */
    unsigned char uKind;           /*!< A value sent with the stream. */
    unsigned char bOwnsData;       /*!< Non-zero when the slot allocated its data. */
    unsigned char uRoute;          /*!< The kind of the stream, an index of g_anDmeStreamRelay. */
    unsigned char bClosing;        /*!< Set to close the stream once it is sent. */
    unsigned char bToHost;         /*!< Non-zero to send the stream through the host. */
    unsigned char abReserved06[2]; /* +0x06 */
    int nReceiver;                 /*!< The client the stream goes to. */
    int nReserved0c;               /* +0x0c */
    unsigned char *pbData;         /*!< The data of the stream. */
    unsigned char *pbStart;        /*!< A position in the data, sent as an offset. */
    unsigned char *pbEnd;          /*!< A position in the data, sent as an offset. */
    int nReserved1c;               /* +0x1c */
    int nRate;                     /*!< The rate the stream is sent at. */
    unsigned short uReserved24;    /* +0x24 */
    unsigned short uMaxChunk;      /*!< The most bytes sent at once. */
    unsigned int uLastSendTime;    /*!< The session time of the last send. */
    RtRingBuffer queue;            /*!< The bytes not sent yet. */
} RtDmeStream;

/** The arguments of the receiver of the objects of a client. */
typedef struct {
    int nEvent;      /*!< 0 when the client claimed its objects, 1 when it released them. */
    int nConnection; /*!< The connection. */
    int nClient;     /*!< The client. */
    int nArgument;   /*!< The argument given with the receiver. */
} RtDmeObjectRangeEvent;

/**
 * Receiver of a client that claimed or released its objects.
 *
 * @param pEvent The event.
 */
typedef void (*RtDmeObjectRangeFunc)(const RtDmeObjectRangeEvent *pEvent);

/** The object ranges of a client, as the system message carries it. 36 bytes. */
typedef struct {
    unsigned char uClient;               /*!< The client. */
    unsigned char uReserved01;           /* +0x01, a registered field, sent uninitialised */
    unsigned char abReserved02[2];       /* +0x02, sent uninitialised */
    unsigned int uReserved04;            /* +0x04, a registered field, sent uninitialised */
    unsigned short uObjectLimit;         /*!< The objects of the session. */
    unsigned short uFirstObject;         /*!< The first object. */
    unsigned short uObjectCount;         /*!< The number of objects. */
    unsigned short uFirstStream;         /*!< The first stream slot. */
    unsigned short uStreamCount;         /*!< The number of stream slots. */
    unsigned char abReserved12[4];       /* +0x12, sent uninitialised */
    char szName[RT_DME_RANGE_NAME_SIZE]; /*!< The name of the range. */
} RtDmeObjectRangeMessage;

/** The state of a client, as the system message carries it. 12 bytes. */
typedef struct {
    unsigned char uRole;           /*!< The role of the client. */
    unsigned char uClient;         /*!< The client. */
    unsigned char auState[2];      /*!< The two state values of the client. */
    unsigned short uObjectCount;   /*!< The persistent objects the client owns. */
    unsigned char abReserved06[2]; /* +0x06, sent uninitialised */
    int nHostRank;                 /*!< The rank of the client as a candidate host. */
} RtDmeClientStateMessage;

/** The port of the first voice device. */
#define RT_DME_VOICE_FIRST_PORT 10076

/** The voice channels of a device. */
#define RT_DME_VOICE_CHANNEL_COUNT 16

/** The voice address of a client, as the system message carries it. 28 bytes. */
typedef struct {
    int nConnection;            /*!< The connection. */
    int nClient;                /*!< The client. */
    unsigned int uLocalAddress; /*!< The private address. */
    int nLocalPort;             /*!< The private port. */
    unsigned int uAddress;      /*!< The public address. */
    int nPort;                  /*!< The public port. */
    int nReserved18;            /*!< A further value of the device. */
} RtDmeVoicePeerMessage;

/**
 * @name The voice device.
 * These routines belong to the device layer of the library, ported separately.
 * @{
 */

/** A voice device. */
typedef struct RtDmeDevice RtDmeDevice;

/** The voice address of a device. 24 bytes. */
typedef struct {
    unsigned int uAddress;      /*!< The public address. */
    unsigned int uLocalAddress; /*!< The private address. */
    int nPort;                  /*!< The public port. */
    int nLocalPort;             /*!< The private port. */
    int nReserved10;            /*!< A further value of the device. */
    int nClient;                /*!< The client the device belongs to. */
} RtDmeDevicePeer;

/** The status a device reports. 24 bytes. */
typedef struct {
    int anReserved[5]; /* +0x00, not read by the engine */
    int nLevel;        /*!< The level of the device. */
} RtDmeDeviceStatus;

/** Receiver of the events of a device. */
typedef int (*RtDmeDeviceEventFunc)(RtDmeDevice *pDevice, int *pnEvent, int nValue1, int nValue2);
/** Receiver of the state changes of a device. */
typedef int (*RtDmeDeviceStateFunc)(RtDmeDevice *pDevice, int nState, int nValue1, int nValue2);
/** Receiver of the data of a device. */
typedef int (*RtDmeDeviceDataFunc)(RtDmeDevice *pDevice, int nValue1, int nValue2, int nValue3);
/** Receiver of the end of the data of a device. */
typedef int (*RtDmeDeviceEndFunc)(RtDmeDevice *pDevice, int nValue1, int nValue2);
/** Receiver of the start of the stream of a device. */
typedef int (*RtDmeDeviceStreamFunc)(RtDmeDevice *pDevice, unsigned int uAddress, int nPort);

/** The settings of a device. 52 bytes. */
typedef struct {
    int nPort;                     /*!< The port. */
    int anFormat[6];               /*!< The format values. */
    int anDevice[2];               /*!< Further values. */
    RtDmeDeviceEventFunc pfnEvent; /*!< Receives the events, or null. */
    RtDmeDeviceStateFunc pfnState; /*!< Receives the state changes, or null. */
    RtDmeDeviceDataFunc pfnData;   /*!< Receives the data, or null. */
    RtDmeDeviceEndFunc pfnEnd;     /*!< Receives the end of the data, or null. */
} RtDmeDeviceSettings;

/**
 * Allocate a device.
 *
 * @return The device.
 * @ghidraAddress NTSC-U/C: 0x002e4840
 */
RtDmeDevice *RtDmeDeviceAlloc(void);

/**
 * Release a device.
 *
 * @param pDevice The device.
 * @ghidraAddress NTSC-U/C: 0x002e48c4
 */
void RtDmeDeviceDestroy(RtDmeDevice *pDevice);

/**
 * Start a device.
 *
 * @param pDevice The device.
 * @param pSettings The settings.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002e4934
 */
int RtDmeDeviceInit(RtDmeDevice *pDevice, const RtDmeDeviceSettings *pSettings);

/**
 * Reach a device through a gateway.
 *
 * @param pDevice The device.
 * @param pszAddress The address of the gateway.
 * @param nPort The port of the gateway.
 * @param pfnStream Receives the start of the stream.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002e4e34
 */
int RtDmeDeviceConnect(RtDmeDevice *pDevice,
                       const char *pszAddress,
                       int nPort,
                       RtDmeDeviceStreamFunc pfnStream);

/**
 * Stop a device.
 *
 * @param pDevice The device.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002e4fbc
 */
int RtDmeDeviceStop(RtDmeDevice *pDevice);

/**
 * Run a device once.
 *
 * @param pDevice The device.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002e501c
 */
int RtDmeDeviceUpdate(RtDmeDevice *pDevice);

/**
 * Open a named channel of a device.
 *
 * @param pDevice The device.
 * @param nMode The mode.
 * @param pnChannel Receives the channel.
 * @param pszName The name.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002e541c
 */
int RtDmeDeviceOpenChannel(RtDmeDevice *pDevice, int nMode, int *pnChannel, const char *pszName);

/**
 * Select the channel of a device that sends.
 *
 * @param pDevice The device.
 * @param nChannel The channel.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002e6d9c
 */
int RtDmeDeviceSelectChannel(RtDmeDevice *pDevice, int nChannel);

/**
 * Set the client of a device.
 *
 * @param pDevice The device.
 * @param nClient The client.
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002e6e84
 */
int RtDmeDeviceSetClient(RtDmeDevice *pDevice, int nClient);

/**
 * Copy the voice address of a device.
 *
 * @param pDevice The device.
 * @param pPeer Receives the address.
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002e6e98
 */
int RtDmeDeviceGetPeer(RtDmeDevice *pDevice, RtDmeDevicePeer *pPeer);

/**
 * Add a peer to a device.
 *
 * @param pDevice The device.
 * @param pPeer The address of the peer.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002e6ed4
 */
int RtDmeDeviceAddPeer(RtDmeDevice *pDevice, const RtDmeDevicePeer *pPeer);

/**
 * Remove a peer from a device.
 *
 * @param pDevice The device.
 * @param nClient The client of the peer.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002e6fa0
 */
int RtDmeDeviceRemovePeer(RtDmeDevice *pDevice, int nClient);

/**
 * Report the status of a device.
 *
 * @param pDevice The device.
 * @param pStatus Receives the status.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002e7094
 */
int RtDmeDeviceGetStatus(RtDmeDevice *pDevice, RtDmeDeviceStatus *pStatus);

/** @} */

/** A set of clients, as a bitmap with its first and last members. */
typedef struct {
    /** One bit per client, and a spare byte the engine clears with them. */
    unsigned char abBits[RT_DME_CLIENT_COUNT / 8 + 1];
    int nFirst; /*!< The lowest client of the set. */
    int nLast;  /*!< The highest client of the set. */
} RtDmeClientSet;

/** An outstanding latency probe. */
typedef struct {
    unsigned char abAddress[4]; /*!< The address probed. */
    unsigned short uPort;       /*!< The port probed. */
    int bClientProbe; /*!< 1 when the engine probes a client, 0 for a probe of the game. */
    union {
        RtDmePingFunc pfnDone;             /*!< Receives the result of a probe of the game. */
        RtDmeClientPingFunc pfnClientDone; /*!< Receives the result of a probe of a client. */
    } done;                                /*!< The receiver, null when the slot is free. */
    unsigned int uSentTime;                /*!< When the probe was sent, in milliseconds. */
    int nConnection;                       /*!< The connection of the client probed. */
    int nClient;                           /*!< The client probed, or RT_DME_NO_CLIENT. */
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

/** Non-zero while the engine is started. */
extern int g_nDmeRunning;
/** The connection the engine serves. */
extern int g_nDmeActiveConnection;
/** The objects in use, as an upper bound of their indices. */
extern int g_nDmeObjectCount;
/** Receives the result of the next connection, then is cleared. */
extern RtDmeConnectFunc g_pfnDmeConnect;
/** Receives the changes of host of connection 0. */
extern rtDmeHostChangeCallback g_pfnDmeHostChange;
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
extern rtDmeAddressResult g_dmeMapping;
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
/** The system message that announces the voice address of a client. */
extern int g_nDmeVoicePeerMessage;
/** The system message that announces the object ranges of a client. */
extern int g_nDmeObjectRangeMessage;
/** The system message that announces the state of a client. */
extern int g_nDmeClientStateMessage;
/** Receives the clients that claimed or released their objects. */
extern RtDmeObjectRangeFunc g_pfnDmeObjectRange;
/** The argument given to g_pfnDmeObjectRange. */
extern int g_nDmeObjectRangeArgument;
/** Receives the hosts of the objects of connection 0 that a departure picked. */
extern rtDmeHostChangeCallback g_pfnDmeObjectHost;
/** The argument given to g_pfnDmeObjectHost. */
extern int g_nDmeObjectHostArgument;
/** Receives the result of the last port mapping. */
extern rtDmeAddressCallback g_pfnDmeMapping;
/** Non-zero once the session of the active connection was reported ready. */
extern unsigned char g_bDmeObjectRangesSent;
/** Non-zero while the objects of the active connection are shared. */
extern unsigned char g_bDmeObjectsShared;
/** Receives the statuses of the server of the next connection. */
extern RtDmeConnectFunc g_pfnDmeStatus;
/** Non-zero while rtDmeConnect() waits for a peer-to-peer connection. */
extern unsigned char g_bDmeConnectWaiting;
/** The gateway the port mappings of the peers go through. */
extern rtNetAddress g_dmeGatewayAddress;
/** The object types. */
extern RtDmeObjectType g_aDmeObjectTypes[RT_DME_OBJECT_TYPE_COUNT];
/** Non-zero for the kinds of stream that a client other than the host sends through the host. */
extern int g_anDmeStreamRelay[RT_DME_STREAM_KIND_COUNT];
/** The filters of the clients that see an object, by RtDmeObject::uFilter. */
extern RtDmeObjectFilterFunc g_apfnDmeObjectFilter[RT_DME_OBJECT_TYPE_COUNT];
/** The system message of a client that left the session. */
extern int g_nDmeClientLeftMessage;
/** The system message of a latency probe and its answer. */
extern int g_nDmePingMessage;
/** The system message of a whole object. */
extern int g_nDmeObjectMessage;
/** The system message of the changed fields of an object. */
extern int g_nDmeObjectUpdateMessage;
/** The system message of a stream that closed. */
extern int g_nDmeStreamClosedMessage;
/** The system message of the data of a stream. */
extern int g_nDmeStreamDataMessage;
/** The system message that probes the addresses of a client. */
extern int g_nDmeAddressProbeMessage;
/** The system message of a console that disconnects. */
extern int g_nDmeDisconnectMessage;
/** The system message that destroys an object. */
extern int g_nDmeObjectDestroyMessage;
/** The system message that asks the owner of an object to lend it. */
extern int g_nDmeObjectLockMessage;
/** The system message that announces the client that holds an object. */
extern int g_nDmeObjectLockedMessage;
/** The system message that announces an object nobody holds. */
extern int g_nDmeObjectUnlockMessage;
/** The system message that asks whether a console hosts a session. */
extern int g_nDmeHostQueryMessage;
/** The system message that answers g_nDmeHostQueryMessage. */
extern int g_nDmeHostReplyMessage;
/** The system message that asks the host for the objects and the stream slots of a client. */
extern int g_nDmeObjectRequestMessage;
/** The system message of the host that grants a client its objects and stream slots. */
extern int g_nDmeObjectGrantMessage;
/** The system message that reports a client ready to the host. */
extern int g_nDmeClientReadyMessage;
/** The system message that refuses a console a place in the session. */
extern int g_nDmeSessionFullMessage;
/** The system message that introduces a client and its addresses. */
extern int g_nDmeClientInfoMessage;
/** The system message of the name of a session. */
extern int g_nDmeSessionNameMessage;
/** The system message of the clock of the host. */
extern int g_nDmeClockMessage;
/** The system message of the rank of a client as a candidate host. */
extern int g_nDmeHostRankMessage;
/** A system message the engine receives and ignores. */
extern int g_nDmeIgnoredMessage;

/**
 * Receiver of the data of a stream.
 *
 * @param nSender The client that sent the data.
 * @param nStream The stream.
 * @param pbData The data.
 * @param nSize The size of the data.
 */
typedef void (*RtDmeStreamDataFunc)(int nSender,
                                    int nStream,
                                    const unsigned char *pbData,
                                    int nSize);

/**
 * Receiver of a stream that closed.
 *
 * @param nSender The client that closed the stream.
 * @param nStream The stream.
 */
typedef void (*RtDmeStreamClosedFunc)(int nSender, int nStream);

/**
 * Receiver of the answer of a host to g_nDmeHostQueryMessage.
 *
 * @param pszAddress The address of the host.
 * @param nPort The port the host answered with.
 */
typedef void (*RtDmeHostReplyFunc)(const char *pszAddress, int nPort);

/** The receivers of the data of the streams, by RtDmeStream::uRoute. */
extern RtDmeStreamDataFunc g_apfnDmeStreamData[RT_DME_STREAM_KIND_COUNT];
/** The receivers of the streams that closed, by RtDmeStream::uRoute. */
extern RtDmeStreamClosedFunc g_apfnDmeStreamClosed[RT_DME_STREAM_KIND_COUNT];
/** The private address this console announces when g_bDmeUseFixedAddress is set. */
extern unsigned char g_abDmeFixedAddress[4];
/** Receives the answers to g_nDmeHostQueryMessage, or null. */
extern RtDmeHostReplyFunc g_pfnDmeHostReply;
/**
 * How the objects of a session are shared. 1 and 2 let a new host be elected, and 1 also leaves
 * the objects when the host grants them.
 */
extern int g_nDmeObjectMode;

/** The arguments of the receiver of a session that became ready. */
typedef struct {
    int nEvent;    /*!< 0 when the session is ready, otherwise the code of the failure. */
    int nArgument; /*!< The argument given with the receiver. */
} RtDmeSessionReady;

/**
 * Receiver of a session that became ready.
 *
 * @param pReady The event.
 */
typedef void (*RtDmeSessionReadyFunc)(const RtDmeSessionReady *pReady);

/** Receives the next session that becomes ready, then is cleared. */
extern RtDmeSessionReadyFunc g_pfnDmeSessionReady;
/** The argument given to g_pfnDmeSessionReady. */
extern int g_nDmeSessionReadyArgument;

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
 * Set the defaults of the voice settings, with the first free port of the devices.
 *
 * @param pSettings The settings.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ae120
 */
int RtDmeVoiceInitSettings(rtDmeVoiceSettings *pSettings);

/**
 * Run the device of the voice of a connection once.
 *
 * @param nConnection The connection.
 * @param bStarted The started flag the voice has while its device runs.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ae1c0
 */
int RtDmeVoiceUpdate(int nConnection, unsigned char bStarted);

/**
 * Open the voice of a connection and its device.
 *
 * @param nConnection The connection.
 * @param nMaxClients The clients the connection takes.
 * @param pSettings The settings.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ae248
 */
int RtDmeVoiceOpen(int nConnection, int nMaxClients, const rtDmeVoiceSettings *pSettings);

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
 * Add the voice address a client announced to the voice of its connection.
 *
 * @param pMessage The announcement.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002af220
 */
int RtDmeVoiceAddPeer(const RtDmeVoicePeerMessage *pMessage);

/**
 * Allocate the object ranges of the clients.
 *
 * @ghidraAddress NTSC-U/C: 0x002afc20
 * @ghidraAddress PAL: 0x002b9b90
 */
void RtDmeInitObjectRanges(void);

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
 * Allocate the object range of a client.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @return The range, or null for a bad index.
 * @ghidraAddress NTSC-U/C: 0x002afcd8
 */
RtDmeObjectRange *RtDmeCreateObjectRange(unsigned int nConnection, unsigned int nClient);

/**
 * Release the object ranges of the clients of a connection.
 *
 * @param nConnection The connection.
 * @ghidraAddress NTSC-U/C: 0x002afdf0
 */
void RtDmeFreeConnectionObjectRanges(unsigned int nConnection);

/**
 * Release the object ranges of every connection.
 *
 * @ghidraAddress NTSC-U/C: 0x002afe58
 */
void RtDmeFreeAllObjectRanges(void);

/**
 * Release the table of the object ranges.
 *
 * @ghidraAddress NTSC-U/C: 0x002afea8
 */
void RtDmeFreeObjectRangeTable(void);

/**
 * Send the object range of the local client to a client, or to every client.
 *
 * @param nConnection The connection.
 * @param nClient The client, or RT_DME_ALL_CLIENTS.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002affb8
 */
int RtDmeSendObjectRange(unsigned int nConnection, int nClient);

/**
 * Open the object range of a client, announce it, and claim its objects.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @ghidraAddress NTSC-U/C: 0x002b00f0
 */
void RtDmeOpenObjectRange(unsigned int nConnection, int nClient);

/**
 * Claim the objects of a client that joined, and send it the object range of this console.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @ghidraAddress NTSC-U/C: 0x002b0188
 */
void RtDmeJoinObjects(unsigned int nConnection, int nClient);

/**
 * Release the objects of a client, passing the persistent ones to the host.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @ghidraAddress NTSC-U/C: 0x002b0290
 * @ghidraAddress PAL: 0x002ba200
 */
void RtDmeReleaseClientObjects(unsigned int nConnection, int nClient);

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
 * Receive the result of the port mapping of the peers.
 *
 * @param pszAddress The public address.
 * @param uPort The public port.
 * @param nResult The result of the mapping.
 * @ghidraAddress NTSC-U/C: 0x002b0980
 */
void RtDmeOnPortMapped(const char *pszAddress, unsigned short uPort, int nResult);

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
int RtDmeSetHostChangeHook(rtDmeHostChangeCallback pfnHook, int nArgument);

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
 * Set the defaults of the parameters of RtDmeConnect().
 *
 * @param pParams The parameters.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b40b0
 */
int RtDmeInitConnectParams(rtDmeConnectParams *pParams);

/**
 * Connect to a session, over a channel for the types 1 and 3 or as datagrams for the type 2.
 *
 * @param pParams The parameters.
 * @param pStatus Receives the status.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b4130
 */
int RtDmeConnect(const rtDmeConnectParams *pParams, rtDmeConnectStatus *pStatus);

/**
 * Set the defaults of the parameters of RtDmeHost().
 *
 * @param pParams The parameters.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b4a70
 */
int RtDmeInitHostParams(rtDmeHostParams *pParams);

/**
 * Host a peer-to-peer session on connection 0.
 *
 * @param pParams The parameters.
 * @param pStatus Receives the status.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b4ad0
 */
int RtDmeHost(const rtDmeHostParams *pParams, rtDmeConnectStatus *pStatus);

/**
 * Set the defaults of an address query.
 *
 * @param pQuery The query.
 * @return 0, or RT_DME_RESULT_INVALID.
 * @ghidraAddress NTSC-U/C: 0x002b4e10
 */
int RtDmeInitAddressQuery(rtDmeAddressQuery *pQuery);

/**
 * Learn the public address of this console, by a port mapping through the gateway.
 *
 * @param pQuery The query.
 * @param pStatus Receives the status.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b4e50
 */
int RtDmeQueryAddresses(const rtDmeAddressQuery *pQuery, rtDmeConnectStatus *pStatus);

/**
 * Set the defaults of an address report.
 *
 * @param pReport The report.
 * @return 0, or RT_DME_RESULT_INVALID.
 * @ghidraAddress NTSC-U/C: 0x002b4fc0
 */
int RtDmeInitAddressReport(rtDmeAddressReport *pReport);

/**
 * Add a peer the server reported to connection 0, when this console is the host.
 *
 * @param pReport The addresses of the peer.
 * @param pStatus Receives the status.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b5000
 */
int RtDmeReportAddresses(const rtDmeAddressReport *pReport, rtDmeConnectStatus *pStatus);

/**
 * Add a peer to connection 0 under its addresses, replacing a client with the same addresses.
 *
 * @param pReport The addresses of the peer.
 * @param pStatus Receives the status.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b50f0
 */
int RtDmeAddReportedPeer(const rtDmeAddressReport *pReport, rtDmeConnectStatus *pStatus);

/**
 * Enable or disable the messages of a connection for the local client, and announce it.
 *
 * @param nConnection The connection.
 * @param bEnable Non-zero to enable the messages.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b5468
 */
int RtDmeEnableConnection(int nConnection, int bEnable);

/**
 * Leave the objects of the session of a connection, keeping the connection.
 *
 * @param nConnection The connection.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b55a8
 */
int RtDmeLeaveSession(int nConnection);

/**
 * Probe the latency of a client of the active connection.
 *
 * @param nClient The client.
 * @ghidraAddress NTSC-U/C: 0x002b58f8
 */
void RtDmeProbeClient(unsigned int nClient);

/**
 * Run the engine once: the channels, the clock, the objects, the probes, and the peers.
 *
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b7728
 */
int RtDmeUpdate(void);

/**
 * Close a connection and clear its clients.
 *
 * @param nConnection The connection.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b7980
 */
int RtDmeDisconnect(int nConnection);

/**
 * Stop the engine and release everything it holds.
 *
 * @return 0, or RT_DME_RESULT_NOT_STARTED.
 * @ghidraAddress NTSC-U/C: 0x002b7bb0
 */
int RtDmeShutdown(void);

/**
 * Set the flags of a client.
 *
 * @param nConnection The connection.
 * @param nClient The client.
 * @param nFlags The flags.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b8828
 */
int RtDmeSetClientFlags(unsigned int nConnection, int nClient, signed char nFlags);

/**
 * Report whether this console is the host of connection 0.
 *
 * @param pbHost Receives 1 when it is the host.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b8ee8
 */
int RtDmeIsLocalHost(unsigned char *pbHost);

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
 * @param bAnnounce Non-zero to announce the departure to the other clients.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002b8ad8
 * @ghidraAddress PAL: 0x002c2a48
 */
int RtDmeExpireClient(int nClient, unsigned int uTimeout, unsigned char bAnnounce);

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
 * Check a connection index.
 *
 * @param nConnection The connection.
 * @return 1 when the index is below RT_DME_CONNECTION_COUNT.
 * @ghidraAddress NTSC-U/C: 0x002b9b40
 */
int RtDmeIsConnectionIndex(unsigned int nConnection);

/**
 * Report the address of the gateway the voice of the peers goes through.
 *
 * @param ppAddress Receives the address.
 * @return 0, or RT_DME_RESULT_INVALID.
 * @ghidraAddress NTSC-U/C: 0x002b9b48
 */
int RtDmeGetGatewayAddress(rtNetAddress **ppAddress);

/**
 * Pick a new host for a connection when this console is the first joined candidate, and take
 * over the objects of the session.
 *
 * @param nConnection The connection.
 * @param nClient Not read.
 * @param nState0 Not read.
 * @param nState1 Not read.
 * @ghidraAddress NTSC-U/C: 0x002bc930
 * @ghidraAddress PAL: 0x002c68a0
 */
void RtDmeElectHost(int nConnection, int nClient, int nState0, int nState1);

/**
 * Register the message types of the engine.
 *
 * @return The result of the last registration.
 * @ghidraAddress NTSC-U/C: 0x002bcbe8
 * @ghidraAddress PAL: 0x002c6b58
 */
int RtDmeRegisterSystemMessages(void);

/**
 * Start the field list of a message type.
 *
 * @return 0, or RT_DME_RESULT_NOT_STARTED.
 * @ghidraAddress NTSC-U/C: 0x002a8d38
 */
int RtDmeBeginFields(void);

/**
 * Append a field to the field list of a message type. The list stores the record address minus
 * the field address with the two numbers as given, and the engine does not read them back.
 *
 * @param pRecord The address of a record of the type.
 * @param pField The address of the field in the record.
 * @param nSize A positive number stored with the field, the size of an element by convention.
 * @param nCount A positive number stored with the field, the count of elements by convention.
 * @return 0, RT_DME_RESULT_NOT_STARTED, or RT_DME_RESULT_INVALID.
 * @ghidraAddress NTSC-U/C: 0x002a8d60
 */
int RtDmeAddField(const void *pRecord, const void *pField, int nSize, int nCount);

/**
 * Register a message type of a class with the field list and its receiver.
 *
 * @param pnType Receives the registered type.
 * @param nClass The class.
 * @param pfnHandler The receiver.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002a8df8
 */
int RtDmeRegisterMessageType(int *pnType, unsigned int nClass, rtDmeMessageHandler pfnHandler);

/**
 * Register a message type of the game with the field list and its receiver.
 *
 * @param pnType Receives the registered type.
 * @param pfnHandler The receiver.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002a8ed8
 */
int RtDmeRegisterApplicationMessage(int *pnType, rtDmeMessageHandler pfnHandler);

/**
 * Free the long messages whose parts stopped arriving for 30 seconds.
 *
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002be128
 */
int RtDmeExpireReassembly(void);

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
