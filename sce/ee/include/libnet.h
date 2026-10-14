#ifndef LIBNET_H
#define LIBNET_H

#include <msifrpc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * EE client of the libnetb IOP module, with the asynchronous transfers of the SCE-RT network
 * library on top. Each call passes its arguments to the module in a 64-byte aligned buffer of the
 * caller, as large as the buffer size sceLibnetInitialize() bound with. The names of the routines
 * other than sceLibnetInitialize() are inferred.
 */

/** The bound client cannot take a buffer of the size or alignment asked for. */
#define LIBNET_ERROR_BUFFER_SIZE (-543)

/** The call to the module failed. */
#define LIBNET_ERROR_CALL (-542)

/** The module could not be bound, or it reported no connection. */
#define LIBNET_ERROR_BIND (-540)

/** An argument is null or out of range. */
#define LIBNET_ERROR_ARGUMENT (-512)

/** An asynchronous transfer failed. */
#define LIBNET_ERROR_ASYNC (-501)

/** Connection types. */
typedef enum {
    sceINETT_DGRAM = 0,   /*!< UDP. */
    sceINETT_CONNECT = 1, /*!< TCP, opened actively. */
    sceINETT_LISTEN = 2,  /*!< TCP, opened passively. */
    sceINETT_RAW = 3,     /*!< Raw IP. */
} sceInetType;

/** An address. */
typedef struct {
    int reserved; /*!< Always zero. */
    unsigned char
        data[12]; /*!< Address bytes. An IPv4 address is the first four, last octet first. */
} sceInetAddress;

/** The parameters of a connection, 0x40 bytes. */
typedef struct {
    int type;                     /*!< A sceInetType. */
    int localPort;                /*!< The local port. */
    sceInetAddress remoteAddress; /*!< The remote address. */
    int remotePort;               /*!< The remote port. */
    int reserved[9];              /* +0x1c, passed through to the module. */
} sceInetParam;

/** The parameters of LibnetAsyncCreate(). */
typedef struct {
    int maxPacketSize; /*!< The largest datagram of a UDP connection. */
    int reserved04;    /* +0x04, not read. */
    sceInetParam inet; /*!< The connection. */
} LibnetAsyncParam;

/** A connection with asynchronous transfers. */
typedef struct LibnetAsyncHandle LibnetAsyncHandle;

/**
 * Bind a client to the libnetb module, retrying until the module serves it.
 *
 * @param cd The client.
 * @param buffersize The size of the argument buffers, at least 128 and a multiple of 64.
 * @param stacksize The stack size of the server thread.
 * @param priority The priority of the server thread.
 * @return 0, LIBNET_ERROR_BUFFER_SIZE, or LIBNET_ERROR_BIND.
 * @ghidraAddress NTSC-U/C: 0x003059c8
 * @ghidraAddress PAL: 0x0030fc60
 */
int sceLibnetInitialize(sceSifMClientData *cd, int buffersize, int stacksize, int priority);

/**
 * Unbind a client from the libnetb module, retrying until the module releases it.
 *
 * @param cd The client.
 * @return 0, or LIBNET_ERROR_BIND.
 * @ghidraAddress NTSC-U/C: 0x00305aa0
 * @ghidraAddress PAL: 0x0030fd38
 */
int sceLibnetTerminate(sceSifMClientData *cd);

/**
 * Create a connection.
 *
 * @param cd The client.
 * @param buffer The argument buffer.
 * @param param The parameters of the connection.
 * @return The connection identifier, LIBNET_ERROR_BIND when the module reports none, or another
 * negative error.
 * @ghidraAddress NTSC-U/C: 0x003056e0
 * @ghidraAddress PAL: 0x0030f778
 */
int sceInetCreate(sceSifMClientData *cd, int *buffer, sceInetParam *param);

/**
 * Open a connection.
 *
 * @param cd The client.
 * @param buffer The argument buffer.
 * @param cid The connection identifier.
 * @param timeout The timeout.
 * @return The result of the module, or LIBNET_ERROR_CALL.
 * @ghidraAddress NTSC-U/C: 0x00305758
 */
