#ifndef LIBRTDME_H
#define LIBRTDME_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Peer-to-peer layer of the SCE-RT network library, which carries the messages of an online
 * session between the consoles. Every entry point takes the lock of the library and reports 0 for
 * success or an error code. The names of the entry points and of their parameters are inferred.
 */

/** The size of the text of an address of a peer, the terminator included. */
#define RT_NET_ADDRESS_SIZE 16

/** The number of addresses a connection lists. */
#define RT_NET_ADDRESS_COUNT 2

/** The size of the public key of a server. */
#define RT_NET_SERVER_KEY_SIZE 64

/** A key of 512 bits. A key may lie at any address of a packet, so it has no alignment. */
typedef union __attribute__((packed)) {
    unsigned char bytes[RT_NET_SERVER_KEY_SIZE];                       /*!< The bytes. */
    unsigned int words[RT_NET_SERVER_KEY_SIZE / sizeof(unsigned int)]; /*!< The words. */
} rtNetKey;

/** The size of the session and access keys, the terminator included. */
#define RT_NET_KEY_SIZE 17

/** Bits of the flags of rtDmeSend(). */
#define RT_DME_SEND_BASE 0x80       /*!< Set in every message. */
#define RT_DME_SEND_ORDERED 0x10    /*!< The message keeps its order. */
#define RT_DME_SEND_GUARANTEED 0x40 /*!< The message is resent until it arrives. */

/** The size of the bytes of a message of the game. */
#define RT_DME_MESSAGE_DATA_SIZE 0x200

/** A message of the game as the library carries it. */
typedef struct {
    unsigned short length;               /*!< The number of bytes of data. */
    char data[RT_DME_MESSAGE_DATA_SIZE]; /*!< The bytes. */
} rtDmeMessage;

/** One address of a peer. */
typedef struct {
    int type;                          /*!< The kind of address. */
    char address[RT_NET_ADDRESS_SIZE]; /*!< The address as text. */
    unsigned int port;                 /*!< The port. */
} rtNetAddress;

/** The addresses and keys that connect a console to a session. The record is 0x9c bytes. */
typedef struct {
    int type;                                        /*!< The kind of connection. */
    rtNetAddress addresses[RT_NET_ADDRESS_COUNT];    /*!< The addresses of the session. */
    int worldId;                                     /*!< The session. */
    unsigned char serverKey[RT_NET_SERVER_KEY_SIZE]; /*!< The public key of the server. */
    char sessionKey[RT_NET_KEY_SIZE];                /*!< The key of the session. */
    char accessKey[RT_NET_KEY_SIZE];                 /*!< The key of the access. */
} rtNetConnectionInfo;

/**
 * Receiver of the messages of one registered type.
 *
 * @param connection The connection the message arrived on.
 * @param worldId The session of the connection.
 * @param sender The peer that sent the message.
 * @param message The message.
 * @return The number of bytes the receiver consumed.
 */
typedef int (*rtDmeMessageHandler)(int connection, int worldId, int sender, rtDmeMessage *message);

/**
 * Allocator the library takes its memory from.
 *
 * @param size The size of the block in bytes.
 * @return The block.
 */
typedef void *(*rtAllocFunc)(int size);

/**
 * Release of a block rtAllocFunc returned.
 *
 * @param block The block.
 */
typedef void (*rtFreeFunc)(void *block);

/** An RSA key pair. */
typedef struct {
    rtNetKey modulus;         /*!< The modulus, the public key. */
    rtNetKey privateExponent; /*!< The private exponent. */
} rtNetKeyPair;

/** The version of the parameters and the status of rtNetInitialize(). */
#define RT_NET_SETTINGS_VERSION 1

/** The size of the text of the version of the library, the terminator included. */
#define RT_NET_VERSION_SIZE 16

