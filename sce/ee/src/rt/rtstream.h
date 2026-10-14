#ifndef RTSTREAM_H
#define RTSTREAM_H

#include "rt/rttime.h"
#include "rt/rtupnp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Datagram transport of the SCE-RT library. Records to a peer are merged into one datagram and
 * queued. A record flagged RT_STREAM_FLAG_GUARANTEED is kept until the peer acknowledges it, and
 * with RT_STREAM_FLAG_ORDERED it is delivered in order. A record flagged only
 * RT_STREAM_FLAG_ORDERED is delivered when it is newer than the last one, and any other record as
 * it arrives. The routines report 0 for success or an RT_STREAM_RESULT_ code.
 */

/** The flags of a record. */
#define RT_STREAM_FLAG_ORDERED 0x10    /*!< Deliver in order, or drop older records. */
#define RT_STREAM_FLAG_GUARANTEED 0x40 /*!< Resend until acknowledged. */
#define RT_STREAM_FLAG_FLUSH 0x80      /*!< Send the queue at the next tick. */

/** The results of the routines. */
#define RT_STREAM_RESULT_FAILED 1          /*!< A socket, an allocation, or the mapping failed. */
#define RT_STREAM_RESULT_NOT_SENT 2        /*!< The record could not be queued. */
#define RT_STREAM_RESULT_INVALID 3         /*!< An argument is null or out of range. */
#define RT_STREAM_RESULT_NO_SLOT 7         /*!< No state is left for another peer. */
#define RT_STREAM_RESULT_ALREADY_STARTED 8 /*!< The transport is running. */
#define RT_STREAM_RESULT_NOT_STARTED 9     /*!< The transport is not running. */
#define RT_STREAM_RESULT_TOO_LARGE 10      /*!< The record does not fit in a datagram. */

/** The kinds of record on the wire. */
#define RT_STREAM_RECORD_PLAIN 0 /*!< Data delivered as it arrives. */
#define RT_STREAM_RECORD_DATA 1  /*!< Data with a sequence number. */
#define RT_STREAM_RECORD_QUERY 2 /*!< The sequences the sender still holds. */
#define RT_STREAM_RECORD_ACK 3   /*!< The queried sequences the receiver holds. */
#define RT_STREAM_RECORD_NAK 4   /*!< A queried sequence the receiver is missing. */

/** The largest datagram. */
#define RT_STREAM_DATAGRAM_SIZE 520

/** The records a stream keeps. */
#define RT_STREAM_MAX_RECORDS 245

/** The peers the transport tracks. */
#define RT_STREAM_MAX_PEERS 18

/** The size of the text of an address, the terminator included. */
#define RT_STREAM_ADDRESS_SIZE 16

/** An IPv4 address. */
typedef struct {
    unsigned char abOctets[4]; /*!< The octets, most significant first. */
} RtIpv4Address;

/** The header of a data record. */
typedef struct {
    unsigned char uType;      /*!< An RT_STREAM_RECORD_ kind. */
    unsigned char uFlags;     /*!< The RT_STREAM_FLAG_ bits. */
    unsigned short uSequence; /*!< The sequence number. */
    unsigned int uSize;       /*!< The size of the data after the header. */
} RtStreamRecordHeader;

/** The header of a query, an acknowledgement, or a negative acknowledgement. */
typedef struct {
    unsigned char uType;     /*!< An RT_STREAM_RECORD_ kind. */
    unsigned char uReserved; /*!< Never set. */
    unsigned short uValue;   /*!< The count of sequences, or the sequence of a NAK. */
} RtStreamListHeader;

/** The last sequence of the ordered records of one peer that are not guaranteed. */
typedef struct {
    RtIpv4Address address;    /*!< The peer, zero when the entry is free. */
    unsigned short uPort;     /*!< The port of the peer. */
    unsigned short uSequence; /*!< The last sequence. */
} RtStreamLatest;

/** The reliable records of one peer, sent or received. */
typedef struct {
    RtIpv4Address address;    /*!< The peer. */
    unsigned short uPort;     /*!< The port of the peer. */
    unsigned short uSequence; /*!< The last sequence sent or delivered. */
    unsigned int uLastQuery;  /*!< The time of the last query sent. */
    unsigned int uLastAck;    /*!< The time the peer last answered. */
    int nRecordCount;         /*!< The records kept. */
    int nNextRecord;          /*!< The next entry a received record takes. */
    unsigned short auSequence[RT_STREAM_MAX_RECORDS]; /*!< The sequence of each record. */
    unsigned int auTime[RT_STREAM_MAX_RECORDS];       /*!< When each record was kept. */
    unsigned char *apbData[RT_STREAM_MAX_RECORDS];    /*!< The data of each record, or null. */
    int anSize[RT_STREAM_MAX_RECORDS];                /*!< The size of each record. */
    unsigned char auFlags[RT_STREAM_MAX_RECORDS];     /*!< The flags of each record. */
} RtStreamSlot;