int sceInetOpen(sceSifMClientData *cd, int *buffer, int cid, int timeout);

/**
 * Resolve a name to an address.
 *
 * @param cd The client.
 * @param buffer The argument buffer.
 * @param flags Flags. With 0x80 set, one more int argument follows @p retries.
 * @param address Receives the address.
 * @param name The name.
 * @param timeout The timeout.
 * @param retries The number of retries.
 * @return The result of the module, or a negative error.
 * @ghidraAddress NTSC-U/C: 0x003055e8
 * @ghidraAddress PAL: 0x0030f680
 */
int sceInetName2Address(sceSifMClientData *cd,
                        int *buffer,
                        int flags,
                        sceInetAddress *address,
                        const char *name,
                        int timeout,
                        int retries,
                        ...);

/**
 * Apply a control operation to a connection.
 *
 * @param cd The client.
 * @param buffer The argument buffer.
 * @param cid The connection identifier.
 * @param code The control code.
 * @param data The buffer of the operation, read and written.
 * @param size The size of @p data.
 * @return The result of the module, or a negative error.
 * @ghidraAddress NTSC-U/C: 0x003057a8
 * @ghidraAddress PAL: 0x0030fa40
 */
int sceInetControl(sceSifMClientData *cd, int *buffer, int cid, int code, void *data, int size);

/**
 * List the interfaces.
 *
 * @param cd The client.
 * @param buffer The argument buffer.
 * @param interfaceIds Receives the interface identifiers.
 * @param count The capacity of @p interfaceIds.
 * @return The number of interfaces, or a negative error.
 * @ghidraAddress NTSC-U/C: 0x00305858
 * @ghidraAddress PAL: 0x0030faf0
 */
int sceInetGetInterfaceList(sceSifMClientData *cd, int *buffer, int *interfaceIds, int count);

/**
 * Apply a control operation to an interface.
 *
 * @param cd The client.
 * @param buffer The argument buffer.
 * @param interfaceId The interface identifier.
 * @param code The control code.
 * @param data The buffer of the operation, read and written.
 * @param size The size of @p data.
 * @return The result of the module, or a negative error.
 * @ghidraAddress NTSC-U/C: 0x003058e8
 * @ghidraAddress PAL: 0x0030fb80
 */
int sceInetInterfaceControl(
    sceSifMClientData *cd, int *buffer, int interfaceId, int code, void *data, int size);

/**
 * Have the module queue the interface events.
 *
 * @param cd The client.
 * @param buffer The argument buffer.
 * @return The result of the module, or LIBNET_ERROR_CALL.
 * @ghidraAddress NTSC-U/C: 0x00305b20
 * @ghidraAddress PAL: 0x0030fdb8
 */
int sceLibnetRegisterHandler(sceSifMClientData *cd, int *buffer);

/**
 * Have the module stop queueing the interface events.
 *
 * @param cd The client.
 * @param buffer The argument buffer.
 * @return The result of the module, or LIBNET_ERROR_CALL.
 * @ghidraAddress NTSC-U/C: 0x00305b60
 * @ghidraAddress PAL: 0x0030fdf8
 */
int sceLibnetUnregisterHandler(sceSifMClientData *cd, int *buffer);

/**
 * Find the address of an interface that is up.
 *
 * With bit 1 of @p flags set, an interface already up is used when there is one. Otherwise, or
 * when none is up, the module waits for interfaces to come up.
 *
 * @param cd The client.
 * @param buffer The argument buffer.
 * @param interfaceIds Receives the interface identifiers. The address is that of the first.
 * @param count The capacity of @p interfaceIds.
 * @param address Receives the address.
 * @param flags The flags.
 * @return 0, or a negative error.
 * @ghidraAddress NTSC-U/C: 0x00305e50
 * @ghidraAddress PAL: 0x003100e8
 */
