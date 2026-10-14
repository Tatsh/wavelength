#ifndef RTSOCKET_H
#define RTSOCKET_H

#include <stdbool.h>

#include <libnet.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Sockets of the SCE-RT library. The routines report 0 for success. */

/** The size of the text of an IPv4 address, the terminator included. */
#define RT_SOCKET_ADDRESS_SIZE 16

/** Kinds of sockets, the values of RtSocketOptions::nType. */
#define RT_SOCKET_STREAM 0          /*!< A stream socket that connects to an address. */
#define RT_SOCKET_DATAGRAM 1        /*!< Datagrams to and from any address. */
#define RT_SOCKET_STREAM_ACCEPTED 2 /*!< A stream socket that waits for a peer. */
#define RT_SOCKET_LISTEN 3          /*!< A raw socket that exchanges two-byte markers. */
#define RT_SOCKET_TYPE_COUNT 4      /*!< The number of kinds of sockets. */

/** The options of a new socket. */
typedef struct {
    int nType;       /*!< The kind of socket, an RT_SOCKET_ value. */
    int nBufferSize; /*!< The largest datagram of a RT_SOCKET_DATAGRAM socket in bytes. */
} RtSocketOptions;

/** A socket. */
typedef struct {
    unsigned char allocated;       /*!< 1 while the record is in use. */
    unsigned char open;            /*!< 1 while the connection of the IOP stack is open. */
    LibnetAsyncHandle *connection; /*!< The connection of the IOP stack. */
    RtSocketOptions options;       /*!< The options the socket was opened with. */
    unsigned short port;           /*!< The local port. */
} RtSocket;

/**
 * Start the socket layer, counting the starts.
 *
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ef800
 * @ghidraAddress PAL: 0x002f9770
 */
int RtSocketStartup(void);

/**
 * Undo one start of the socket layer, stopping it at the last.
 *
 * @return 0, 4 when it was not started, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ef860
 * @ghidraAddress PAL: 0x002f97d0
 */
int RtSocketShutdown(void);

/**
 * Create a socket. A stream connects to the address, and a datagram socket binds to the port.
 *
 * @param ppSocket Receives the socket, or null on failure.
 * @param pszAddress The address a RT_SOCKET_STREAM socket connects to, otherwise null.
 * @param uPort The port.
 * @param pOptions The options, or null for a RT_SOCKET_STREAM socket.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ef8a8
 * @ghidraAddress PAL: 0x002f9818
 */
int RtSocketCreate(RtSocket **ppSocket,
                   const char *pszAddress,
                   unsigned short uPort,
                   const RtSocketOptions *pOptions);

/**
 * Close a socket and release its record.
 *
 * @param pSocket The socket.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ef998
 * @ghidraAddress PAL: 0x002f9908
 */
int RtSocketClose(RtSocket *pSocket);

/**
 * Send bytes on a stream.
 *
 * @param pSocket The socket.
 * @param pData The bytes.
 * @param uSize The number of bytes.
 * @param puSent Receives the bytes sent.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002efa00
 * @ghidraAddress PAL: 0x002f9970
 */
int RtSocketSend(RtSocket *pSocket,
                 const void *pData,
                 unsigned short uSize,
                 unsigned short *puSent);

/**
 * Send a datagram.
 *
 * @param pSocket The socket.
 * @param pszAddress The address to send to.
 * @param uPort The port to send to.
 * @param pData The bytes.
 * @param uSize The number of bytes.
 * @param puSent Receives the bytes sent.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002efaa8
 * @ghidraAddress PAL: 0x002f9a18
 */
int RtSocketSendTo(RtSocket *pSocket,
                   const char *pszAddress,
                   unsigned short uPort,
                   const void *pData,
                   unsigned short uSize,
                   unsigned short *puSent);

/**
 * Receive bytes from a stream without waiting.
 *
 * @param pSocket The socket.
 * @param pBuffer Receives the bytes.
 * @param uSize The size of the buffer.
 * @param puReceived Receives the bytes received.
 * @return 0, 13 when the peer closed the stream, or another error.
 * @ghidraAddress NTSC-U/C: 0x002efc18
 * @ghidraAddress PAL: 0x002f9b88
 */
int RtSocketReceive(RtSocket *pSocket,
                    void *pBuffer,
                    unsigned short uSize,
                    unsigned short *puReceived);

/**
 * Receive a datagram without waiting.
 *
 * @param pSocket The socket.
 * @param pszAddress Receives the address of the sender.
 * @param puPort Receives the port of the sender.
 * @param pBuffer Receives the bytes.
 * @param uSize The size of the buffer.
 * @param puReceived Receives the bytes received, 0 when no datagram is waiting.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002efcc0
 * @ghidraAddress PAL: 0x002f9c30
 */
int RtSocketReceiveFrom(RtSocket *pSocket,
                        char *pszAddress,
                        unsigned short *puPort,
                        void *pBuffer,
                        unsigned short uSize,
                        unsigned short *puReceived);

/**
 * Queue a two-byte marker on a RT_SOCKET_LISTEN socket.
 *
 * @param pSocket The socket.
 * @param pbReady Receives true once the marker was queued.
 * @param pszAddress The address of the peer.
 * @param uPort The marker, the port of the peer.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002efb70
 * @ghidraAddress PAL: 0x002f9ae0
 */
int RtSocketPollListener(RtSocket *pSocket,
                         bool *pbReady,
                         const char *pszAddress,
                         unsigned short uPort);

/**
 * Take the next two-byte marker a RT_SOCKET_LISTEN socket has received, without waiting.
 *
 * @param pSocket The socket.
 * @param pbAccepted Receives true when a marker arrived.
 * @param pszAddress Receives the address of the peer.
 * @param puPort Receives the marker, the port of the peer.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002efd88
 * @ghidraAddress PAL: 0x002f9cf8
 */
int RtSocketAccept(RtSocket *pSocket, bool *pbAccepted, char *pszAddress, unsigned short *puPort);

/**
 * Write the address of this console as text.
 *
 * @param pszAddress Receives the address, RT_SOCKET_ADDRESS_SIZE bytes.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002efe50
 * @ghidraAddress PAL: 0x002f9dc0
 */
int RtSocketGetLocalAddress(char *pszAddress);

#ifdef __cplusplus
}
#endif

#endif