/** A peer the transport heard from. */
typedef struct {
    char szAddress[RT_STREAM_ADDRESS_SIZE]; /*!< The address, empty when the entry is free. */
    int nPort;                              /*!< The port. */
    RtTime lastHeard;                       /*!< When the peer was last heard. */
} RtStreamPeer;

/**
 * Receiver of a peer that was not heard for too long.
 *
 * @param pszAddress The address of the peer.
 * @param uPort The port of the peer.
 */
typedef void (*RtStreamTimeoutFunc)(const char *pszAddress, unsigned short uPort);

/**
 * Receiver of the result of RtStreamMapPort().
 *
 * @param pStream Always 0.
 * @param pszAddress The address the gateway shows, or empty.
 * @param uPort The port the gateway shows, or 0.
 * @param nFailed Non-zero when the mapping failed.
 */
typedef void (*RtStreamMappingFunc)(RtSocket *pStream,
                                    const char *pszAddress,
                                    unsigned short uPort,
                                    int nFailed);

/** The datagram socket. */
extern RtSocket *g_pStreamSocket;
/** The UPnP client that maps the port of the socket. */
extern RtUpnpContext *g_pStreamUpnp;
/** The receiver of the result of RtStreamMapPort(). */
extern RtStreamMappingFunc g_pfnStreamMapping;
/** The service the gateway mapped the port with. */
extern int g_nStreamMappedService;
/** The address of the gateway, whose datagrams are not queued. */
extern char g_szStreamGateway[RT_STREAM_ADDRESS_SIZE];
/** The port of the gateway. */
extern unsigned short g_uStreamGatewayPort;
/** The records merged for the next datagram. */
extern unsigned char g_abStreamMerge[RT_STREAM_DATAGRAM_SIZE];
/** The size of the records merged. */
extern int g_nStreamMergeSize;
/** The queue of datagrams to send. */
extern unsigned char *g_pbStreamTxQueue;
/** The end of the datagrams to send. */
extern unsigned char *g_pbStreamTxQueueEnd;
/** The size of the queue of datagrams to send. */
extern int g_nStreamTxQueueSize;
/** The queue of datagrams received. */
extern unsigned char *g_pbStreamRxQueue;
/** The end of the datagrams received. */
extern unsigned char *g_pbStreamRxQueueEnd;
/** The size of the queue of datagrams received. */
extern int g_nStreamRxQueueSize;
/** When the queue of datagrams to send was last sent. */
extern RtTime g_streamLastFlush;
/** The records flagged RT_STREAM_FLAG_FLUSH queued since then. */
extern unsigned char g_uStreamFlushCount;
/** The milliseconds between sends of the queue. */
extern unsigned int g_uStreamFlushInterval;

/**
 * Report the result of the port mapping to the receiver RtStreamMapPort() was given.
 *
 * @param pContext The UPnP client.
 * @param nResult 0 for success.
 * @param nService The service the port was mapped with.
 * @param pszAddress The address the gateway shows.
 * @param uPort The port the gateway shows.
 * @param pUser Not read.
 * @ghidraAddress NTSC-U/C: 0x002c5ba8
 * @ghidraAddress PAL: 0x002cfb18
 */
void RtStreamOnMapping(struct RtUpnpContext *pContext,
                       int nResult,
                       int nService,
                       const char *pszAddress,
                       unsigned short uPort,
                       void *pUser);

/**
 * Queue every datagram the socket has, except those of the gateway, and note their senders as
 * heard.
 *
 * @ghidraAddress NTSC-U/C: 0x002c5c30
 * @ghidraAddress PAL: 0x002cfba0
 */
void RtStreamReadDatagrams(void);

/**
 * Drop the queued datagrams of a peer, or every queued datagram.
 *
 * @param pszAddress The peer, or null for every datagram.
 * @param uPort The port of the peer.
 * @ghidraAddress NTSC-U/C: 0x002c5db0
 * @ghidraAddress PAL: 0x002cfd20
 */
void RtStreamDropDatagrams(const char *pszAddress, unsigned short uPort);

/**
 * Take the oldest queued datagram.
 *
 * @param pszAddress Receives the sender, RT_STREAM_ADDRESS_SIZE bytes.
 * @param puPort Receives the port of the sender.
 * @param puSize Receives the size of the datagram, 0 when none is queued.
 * @param pbData Receives the datagram.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c5f10
 * @ghidraAddress PAL: 0x002cfe80
 */