int sceLibnetWaitGetAddress(sceSifMClientData *cd,
                            int *buffer,
                            int *interfaceIds,
                            int count,
                            sceInetAddress *address,
                            int flags);

/**
 * Create a connection and the buffers of its asynchronous transfers.
 *
 * @param cd The client.
 * @param buffer The argument buffer.
 * @param handle Receives the connection.
 * @param param The parameters.
 * @return 0, LIBNET_ERROR_ARGUMENT, LIBNET_ERROR_ASYNC, or the error of sceInetCreate(). A
 * failed allocation of the handle itself reports 0 and leaves @p handle null.
 * @ghidraAddress NTSC-U/C: 0x00305e98
 * @ghidraAddress PAL: 0x00310130
 */
int LibnetAsyncCreate(sceSifMClientData *cd,
                      int *buffer,
                      LibnetAsyncHandle **handle,
                      LibnetAsyncParam *param);

/**
 * Start the transfer threads of a connection and open it.
 *
 * @param cd The client.
 * @param buffer The argument buffer.
 * @param handle The connection.
 * @param timeout The timeout of the open.
 * @return The result of sceInetOpen(), LIBNET_ERROR_ARGUMENT, or LIBNET_ERROR_ASYNC.
 * @ghidraAddress NTSC-U/C: 0x00305fb8
 * @ghidraAddress PAL: 0x00310250
 */
int LibnetAsyncOpen(sceSifMClientData *cd, int *buffer, LibnetAsyncHandle *handle, int timeout);

/**
 * Close a connection and release it.
 *
 * @param cd The client.
 * @param buffer The argument buffer.
 * @param handle The connection.
 * @param timeout The time to let the pending sends finish.
 * @return 0, LIBNET_ERROR_ARGUMENT, or LIBNET_ERROR_ASYNC.
 * @ghidraAddress NTSC-U/C: 0x003060d0
 * @ghidraAddress PAL: 0x00310368
 */
int LibnetAsyncClose(sceSifMClientData *cd, int *buffer, LibnetAsyncHandle *handle, int timeout);

/**
 * Read the bytes a TCP connection has received.
 *
 * @param cd Not read.
 * @param buffer Not read.
 * @param handle The connection.
 * @param data Receives the bytes.
 * @param size The capacity of @p data.
 * @param flags Receives the flags of the last transfer when no byte is waiting.
 * @return The number of bytes read, 0, a negative error of the transfer, or
 * LIBNET_ERROR_ARGUMENT.
 * @ghidraAddress NTSC-U/C: 0x00306168
 * @ghidraAddress PAL: 0x00310400
 */
int LibnetAsyncReadStream(sceSifMClientData *cd,
                          int *buffer,
                          LibnetAsyncHandle *handle,
                          void *data,
                          int size,
                          int *flags);

/**
 * Hand bytes to the sending transfer of a TCP connection.
 *
 * @param cd Not read.
 * @param buffer Not read.
 * @param handle The connection.
 * @param data The bytes.
 * @param size The number of bytes.
 * @return The number of bytes taken, 0 while the previous bytes are being sent, a negative error
 * of the transfer, or LIBNET_ERROR_ARGUMENT.
 * @ghidraAddress NTSC-U/C: 0x00306378
 * @ghidraAddress PAL: 0x00310600
 */
int LibnetAsyncWriteStream(sceSifMClientData *cd,
                           int *buffer,
                           LibnetAsyncHandle *handle,
                           const void *data,
                           unsigned int size);

/**
 * Take the next datagram a connection has received.
 *
 * @param cd The client, only tested for null.
 * @param buffer The argument buffer, only tested for null.
 * @param handle The connection.
 * @param data Receives the datagram.
 * @param size The capacity of @p data.
 * @param flags Receives the flags of the datagram.
 * @param address Receives the sender.
 * @param port Receives the port of the sender.
 * @param timeout At least -1. Not otherwise read.
 * @return The length taken, 0 when none is waiting, or a negative error.
 * @ghidraAddress NTSC-U/C: 0x00306280
 * @ghidraAddress PAL: 0x00310508
 */
