#ifndef LIBRTNET_H
#define LIBRTNET_H

#include <stdbool.h>

#include <libnet.h>
#include <librtinet.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Socket layer of the SCE-RT network library.
 *
 * The names of the types, the routines, and their parameters are inferred.
 */

/** A socket of the library. */
typedef struct {
    unsigned char allocated;       /*!< 1 while the record is in use. */
    unsigned char open;            /*!< 1 while the connection of the IOP stack is open. */
    LibnetAsyncHandle *connection; /*!< The connection of the IOP stack. */
    rtNetSocketOptions options;    /*!< The options the socket was opened with. */
    unsigned short port;           /*!< The local port. */
} rtNetSocket;

/**
 * Start the socket layer, counting the users.
 *
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002ef800
 * @ghidraAddress PAL: 0x002f9770
 */
int rtNetSocketInit(void);

/**
 * Stop the socket layer once its last user stops it.
 *
 * @return 0, 4 when the layer is not started, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002ef860
 * @ghidraAddress PAL: 0x002f97d0
 */
int rtNetSocketShutdown(void);

/**
 * Open a socket.
 *
 * @param socket Receives the socket, or null on failure.
 * @param address The dotted address to connect a RT_NET_SOCKET_STREAM socket to, otherwise null.
 * @param port The local port.
 * @param options The options, or null for a RT_NET_SOCKET_STREAM socket.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002ef8a8
 * @ghidraAddress PAL: 0x002f9818
 */
int rtNetSocketCreate(rtNetSocket **socket,
                      const char *address,
                      unsigned short port,
                      const rtNetSocketOptions *options);

/**
 * Close a socket and release its record.
 *
 * @param socket The socket.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002ef998
 * @ghidraAddress PAL: 0x002f9908
 */
int rtNetSocketClose(rtNetSocket *socket);

/**
 * Send bytes on a stream socket.
 *
 * @param socket The socket.
 * @param data The bytes.
 * @param length The number of bytes.
 * @param sent Receives the number of bytes sent.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002efa00
 * @ghidraAddress PAL: 0x002f9970
 */
int rtNetSocketSendStream(rtNetSocket *socket,
                          const void *data,
                          unsigned short length,
                          unsigned short *sent);

/**
 * Send a datagram.
 *
 * @param socket The socket.
 * @param address The dotted address of the receiver.
 * @param port The port of the receiver.
 * @param data The bytes.
 * @param length The number of bytes.
 * @param sent Receives the number of bytes sent.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002efaa8
 * @ghidraAddress PAL: 0x002f9a18
 */
int rtNetSocketSendTo(rtNetSocket *socket,
                      const char *address,
                      unsigned short port,
                      const void *data,
                      unsigned short length,
                      unsigned short *sent);

/**
 * Start waiting for stream peers on a RT_NET_SOCKET_LISTEN socket.
 *
 * @param socket The socket.
 * @param listening Receives true once the socket waits.
 * @param address The dotted local address.
 * @param port The local port.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002efb70
 * @ghidraAddress PAL: 0x002f9ae0
 */
int rtNetSocketListen(rtNetSocket *socket,
                      bool *listening,
                      const char *address,
                      unsigned short port);

/**
 * Receive bytes on a stream socket.
 *
 * @param socket The socket.
 * @param buffer Receives the bytes.
 * @param size The size of the buffer in bytes.
 * @param received Receives the number of bytes received.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002efc18
 * @ghidraAddress PAL: 0x002f9b88
 */
int rtNetSocketReceiveStream(rtNetSocket *socket,
                             void *buffer,
                             unsigned short size,
                             unsigned short *received);

/**
 * Receive a datagram.
 *
 * @param socket The socket.
 * @param address Receives the dotted address of the sender.
 * @param port Receives the port of the sender.
 * @param buffer Receives the bytes.
 * @param size The size of the buffer in bytes.
 * @param received Receives the number of bytes received.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002efcc0
 * @ghidraAddress PAL: 0x002f9c30
 */
int rtNetSocketReceiveFrom(rtNetSocket *socket,
                           char *address,
                           unsigned short *port,
                           void *buffer,
                           unsigned short size,
                           unsigned short *received);

/**
 * Check a RT_NET_SOCKET_LISTEN socket for a stream peer.
 *
 * @param socket The socket.
 * @param accepted Receives true when a peer arrived.
 * @param address Receives the dotted address of the peer.
 * @param port Receives the port of the peer.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002efd88
 * @ghidraAddress PAL: 0x002f9cf8
 */
int rtNetSocketAccept(rtNetSocket *socket, bool *accepted, char *address, unsigned short *port);

/**
 * Write the dotted address of this console.
 *
 * @param address Receives the address.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002efe50
 * @ghidraAddress PAL: 0x002f9dc0
 */
int rtNetGetLocalAddress(char *address);

// Routines of the network core that the network core headers do not declare yet.

/** A moment of the clock of the library. */
typedef struct {
    unsigned int seconds;      /*!< The whole seconds. */
    unsigned int microseconds; /*!< The microseconds past the seconds. */
} rtNetTime;

/**
 * Read the clock of the library.
 *
 * @param time Receives the moment.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002c9e58
 * @ghidraAddress PAL: 0x002d3dc8
 */
int rtNetTimeNow(rtNetTime *time);

/**
 * Measure the time since a moment.
 *
 * @param since The moment.
 * @param elapsed Receives the time since the moment in milliseconds.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002c9eac
 * @ghidraAddress PAL: 0x002d3e1c
 */
int rtNetTimeElapsedMs(const rtNetTime *since, unsigned int *elapsed);

#ifdef __cplusplus
}
#endif

#endif