/** The parameters of rtNetInitialize(). */
typedef struct {
    int version;                 /*!< RT_NET_SETTINGS_VERSION. */
    int applicationId;           /*!< The identifier of the game. */
    const rtNetKeyPair *keyPair; /*!< The key pair of the console, or null. */
    int port;                    /*!< The port of the peers, or 0 for the default. */
} rtNetSettings;

/** The status rtNetInitialize() fills. */
typedef struct {
    int version;                              /*!< RT_NET_SETTINGS_VERSION. */
    int result;                               /*!< 0, or an error code. */
    char libraryVersion[RT_NET_VERSION_SIZE]; /*!< The version of the library. */
} rtNetInitStatus;

/**
 * Set the allocator of the library.
 *
 * @param alloc The allocator.
 * @param release The release.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a82a0
 */
int rtNetSetMemoryFunctions(rtAllocFunc alloc, rtFreeFunc release);

/**
 * Set the defaults of the parameters of rtNetInitialize().
 *
 * @param params The parameters.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a7c28
 */
int rtNetDefaultSettings(rtNetSettings *params);

/**
 * Start the library.
 *
 * @param params The parameters.
 * @param status Receives a further status.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a7c70
 */
int rtNetInitialize(const rtNetSettings *params, rtNetInitStatus *status);

/**
 * Stop the library.
 *
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a7dd8
 */
int rtNetClose(void);

/**
 * Start the datagram service of the peers with the sizes of its queues.
 *
 * @param sendSize The size of the send queue in bytes.
 * @param receiveSize The size of the receive queue in bytes.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8868
 */
int rtNetSetBufferSizes(int sendSize, int receiveSize);

/**
 * Generate the key pair of the console.
 *
 * @param publicKey Receives the public key.
 * @param privateKey Receives the private key.
 * @ghidraAddress NTSC-U/C: 0x002a9f74
 */
void rtNetGenerateKeyPair(rtNetKey *publicKey, rtNetKey *privateKey);

/**
 * Receiver of the result of rtDmeHost() and rtDmeConnect(), and of a status of the connection.
 *
 * @param connection The connection, or -1 when none was opened.
 * @param state The state of the connection: 0 idle, 1 connected, 2 connecting, or 3 failed.
 * @param status The status the server or the channels reported.
 * @param result 0 for success, otherwise an error.
 */
typedef void (*rtDmeConnectCallback)(int connection, int state, int status, int result);

/**
 * Receiver of a peer that joined or left a connection.
 *
 * @param connection The connection.
 * @param client The peer.
 * @param address The address the peer was reported at.
 */
typedef void (*rtDmeClientCallback)(int connection, int client, const char *address);

/** The arguments of the receiver of a change of host. */
typedef struct {
    int connection; /*!< The connection, always 0. */
    int host;       /*!< The new host. */
    int argument;   /*!< The argument given with the receiver. */
} rtDmeHostChange;

/**
 * Receiver of a change of the host of connection 0.
 *
 * @param change The change.
 */
typedef void (*rtDmeHostChangeCallback)(const rtDmeHostChange *change);

/** The arguments of the receiver of the events of the voice. */
typedef struct {
    int event;    /*!< Set by the receiver, below 5. */
    int value1;   /*!< The first value of the device. */
    int value2;   /*!< The second value of the device. */
    int result;   /*!< Set by the receiver, the result given to the device. */
    int argument; /*!< The argument given with the receiver. */
} rtDmeVoiceEvent;

/** The arguments of the receiver of the state changes of the voice. */
typedef struct {
    int state;    /*!< The state, below 5. */
    int level;    /*!< The level the device reports. */
    int value1;   /*!< The first value of the device. */
    int value2;   /*!< The second value of the device. */
    int result;   /*!< Starts as value2, and is given back to the device. */
    int argument; /*!< The argument given with the receiver. */
} rtDmeVoiceState;

