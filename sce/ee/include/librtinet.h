#ifndef LIBRTINET_H
#define LIBRTINET_H

#include <stdbool.h>

#include <libnet.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Network driver of the SCE-RT network library over the libnetb client.
 *
 * The names of the routines and their parameters are inferred.
 */

/** Kinds of sockets, the values of rtNetSocketOptions::type. */
#define RT_NET_SOCKET_STREAM 0          /*!< A stream socket that connects to an address. */
#define RT_NET_SOCKET_DATAGRAM 1        /*!< A datagram socket. */
#define RT_NET_SOCKET_STREAM_ACCEPTED 2 /*!< A stream socket that waits for a peer. */
#define RT_NET_SOCKET_LISTEN 3          /*!< A raw socket that exchanges two-byte markers. */
#define RT_NET_SOCKET_TYPE_COUNT 4      /*!< The number of kinds of sockets. */

/** The options of a socket. */
typedef struct {
    int type;       /*!< One of the RT_NET_SOCKET_ kinds. */
    int bufferSize; /*!< The largest datagram of a RT_NET_SOCKET_DATAGRAM socket in bytes. */
} rtNetSocketOptions;

/**
 * Bind the libnetb client, wait for an interface to come up, and read its addresses.
 *
 * @return 0, 4 when the driver is already started, or another error code.
 * @ghidraAddress NTSC-U/C: 0x002f8560
 * @ghidraAddress PAL: 0x003024d0
 */
int rtNetInetInit(void);

/**
 * Release the libnetb client.
 *
 * @return 0, 5 when the driver is not started, or 1 on failure.
 * @ghidraAddress NTSC-U/C: 0x002f8628
 * @ghidraAddress PAL: 0x00302598
 */
int rtNetInetShutdown(void);

/**
 * Create and open a connection.
 *
 * @param connection Receives the connection, or null on failure.
 * @param address The dotted address to connect a RT_NET_SOCKET_STREAM socket to, otherwise null.
 * @param port The port, the remote one of a RT_NET_SOCKET_STREAM socket and the local one
 * otherwise. Not read for a RT_NET_SOCKET_LISTEN socket.
 * @param options The options.
 * @return 0, 1 on failure, 2 for a bad argument, 3 for an unknown kind, or 5 when the driver is not
 * started.
 * @ghidraAddress NTSC-U/C: 0x002f86a8
 * @ghidraAddress PAL: 0x00302618
 */
int rtNetInetOpen(LibnetAsyncHandle **connection,
                  const char *address,
                  unsigned short port,
                  const rtNetSocketOptions *options);

/**
 * Close a connection.
 *
 * @param connection The connection.
 * @return 0, 1 on failure, 2 for a bad connection, or 5 when the driver is not started.
 * @ghidraAddress NTSC-U/C: 0x002f8960
 * @ghidraAddress PAL: 0x003028d0
 */
int rtNetInetClose(LibnetAsyncHandle *connection);

/**
 * Send bytes on a stream connection.
 *
 * @param connection The connection.
 * @param data The bytes.
 * @param length The number of bytes.
 * @param sent Receives the number of bytes taken.
 * @return 0, 1 on failure, 2 for a bad argument, or 5 when the driver is not started.
 * @ghidraAddress NTSC-U/C: 0x002f89b0
 * @ghidraAddress PAL: 0x00302920
 */
int rtNetInetSendStream(LibnetAsyncHandle *connection,
                        const void *data,
                        unsigned short length,
                        unsigned short *sent);

/**
 * Queue a datagram.
 *
 * @param connection The connection.
 * @param address The dotted address of the receiver.
 * @param port The port of the receiver.
 * @param data The bytes.
 * @param length The number of bytes.
 * @param sent Receives the number of bytes queued.
 * @return 0, 1 on failure, 2 for a bad argument, or 5 when the driver is not started.
 * @ghidraAddress NTSC-U/C: 0x002f8a48
 * @ghidraAddress PAL: 0x003029b8
 */
int rtNetInetSendTo(LibnetAsyncHandle *connection,
                    const char *address,
                    unsigned short port,
                    const void *data,
                    unsigned short length,
                    unsigned short *sent);

/**
 * Queue a two-byte marker on a RT_NET_SOCKET_LISTEN connection.
 *
 * @param connection The connection.
 * @param sent Receives true when the marker was queued.
 * @param address The dotted address of the receiver.
 * @param marker The marker.
 * @return 0, 1 on failure, 2 for a bad argument, or 5 when the driver is not started.
 * @ghidraAddress NTSC-U/C: 0x002f8b38
 * @ghidraAddress PAL: 0x00302aa8
 */
int rtNetInetSendMarker(LibnetAsyncHandle *connection,
                        bool *sent,
                        const char *address,
                        unsigned short marker);

/**
 * Read the bytes a stream connection has received.
 *
 * @param connection The connection.
 * @param buffer Receives the bytes.
 * @param size The size of the buffer in bytes. At most 4096 bytes are read.
 * @param received Receives the number of bytes read.
 * @return 0, 1 on failure, 2 for a bad argument, 5 when the driver is not started, or 13 when the
 * peer closed the stream.
 * @ghidraAddress NTSC-U/C: 0x002f8bf8
 * @ghidraAddress PAL: 0x00302b68
 */
int rtNetInetReceiveStream(LibnetAsyncHandle *connection,
                           void *buffer,
                           unsigned short size,
                           unsigned short *received);

/**
 * Take the next datagram a connection has received.
 *
 * @param connection The connection.
 * @param address Receives the dotted address of the sender.
 * @param port Receives the port of the sender.
 * @param buffer Receives the bytes.
 * @param size The size of the buffer in bytes. At most 4096 bytes are read.
 * @param received Receives the number of bytes read, 0 when no datagram is waiting.
 * @return 0, 1 on failure, 2 for a bad argument, or 5 when the driver is not started.
 * @ghidraAddress NTSC-U/C: 0x002f8cb0
 * @ghidraAddress PAL: 0x00302c20
 */
int rtNetInetReceiveFrom(LibnetAsyncHandle *connection,
                         char *address,
                         unsigned short *port,
                         void *buffer,
                         unsigned short size,
                         unsigned short *received);

/**
 * Take the next two-byte marker a RT_NET_SOCKET_LISTEN connection has received.
 *
 * @param connection The connection.
 * @param received Receives true when a marker was taken.
 * @param address Receives the dotted address of the sender.
 * @param marker Receives the marker.
 * @return 0, 1 on failure, 2 for a bad argument, or 5 when the driver is not started.
 * @ghidraAddress NTSC-U/C: 0x002f8dc8
 * @ghidraAddress PAL: 0x00302d38
 */
int rtNetInetReceiveMarker(LibnetAsyncHandle *connection,
                           bool *received,
                           char *address,
                           unsigned short *marker);

/**
 * Copy the dotted address of this console that rtNetInetInit() read.
 *
 * @param address Receives the address.
 * @return 0, 2 for a null @p address, or 5 when the driver is not started.
 * @ghidraAddress NTSC-U/C: 0x002f8ea0
 * @ghidraAddress PAL: 0x00302e10
 */
int rtNetInetCopyLocalAddress(char *address);

#ifdef __cplusplus
}
#endif

#endif