int RtStreamTakeDatagram(char *pszAddress,
                         unsigned short *puPort,
                         unsigned short *puSize,
                         unsigned char *pbData);

/**
 * Report the room left in the queue of datagrams to send.
 *
 * @param pStream Not read.
 * @return The room in bytes.
 * @ghidraAddress NTSC-U/C: 0x002c6098
 * @ghidraAddress PAL: 0x002d0008
 */
int RtStreamGetTxRoom(RtSocket *pStream);

/**
 * Queue records to send, appending them to a queued datagram to the same peer when they fit.
 *
 * @param pStream Not read.
 * @param pszAddress The peer.
 * @param uPort The port of the peer.
 * @param uFlags The flags of the records.
 * @param nSize The size of the records.
 * @param pData The records.
 * @return Non-zero when the records were queued.
 * @ghidraAddress NTSC-U/C: 0x002c60c0
 * @ghidraAddress PAL: 0x002d0030
 */
int RtStreamQueueTx(RtSocket *pStream,
                    const char *pszAddress,
                    unsigned short uPort,
                    unsigned char uFlags,
                    int nSize,
                    const void *pData);

/**
 * Remove sent bytes from the queue of datagrams to send.
 *
 * @param pbSpan The bytes.
 * @param nSize The number of bytes.
 * @ghidraAddress NTSC-U/C: 0x002c6380
 * @ghidraAddress PAL: 0x002d02f0
 */
void RtStreamRemoveTx(unsigned char *pbSpan, int nSize);

/**
 * Send the queued datagrams until the socket takes one only in part.
 *
 * @param pStream Not read.
 * @ghidraAddress NTSC-U/C: 0x002c6410
 * @ghidraAddress PAL: 0x002d0380
 */
void RtStreamFlushTx(RtSocket *pStream);

/**
 * Queue records to send, and send the queue when it is due.
 *
 * @param pStream Not read.
 * @param pszAddress The peer.
 * @param uPort The port of the peer.
 * @param uFlags The flags of the records.
 * @param pData The records.
 * @param nSize The size of the records.
 * @param puQueued Receives the size queued, or 0.
 * @ghidraAddress NTSC-U/C: 0x002c6550
 * @ghidraAddress PAL: 0x002d04c0
 */
void RtStreamSendTx(RtSocket *pStream,
                    const char *pszAddress,
                    unsigned short uPort,
                    unsigned char uFlags,
                    const void *pData,
                    int nSize,
                    unsigned short *puQueued);

/**
 * Append bytes to the records merged for the next datagram, when they fit.
 *
 * @param pData The bytes.
 * @param nSize The number of bytes.
 * @ghidraAddress NTSC-U/C: 0x002c65f0
 * @ghidraAddress PAL: 0x002d0560
 */
void RtStreamMerge(const void *pData, int nSize);

/**
 * Queue the records merged as one datagram to a peer.
 *
 * @param pStream Not read.
 * @param pszAddress The peer.
 * @param uPort The port of the peer.
 * @param uFlags The flags of the records.
 * @return The size queued, or 0.
 * @ghidraAddress NTSC-U/C: 0x002c6668
 * @ghidraAddress PAL: 0x002d05d8
 */
unsigned short RtStreamFlushMerge(RtSocket *pStream,
                                  const char *pszAddress,
                                  unsigned short uPort,
                                  unsigned char uFlags);

/**
 * Report the sequence after another, wrapping before 0xfffe.
 *
 * @param uSequence The sequence.
 * @return The next sequence.
 * @ghidraAddress NTSC-U/C: 0x002c66b8
 * @ghidraAddress PAL: 0x002d0628
 */
unsigned short RtStreamNextSequence(unsigned short uSequence);

/**
 * Report whether a sequence is newer than another, across the wrap.
 *
 * @param nSequence The sequence.
 * @param nLast The other sequence.
 * @return Non-zero when nSequence is newer.
 * @ghidraAddress NTSC-U/C: 0x002c66e0
 * @ghidraAddress PAL: 0x002d0650
 */
int RtStreamIsNewer(int nSequence, int nLast);

/**
 * Parse an address written as `a.b.c.d`.
 *
 * @param pAddress Receives the address.
 * @param pszAddress The text.
 * @ghidraAddress NTSC-U/C: 0x002c6710
 * @ghidraAddress PAL: 0x002d0680
 */
void RtStreamParseAddress(RtIpv4Address *pAddress, const char *pszAddress);

