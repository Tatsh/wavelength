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
 * @param type The registered type.
 * @param sender The peer that sent the message.
 * @param message The message.
 * @return The number of bytes the receiver consumed.
 */
typedef int (*rtDmeMessageHandler)(int connection, int type, int sender, rtDmeMessage *message);

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

/** The parameters of rtNetInitialize(). 0x10 bytes. */
typedef struct {
    int reserved00;                 /* +0x00, set by rtNetDefaultSettings(). */
    int applicationId;              /*!< The identifier of the game. */
    const unsigned char *publicKey; /*!< The public key of the console, or null. */
    int reserved0C;                 /* +0x0c, set by rtNetDefaultSettings(). */
} rtNetSettings;

/** The status rtNetInitialize() fills. 0x18 bytes. */
typedef struct {
    int reserved[6]; /* +0x00, not read by the game. */
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
 * Set the sizes of the buffers of the library.
 *
 * @param receiveSize The size of the receive buffer in bytes.
 * @param sendSize The size of the send buffer in bytes.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8868
 */
int rtNetSetBufferSizes(int receiveSize, int sendSize);

/**
 * Generate the key pair of the console.
 *
 * @param publicKey Receives the public key.
 * @param privateKey Receives the private key.
 * @ghidraAddress NTSC-U/C: 0x002a9f74
 */
void rtNetGenerateKeyPair(unsigned char publicKey[RT_NET_SERVER_KEY_SIZE],
                          unsigned char privateKey[RT_NET_SERVER_KEY_SIZE]);

/**
 * Receiver of the result of rtDmeHost() and rtDmeConnect().
 *
 * @param connection The connection.
 * @param reserved1 Not read by the game.
 * @param reserved2 Not read by the game.
 * @param status 0 for success, otherwise an error.
 */
typedef void (*rtDmeConnectCallback)(int connection, int reserved1, int reserved2, int status);

/**
 * Receiver of a peer that joined or left a connection.
 *
 * @param connection The connection.
 * @param client The peer.
 */
typedef void (*rtDmeClientCallback)(int connection, int client);

/** The parameters of rtDmeHost(). 0x68 bytes. */
typedef struct {
    int reserved00;                    /* +0x00, set by rtDmeInitHostParams(). */
    rtDmeConnectCallback onConnect;    /*!< Receives the result. */
    rtDmeClientCallback onClientJoin;  /*!< Receives a peer that joined. */
    rtDmeClientCallback onClientLeave; /*!< Receives a peer that left. */
    int maxClients;                    /*!< The peers the connection takes. */
    char reserved14[0x54];             /* +0x14, set by rtDmeInitHostParams(). */
} rtDmeHostParams;

/** The parameters of rtDmeConnect(). 0x114 bytes. */
typedef struct {
    int reserved00;                    /* +0x00, set by rtDmeInitConnectParams(). */
    int maxClients;                    /*!< The peers the connection takes. */
    rtNetConnectionInfo info;          /*!< The session to connect to. */
    rtDmeConnectCallback onConnect;    /*!< Receives the result. */
    rtDmeClientCallback onClientReady; /*!< Unset by the game. */
    rtDmeClientCallback onClientJoin;  /*!< Receives a peer that joined. */
    rtDmeClientCallback onClientLeave; /*!< Receives a peer that left. */
    char reservedB4[0x60];             /* +0xb4, set by rtDmeInitConnectParams(). */
} rtDmeConnectParams;

/** The status rtDmeHost() and rtDmeConnect() fill. 0xc bytes. */
typedef struct {
    int reserved[3]; /* +0x00, not read by the game. */
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
 * Append a field to the field list of a message type.
 *
 * The field is described by its offset in the record, the size of its elements, and their count.
 *
 * @param record The address of a record of the type.
 * @param field The address of the field in the record.
 * @param size The size of an element of the field in bytes.
 * @param count The number of elements.
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
                          int messageClass,
                          int type,
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
    int reserved00;                /* +0x00, set by rtDmeInitAddressQuery(). */
    rtDmeAddressCallback onResult; /*!< Receives the result. */
    rtNetAddress server;           /*!< The server to ask. */
} rtDmeAddressQuery;

/** The parameters of rtDmeReportAddresses(). 0x34 bytes. */
typedef struct {
    int reserved00;                               /* +0x00, set by rtDmeInitAddressReport(). */
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
 * Service the library once per frame.
 *
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002a8bf8
 * @ghidraAddress PAL: 0x002b2b68
 */
int rtDmeUpdate(void);

#ifdef __cplusplus
}
#endif

#endif