int LibnetAsyncRecvFrom(sceSifMClientData *cd,
                        int *buffer,
                        LibnetAsyncHandle *handle,
                        void *data,
                        int size,
                        int *flags,
                        sceInetAddress *address,
                        int *port,
                        int timeout);

/**
 * Take a two-byte marker datagram a connection has received.
 *
 * @param cd The client, only tested for null.
 * @param buffer Not read.
 * @param handle The connection.
 * @param marker Receives the marker.
 * @param address Receives the sender.
 * @return 1 when a marker was taken, 0 when none, or a negative error.
 * @ghidraAddress NTSC-U/C: 0x00306310
 * @ghidraAddress PAL: 0x00310598
 */
int LibnetAsyncRecvMarker(sceSifMClientData *cd,
                          int *buffer,
                          LibnetAsyncHandle *handle,
                          void *marker,
                          sceInetAddress *address);

/**
 * Queue a datagram on a connection.
 *
 * @param cd The client, only tested for null.
 * @param buffer The argument buffer, only tested for null.
 * @param handle The connection.
 * @param data The datagram.
 * @param size The length of the datagram.
 * @param flags The flags of the datagram.
 * @param address The receiver.
 * @param port The port of the receiver.
 * @param timeout At least -1. Not otherwise read.
 * @return The length queued, 0 when the ring buffer is full, or a negative error.
 * @ghidraAddress NTSC-U/C: 0x003064b0
 * @ghidraAddress PAL: 0x00310738
 */
int LibnetAsyncSendTo(sceSifMClientData *cd,
                      int *buffer,
                      LibnetAsyncHandle *handle,
                      const void *data,
                      int size,
                      int *flags,
                      const sceInetAddress *address,
                      int port,
                      int timeout);

/**
 * Queue a two-byte marker datagram on a connection.
 *
 * @param cd The client, only tested for null.
 * @param buffer Not read.
 * @param handle The connection.
 * @param marker The marker.
 * @param address The receiver.
 * @return 1 when the marker was queued, 0 when the ring buffer is full, or a negative error.
 * @ghidraAddress NTSC-U/C: 0x00306548
 * @ghidraAddress PAL: 0x003107d0
 */
int LibnetAsyncSendMarker(sceSifMClientData *cd,
                          int *buffer,
                          LibnetAsyncHandle *handle,
                          unsigned short marker,
                          const sceInetAddress *address);

/**
 * Apply a control operation to a connection.
 *
 * @param cd The client.
 * @param buffer The argument buffer.
 * @param handle The connection.
 * @param code The control code.
 * @param data The buffer of the operation, read and written.
 * @param size The size of @p data.
 * @return The result of sceInetControl(), or LIBNET_ERROR_ARGUMENT.
 * @ghidraAddress NTSC-U/C: 0x003065b0
 * @ghidraAddress PAL: 0x00310838
 */
int LibnetAsyncControl(
    sceSifMClientData *cd, int *buffer, LibnetAsyncHandle *handle, int code, void *data, int size);

/**
 * Read an address from its dotted text.
 *
 * @param address Receives the address.
 * @param text The text, four decimal octets separated by full stops.
 * @return 0, or LIBNET_ERROR_ARGUMENT.
 * @ghidraAddress NTSC-U/C: 0x003065e0
 * @ghidraAddress PAL: 0x00310868
 */
int LibnetStringToAddress(sceInetAddress *address, const char *text);

/**
 * Write an address as dotted text.
 *
 * @param text Receives the text.
 * @param address The address.
 * @return 0, or LIBNET_ERROR_ARGUMENT.
 * @ghidraAddress NTSC-U/C: 0x00306678
 * @ghidraAddress PAL: 0x00310900
 */
int LibnetAddressToString(char *text, const sceInetAddress *address);

#ifdef __cplusplus
}
#endif

#endif
