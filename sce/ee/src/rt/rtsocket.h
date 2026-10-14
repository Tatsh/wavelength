#ifndef RTSOCKET_H
#define RTSOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

/** Sockets of the SCE-RT library. The routines report 0 for success. */

/** The size of the text of an IPv4 address, the terminator included. */
#define RT_SOCKET_ADDRESS_SIZE 16

/** Kinds of sockets. */
#define RT_SOCKET_STREAM 0   /*!< A connected stream. */
#define RT_SOCKET_DATAGRAM 1 /*!< Datagrams to and from any address. */

/** The options of a new socket. */
typedef struct {
    int nType;       /*!< The kind of socket, an RT_SOCKET_ value. */
    int nBufferSize; /*!< The size of the buffers of the socket. */
} RtSocketOptions;

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
 * @param pnSocket Receives the socket.
 * @param pszAddress The address to connect to, or null.
 * @param uPort The port.
 * @param pOptions The options, or null for the defaults.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ef8a8
 * @ghidraAddress PAL: 0x002f9818
 */
int RtSocketCreate(int *pnSocket,
                   const char *pszAddress,
                   unsigned short uPort,
                   const RtSocketOptions *pOptions);

/**
 * Close a socket.
 *
 * @param nSocket The socket.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ef998
 * @ghidraAddress PAL: 0x002f9908
 */
int RtSocketClose(int nSocket);

/**
 * Send bytes on a stream.
 *
 * @param nSocket The socket.
 * @param pData The bytes.
 * @param uSize The number of bytes.
 * @param pnSent Receives the bytes sent.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002efa00
 * @ghidraAddress PAL: 0x002f9970
 */
int RtSocketSend(int nSocket, const void *pData, unsigned short uSize, short *pnSent);

/**
 * Send a datagram.
 *
 * @param nSocket The socket.
 * @param pszAddress The address to send to.
 * @param uPort The port to send to.
 * @param pData The bytes.
 * @param uSize The number of bytes.
 * @param pnSent Receives the bytes sent.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002efaa8
 * @ghidraAddress PAL: 0x002f9a18
 */
int RtSocketSendTo(int nSocket,
                   const char *pszAddress,
                   unsigned short uPort,
                   const void *pData,
                   unsigned short uSize,
                   short *pnSent);

/**
 * Receive bytes from a stream without waiting.
 *
 * @param nSocket The socket.
 * @param pBuffer Receives the bytes.
 * @param uSize The size of the buffer.
 * @param puReceived Receives the bytes received.
 * @return 0, 13 when the peer closed the stream, or another error.
 * @ghidraAddress NTSC-U/C: 0x002efc18
 * @ghidraAddress PAL: 0x002f9b88
 */
int RtSocketReceive(int nSocket, void *pBuffer, unsigned short uSize, unsigned short *puReceived);

/**
 * Receive a datagram without waiting.
 *
 * @param nSocket The socket.
 * @param pszAddress Receives the address of the sender.
 * @param puPort Receives the port of the sender.
 * @param pBuffer Receives the bytes.
 * @param uSize The size of the buffer.
 * @param pnReceived Receives the bytes received.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002efcc0
 * @ghidraAddress PAL: 0x002f9c30
 */
int RtSocketReceiveFrom(int nSocket,
                        char *pszAddress,
                        unsigned short *puPort,
                        void *pBuffer,
                        unsigned short uSize,
                        short *pnReceived);

/**
 * Check a listening socket for a peer at an address.
 *
 * @param nSocket The listening socket.
 * @param pbReady Receives non-zero when the peer is there.
 * @param pszAddress The address of the peer.
 * @param uPort The port of the peer.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002efb70
 * @ghidraAddress PAL: 0x002f9ae0
 */
int RtSocketPollListener(int nSocket,
                         unsigned char *pbReady,
                         const char *pszAddress,
                         unsigned short uPort);

/**
 * Accept the next peer of a listening socket without waiting.
 *
 * @param nSocket The listening socket.
 * @param pbAccepted Receives 1 when a peer was accepted.
 * @param pszAddress Receives the address of the peer.
 * @param puPort Receives the port of the peer.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002efd88
 * @ghidraAddress PAL: 0x002f9cf8
 */
int RtSocketAccept(int nSocket,
                   unsigned char *pbAccepted,
                   char *pszAddress,
                   unsigned short *puPort);

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