/**
 * Write an address as `a.b.c.d`.
 *
 * @param pszAddress Receives the text.
 * @param pAddress The address.
 * @return The length of the text.
 * @ghidraAddress NTSC-U/C: 0x002c67d8
 * @ghidraAddress PAL: 0x002d0748
 */
int RtStreamFormatAddress(char *pszAddress, const RtIpv4Address *pAddress);

/**
 * Start the sockets and the traffic statistics, and forget the gateway.
 *
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c6828
 * @ghidraAddress PAL: 0x002d0798
 */
int RtStreamStartup(void);

/**
 * Stop the sockets.
 *
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002c6860
 * @ghidraAddress PAL: 0x002d07d0
 */
int RtStreamCleanup(void);

/**
 * Open the socket, the UPnP client, and the queues of the transport.
 *
 * @param ppStream Receives the stream, always null.
 * @param nPort The port of the socket.
 * @param nPeerTimeout The milliseconds a peer may stay silent, or 0 for 45000.
 * @param nTxQueueSize The size of the queue of datagrams to send, at least 16384.
 * @param nRxQueueSize The size of the queue of datagrams received, at least 4096.
 * @param uFlushInterval The milliseconds between sends of the queue.
 * @param pfnTimeout Receives the peers that stayed silent too long, or null.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c6880
 * @ghidraAddress PAL: 0x002d07f0
 */
int RtStreamOpen(RtSocket **ppStream,
                 int nPort,
                 int nPeerTimeout,
                 int nTxQueueSize,
                 int nRxQueueSize,
                 unsigned int uFlushInterval,
                 RtStreamTimeoutFunc pfnTimeout);

/**
 * Run the transport for half a second, then release it.
 *
 * @param pStream The stream.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c6b18
 * @ghidraAddress PAL: 0x002d0a88
 */
int RtStreamClose(RtSocket *pStream);

/**
 * Note a peer as heard.
 *
 * @param pStream Not read.
 * @param pszAddress The peer.
 * @param uPort The port of the peer.
 * @ghidraAddress NTSC-U/C: 0x002c7ee8
 * @ghidraAddress PAL: 0x002d1e58
 */
void RtStreamTouchPeer(RtSocket *pStream, const char *pszAddress, unsigned short uPort);

/**
 * Update the statistics, advance the port mapping, send the queue when it is due, query the
 * unacknowledged records, time out silent peers, and read the socket.
 *
 * @param pStream The stream.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c8048
 * @ghidraAddress PAL: 0x002d1fb8
 */
int RtStreamTick(RtSocket *pStream);

/**
 * Forget the records and the datagrams of a peer.
 *
 * @param pStream The stream.
 * @param pszAddress The peer.
 * @param nPort The port of the peer.
 * @param bKeep Non-zero to forget nothing.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c80f0
 * @ghidraAddress PAL: 0x002d2060
 */
int RtStreamDisconnect(RtSocket *pStream, const char *pszAddress, int nPort, unsigned char bKeep);

/**
 * Send a record to a peer, by the delivery its flags ask for.
 *
 * @param pStream The stream.
 * @param nFlags The RT_STREAM_FLAG_ bits.
 * @param pszAddress The peer.
 * @param nPort The port of the peer.
 * @param pData The record.
 * @param uSize The size of the record.
 * @param puSent Receives the size sent, or 0.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c85d0
 * @ghidraAddress PAL: 0x002d2540
 */
int RtStreamSend(RtSocket *pStream,
                 int nFlags,
                 const char *pszAddress,
                 int nPort,
                 const void *pData,
                 unsigned short uSize,
                 unsigned short *puSent);

/**
 * Deliver the data received, from the records in order or from the next queued datagram.
 *
 * @param pStream The stream.
 * @param pszAddress Receives the sender.
 * @param pnPort Receives the port of the sender.
 * @param pBuffer Receives the data.
 * @param uSize The size of the buffer.
 * @param puReceived Receives the size delivered.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c9748
 * @ghidraAddress PAL: 0x002d36b8
 */
int RtStreamReceive(RtSocket *pStream,
                    char *pszAddress,
                    int *pnPort,
                    void *pBuffer,
                    unsigned short uSize,
                    unsigned short *puReceived);

/**
 * Map the port of the socket on the gateway.
 *
 * @param pStream Not read.
 * @param pszGateway The address of the gateway.
 * @param nPort The port of the gateway.
 * @param pfnDone Receives the result.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c99c0
 * @ghidraAddress PAL: 0x002d3930
 */
int RtStreamMapPort(RtSocket *pStream,
                    const char *pszGateway,
                    int nPort,
                    RtStreamMappingFunc pfnDone);

#ifdef __cplusplus
}
#endif

#endif