/** The arguments of the receiver of the voice data. */
typedef struct {
    int value1;   /*!< The first value of the device. */
    int value2;   /*!< The second value of the device. */
    int value3;   /*!< The third value of the device. */
    int result;   /*!< Set by the receiver, the result given to the device. */
    int argument; /*!< The argument given with the receiver. */
} rtDmeVoiceData;

/** The arguments of the receiver of the end of the voice data. */
typedef struct {
    int value1;   /*!< The first value of the device. */
    int value2;   /*!< The second value of the device. */
    int result;   /*!< Set by the receiver, the result given to the device. */
    int argument; /*!< The argument given with the receiver. */
} rtDmeVoiceEnd;

/** Receiver of the events of the voice. */
typedef void (*rtDmeVoiceEventCallback)(rtDmeVoiceEvent *event);
/** Receiver of the state changes of the voice. */
typedef void (*rtDmeVoiceStateCallback)(rtDmeVoiceState *state);
/** Receiver of the voice data. */
typedef void (*rtDmeVoiceDataCallback)(rtDmeVoiceData *data);
/** Receiver of the end of the voice data. */
typedef void (*rtDmeVoiceEndCallback)(rtDmeVoiceEnd *end);

/** The settings of the voice of a connection. 0x54 bytes. */
typedef struct {
    int version;                     /*!< 1. */
    int enabled;                     /*!< 1 to open the voice with the connection. */
    int port;                        /*!< The port of the device. */
    int channelCount;                /*!< The channels of the device, at most 16. */
    int mode;                        /*!< 0 or 1, the mode the channels open in. */
    int format[6];                   /*!< The format values handed to the device. */
    rtDmeVoiceEventCallback onEvent; /*!< Receives the events, or null. */
    int eventArgument;               /*!< The argument of onEvent. */
    rtDmeVoiceStateCallback onState; /*!< Receives the state changes, or null. */
    int stateArgument;               /*!< The argument of onState. */
    int device[2];                   /*!< Values handed to the device. */
    rtDmeVoiceDataCallback onData;   /*!< Receives the data, or null. */
    int dataArgument;                /*!< The argument of onData. */
    rtDmeVoiceEndCallback onEnd;     /*!< Receives the end of the data, or null. */
    int endArgument;                 /*!< The argument of onEnd. */
} rtDmeVoiceSettings;

/** The parameters of rtDmeHost(). 0x68 bytes. */
typedef struct {
    int version;                       /*!< Set by rtDmeInitHostParams(). */
    rtDmeConnectCallback onConnect;    /*!< Receives the result. */
    rtDmeClientCallback onClientJoin;  /*!< Receives a peer that joined. */
    rtDmeClientCallback onClientLeave; /*!< Receives a peer that left. */
    int maxClients;                    /*!< The peers the connection takes. */
    rtDmeVoiceSettings voice;          /*!< The voice of the connection. */
} rtDmeHostParams;

/** The parameters of rtDmeConnect(). 0x114 bytes. */
typedef struct {
    int version;                          /*!< Set by rtDmeInitConnectParams(). */
    int maxClients;                       /*!< The peers the connection takes. */
    rtNetConnectionInfo info;             /*!< The session to connect to. */
    rtDmeConnectCallback onConnect;       /*!< Receives the result. */
    rtDmeConnectCallback onStatus;        /*!< Receives the statuses of the server. Unset. */
    rtDmeClientCallback onClientJoin;     /*!< Receives a peer that joined. */
    rtDmeClientCallback onClientLeave;    /*!< Receives a peer that left. */
    int port;                             /*!< The port of a datagram connection. */
    rtDmeHostChangeCallback onHostChange; /*!< Receives the changes of host, or null. */
    int hostChangeArgument;               /*!< The argument of onHostChange. */
    rtDmeVoiceSettings voice;             /*!< The voice of the connection. */
} rtDmeConnectParams;

/** The status rtDmeHost() and rtDmeConnect() fill. 0xc bytes. */
typedef struct {
    int version;    /*!< 1. */
    int result;     /*!< 0, or an error code. */
    int connection; /*!< The connection, or -1. */
} rtDmeConnectStatus;

/**
 * Set the defaults of the parameters of rtDmeHost().
 *
 * @param params The parameters.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8400
 */
int rtDmeInitHostParams(rtDmeHostParams *params);

/**
 * Start hosting a connection.
 *
 * @param params The parameters.
 * @param status Receives a further status.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8498
 */
int rtDmeHost(const rtDmeHostParams *params, rtDmeConnectStatus *status);

/**
 * Set the defaults of the parameters of rtDmeConnect().
 *
 * @param params The parameters.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a82c0
 */
int rtDmeInitConnectParams(rtDmeConnectParams *params);

/**
 * Start connecting to a session.
 *
 * @param params The parameters.
 * @param status Receives a further status.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8358
 */
int rtDmeConnect(const rtDmeConnectParams *params, rtDmeConnectStatus *status);

/**
 * Report the peer index of this console on a connection.
 *
 * @param client Receives the index.
 * @param connection The connection.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8a80
 */
int rtDmeGetLocalClient(int *client, int connection);

/**
 * Enable or disable the messages of a connection.
 *
 * @param connection The connection.
 * @param enable Non-zero to enable the messages.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a87c0
 */
int rtDmeEnableConnection(int connection, int enable);

/**
 * Close a connection.
 *
 * @param connection The connection.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8c80
 */
int rtDmeDisconnect(int connection);

/**
 * Start the field list of a message type.
 *
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a7ed8
 */
int rtDmeBeginFields(void);

/**
 * Append a field to the field list of a message type. The library stores the two addresses and
 * the two numbers, and does not read them back.
 *
 * @param record The address of a record of the type, not null.
 * @param field The address of the field in the record, not null.
 * @param size A positive number, by convention the size of an element in bytes. Callers also pass
 * the size of the whole field here with the same value as the count.
 * @param count A positive number, by convention the number of elements.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a7f60
 */
int rtDmeAddField(const void *record, const void *field, int size, int count);

/**
 * Register a message type with the field list and its receiver.
 *
 * @param type Receives the registered type.
 * @param handler The receiver.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a80f0
 */
int rtDmeRegisterMessage(int *type, rtDmeMessageHandler handler);

/**
 * Send a message to one peer or to every peer of a connection.
 *
 * @param flags A combination of the RT_DME_SEND_ bits.
 * @param connection The connection.
 * @param receiver The peer, or -1 for every peer.
 * @param type The registered type.
 * @param size The number of bytes of the message, its length included.
 * @param message The message.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a81a0
 */
int rtDmeSend(signed char flags,
              int connection,
              int receiver,
              int type,
              unsigned short size,
              const rtDmeMessage *message);

/** The message class of the messages exchanged with the lobby server. */
#define RT_DME_CLASS_LOBBY 1

/** The message class of the messages exchanged with the game server. */
#define RT_DME_CLASS_SERVER 3

/**
 * Register a message type of a message class with the field list and its receiver.
 *
 * @param type Receives the registered type.
 * @param messageClass The message class.
 * @param handler The receiver, or null for a type the console only sends.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8038
 * @ghidraAddress PAL: 0x002b1fa8
 */
int rtDmeRegisterClassMessage(int *type, int messageClass, rtDmeMessageHandler handler);

/**
 * Send a message of a message class.
 *
 * @param flags A combination of the RT_DME_SEND_ bits.
 * @param connection The connection.
 * @param receiver The peer, or 0xffff for the server.
 * @param messageClass The message class.
 * @param type The registered type.
 * @param size The size of the record in bytes.
 * @param record The record.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002b22d8
 * @ghidraAddress PAL: 0x002bc248
 */
int rtDmeSendClassMessage(signed char flags,
                          int connection,
                          int receiver,
                          unsigned int messageClass,
                          unsigned int type,
                          int size,
                          const void *record);

/** The addresses the server sees for a console. */
typedef struct {
    int complete;                                 /*!< Non-zero once the addresses are known. */
    int error;                                    /*!< 0 for success, otherwise an error. */
    rtNetAddress addresses[RT_NET_ADDRESS_COUNT]; /*!< The addresses. */
} rtDmeAddressResult;

/**
 * Receiver of the result of rtDmeQueryAddresses().
 *
 * @param result The result.
 */
typedef void (*rtDmeAddressCallback)(const rtDmeAddressResult *result);

/** The parameters of rtDmeQueryAddresses(). 0x20 bytes. */
typedef struct {
    int version;                   /*!< Set by rtDmeInitAddressQuery(). */
    rtDmeAddressCallback onResult; /*!< Receives the result. */
    rtNetAddress server;           /*!< The server to ask. */
} rtDmeAddressQuery;

/** The parameters of rtDmeReportAddresses(). 0x34 bytes. */
typedef struct {
    int version;                                  /*!< Set by rtDmeInitAddressReport(). */
    rtNetAddress addresses[RT_NET_ADDRESS_COUNT]; /*!< The addresses of the peer. */
} rtDmeAddressReport;

/**
 * Set the defaults of the parameters of rtDmeQueryAddresses().
 *
 * @param query The parameters.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8540
 * @ghidraAddress PAL: 0x002b24b0
 */
int rtDmeInitAddressQuery(rtDmeAddressQuery *query);

/**
 * Ask a server for the addresses it sees for this console.
 *
 * @param query The parameters.
 * @param status Receives a further status.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a85d8
 * @ghidraAddress PAL: 0x002b2548
 */
int rtDmeQueryAddresses(const rtDmeAddressQuery *query, rtDmeConnectStatus *status);

/**
 * Set the defaults of the parameters of rtDmeReportAddresses().
 *
 * @param report The parameters.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8680
 * @ghidraAddress PAL: 0x002b25f0
 */
int rtDmeInitAddressReport(rtDmeAddressReport *report);

/**
 * Hand the library the addresses of a peer.
 *
 * @param report The parameters.
 * @param status Receives a further status.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8718
 * @ghidraAddress PAL: 0x002b2688
 */
int rtDmeReportAddresses(const rtDmeAddressReport *report, rtDmeConnectStatus *status);

/**
 * Write the local address of the console as text.
 *
 * @param address Receives the text.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8950
 * @ghidraAddress PAL: 0x002b28c0
 */
int rtDmeGetLocalAddress(char address[RT_NET_ADDRESS_SIZE]);

/**
 * Report the address of the sender of the last datagram.
 *
 * @param address Receives the 4 bytes of the address.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a89e8
 */
int rtDmeGetSenderAddress(unsigned char address[4]);

/**
 * Report the time of this console in milliseconds.
 *
 * @param time Receives the time.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8b30
 */
int rtDmeGetLocalTime(unsigned int *time);

/**
 * Service the library once per frame.
 *
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8bf8
 * @ghidraAddress PAL: 0x002b2b68
 */
int rtDmeUpdate(void);

/**
 * Parse dotted decimal text into an address, its first number in the top byte. Characters other
 * than digits and dots are skipped.
 *
 * @param address The text, or null.
 * @return The address, 0 for null text.
 * @ghidraAddress NTSC-U/C: 0x002af95c
 */
unsigned int rtNetParseAddress(const char *address);

/**
 * Write an address as dotted decimal text, its top byte first.
 *
 * @param value The address.
 * @param address Receives the text, RT_NET_ADDRESS_SIZE bytes. Nothing is written when it is null.
 * @ghidraAddress NTSC-U/C: 0x002af9cc
 */
void rtNetFormatAddress(unsigned int value, char *address);

#ifdef __cplusplus
}
#endif

#endif
