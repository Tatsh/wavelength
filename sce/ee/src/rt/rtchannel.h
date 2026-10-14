#ifndef RTCHANNEL_H
#define RTCHANNEL_H

#include "rt/rtlargeint.h"
#include "rt/rtringbuffer.h"
#include "rt/rttime.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Channels of the SCE-RT library. A channel is a connection to a server, over a stream link and,
 * for a game host, a datagram link too. Each link batches typed records into a packet, frames the
 * packets into its send ring, and parses the records its receive ring gathers. The routines report
 * 0 for success or an RT_CHANNEL_RESULT_ code.
 */

/** The results of the routines. */
#define RT_CHANNEL_RESULT_SHORT 1          /*!< A record is too short. */
#define RT_CHANNEL_RESULT_INVALID 2        /*!< An argument is null or out of range. */
#define RT_CHANNEL_RESULT_UNKNOWN_RECORD 3 /*!< The record type is not handled. */
#define RT_CHANNEL_RESULT_NOT_STARTED 5    /*!< The channels are not started. */
#define RT_CHANNEL_RESULT_SOCKET 6         /*!< A socket failed. */
#define RT_CHANNEL_RESULT_FIELD 7          /*!< A field could not be read or written. */
#define RT_CHANNEL_RESULT_NO_MEMORY 8      /*!< An allocation failed. */
#define RT_CHANNEL_RESULT_NO_LINK 9        /*!< The link is missing. */
#define RT_CHANNEL_RESULT_CLOSED 11        /*!< A callback closed the channel. */
#define RT_CHANNEL_RESULT_TIMEOUT 13       /*!< A send did not finish in time. */
#define RT_CHANNEL_RESULT_BAD_VERSION 16   /*!< The peer runs an incompatible protocol. */
#define RT_CHANNEL_RESULT_NO_ROOM 20       /*!< The send ring has too little room. */
#define RT_CHANNEL_RESULT_RING 21          /*!< A ring buffer failed. */
#define RT_CHANNEL_RESULT_BAD_STATE 23     /*!< The state is out of range. */
#define RT_CHANNEL_RESULT_CLOCK 24         /*!< The clock failed. */
#define RT_CHANNEL_RESULT_NOT_SIGNED 27 /*!< Signing or decryption needs keys that are missing. */
#define RT_CHANNEL_RESULT_NO_ENCRYPTION 28 /*!< An encrypted record arrived without a header. */

/** The modes of a channel. */
#define RT_CHANNEL_MODE_CLIENT 0 /*!< A stream link only. */
#define RT_CHANNEL_MODE_HOST 1   /*!< A stream link and a datagram link. */

/** The transports of a link. */
#define RT_CHANNEL_TRANSPORT_STREAM 0   /*!< A connected stream. */
#define RT_CHANNEL_TRANSPORT_DATAGRAM 1 /*!< Datagrams to one address. */

/** The links of a channel. */
#define RT_CHANNEL_LINK_STREAM 0   /*!< The stream link. */
#define RT_CHANNEL_LINK_DATAGRAM 1 /*!< The datagram link of a host. */
#define RT_CHANNEL_LINK_COUNT 2    /*!< The number of links. */

/** The peer that stands for every peer. */
#define RT_CHANNEL_ALL_PEERS 0xffff

/** The peer RtChannelQueueRecord() notes when flagged records went to several peers. */
#define RT_CHANNEL_MIXED_PEERS 0xfffe

/** The largest data of the records batched into one packet. */
#define RT_CHANNEL_PACKET_DATA 512

/** The largest packet a link frames, and the largest datagram it sends. */
#define RT_CHANNEL_MAX_PACKET 584

/** The type bit that marks an encrypted record. */
#define RT_CHANNEL_RECORD_ENCRYPTED 0x80

/** The size of a record header. */
#define RT_CHANNEL_RECORD_HEADER_SIZE 3

/** The size of the text of an address, the terminator included. */
#define RT_CHANNEL_ADDRESS_SIZE 16

/** The size of the tokens a connection request and its reply carry. */
#define RT_CHANNEL_TOKEN_SIZE 16

/** The header of a record. */
typedef struct {
    unsigned char uType;  /*!< The type, with RT_CHANNEL_RECORD_ENCRYPTED. */
    unsigned short uSize; /*!< The size of the data after the header. */
} RtRecordHeader;

/** The records batched for the next packet of a link. */
typedef struct {
    int nType;                 /*!< The type of the records. */
    unsigned short uPeer;      /*!< The peer the records go to. */
    unsigned short uSize;      /*!< The size of the data. */
    unsigned short uCount;     /*!< The number of records. */
    unsigned char abData[586]; /*!< The data, at most RT_CHANNEL_PACKET_DATA bytes. */
} RtChannelPacket;

/** A link of a channel. */
typedef struct {
    int nTransport;                          /*!< An RT_CHANNEL_TRANSPORT_ value. */
    unsigned char bAllocated;                /*!< Set when the link is created. */
    unsigned char bOpen;                     /*!< Non-zero once the link has a socket. */
    unsigned short uChannel;                 /*!< The channel. */
    char szAddress[RT_CHANNEL_ADDRESS_SIZE]; /*!< The address of a datagram link. */
    unsigned short uPort;                    /*!< The port of a datagram link. */
    int nSocket;                             /*!< The socket. */
    RtTime lastSend;                         /*!< When the link last sent. */
    RtTime lastReceive;                      /*!< When the link last received. */
    unsigned int uKeepaliveInterval;         /*!< The milliseconds between keep-alives, or 0. */
    RtTime lastKeepalive;                    /*!< When the link last sent a keep-alive. */
    unsigned short uPending;                 /*!< Keep-alives sent and not acknowledged. */
    unsigned int uFlushInterval;             /*!< The milliseconds a packet may wait, or 0. */
    unsigned char bPeerNoted;                /*!< Non-zero once uPeer was set. */
    unsigned short uPeer;                    /*!< The peer flagged records went to. */
    unsigned int uSendSize;                  /*!< The size of the send ring. */
    unsigned int uReceiveSize;               /*!< The size of the receive ring. */
    RtRingBuffer receive;                    /*!< The bytes received and not parsed. */
    RtRingBuffer send;                       /*!< The packets framed and not sent. */
    RtChannelPacket packet;                  /*!< The records batched for the next packet. */
} RtChannelLink;

/** The name and the key of a peer, as a peer record carries them. */
typedef struct {
    char szName[RT_CHANNEL_ADDRESS_SIZE]; /*!< The name of the peer. */
    RtKey publicKey;                      /*!< The RSA public key, when the record has one. */
} RtChannelPeerInfo;

/**
 * Receiver of the result of a connection.
 *
 * @param uChannel The channel.
 * @param nConnected Non-zero when the channel connected.
 * @param uPeer The peer index the server gave this console.
 * @param uValue The value the server sent.
 * @param nReason Why the connection failed.
 */
typedef void (*RtChannelConnectFunc)(unsigned short uChannel,
                                     int nConnected,
                                     unsigned short uPeer,
                                     unsigned short uValue,
                                     int nReason);

/**
 * Receiver of the data of a peer.
 *
 * @param uChannel The channel.
 * @param uPeer The sender, or RT_CHANNEL_ALL_PEERS.
 * @param pData The data.
 * @param uSize The size of the data.
 */
typedef void (*RtChannelDataFunc)(unsigned short uChannel,
                                  unsigned short uPeer,
                                  const void *pData,
                                  unsigned short uSize);

/**
 * Receiver of a peer that joined.
 *
 * @param uChannel The channel.
 * @param uPeer The peer.
 * @param pInfo The name of the peer and its key.
 * @param bHasKey Non-zero when the record has a public key.
 */
typedef void (*RtChannelPeerFunc)(unsigned short uChannel,
                                  unsigned short uPeer,
                                  const RtChannelPeerInfo *pInfo,
                                  int bHasKey);

/**
 * Receiver of a status the server sent.
 *
 * @param uChannel The channel.
 * @param nStatus The status.
 */
typedef void (*RtChannelStatusFunc)(unsigned short uChannel, int nStatus);

/** The parameters of a channel. */
typedef struct {
    int nMode;                               /*!< An RT_CHANNEL_MODE_ value. */
    char szAddress[RT_CHANNEL_ADDRESS_SIZE]; /*!< The address of the server. */
    unsigned short uPort;                    /*!< The port of the server. */
    unsigned short uClientValue;             /*!< A value of the connection and the requests. */
    unsigned short uLocalPort;               /*!< The port the datagram link of a host binds. */
    RtKey *pServerKey;                       /*!< The RSA public key of the server, or null. */
    unsigned int uKeepaliveInterval;         /*!< The milliseconds between keep-alives, or 0. */
    unsigned int uFlushInterval;             /*!< The milliseconds a packet may wait, or 0. */
    unsigned int uSendSize;                  /*!< The size of the send ring of a link. */
    unsigned int uReceiveSize;               /*!< The size of the receive ring of a link. */
    void *pRequestData;                      /*!< The data of the request after the keys. */
    unsigned short uRequestSize;             /*!< The size of the request data. */
    RtChannelConnectFunc pfnConnect;         /*!< Receives the result of the connection. */
    RtChannelPeerFunc pfnPeer;               /*!< Receives the peers that joined. */
    RtChannelDataFunc pfnData;               /*!< Receives the data of the peers. */
    RtChannelStatusFunc pfnStatus;           /*!< Receives the statuses of the server. */
} RtChannelConfig;

/** A channel. */
typedef struct {
    unsigned short uId;                       /*!< The index of the channel. */
    int nMode;                                /*!< An RT_CHANNEL_MODE_ value. */
    int nState;                               /*!< The state, below 12. */
    RtChannelConfig config;                   /*!< The parameters the channel was opened with. */
    unsigned short uPeer;                     /*!< The peer index the server gave this console. */
    unsigned short uServerValue;              /*!< The value the server sent. */
    char szServerName[RT_CHANNEL_TOKEN_SIZE]; /*!< The name the server sent. */
    unsigned short uConfigValue;              /*!< The value of the configuration record. */
    RtTime connectTime;                       /*!< When the connection request was sent. */
    RtChannelLink *apLinks[RT_CHANNEL_LINK_COUNT]; /*!< The links. */
    RtChannelLink *pActiveLink;                    /*!< The link the requests go out on. */
    RtChannelConnectFunc pfnConnect;               /*!< Receives the result of the connection. */
    RtChannelDataFunc pfnData;                     /*!< Receives the data of the peers. */
    RtChannelPeerFunc pfnPeer;                     /*!< Receives the peers that joined. */
    RtChannelStatusFunc pfnStatus;                 /*!< Receives the statuses of the server. */
} RtChannel;

/** Non-zero while the channels are started. */
extern unsigned char g_bChannelsStarted;
/** The number of entries of the channel table. */
extern unsigned short g_uChannelCount;
/** The channel table. */
extern RtChannel **g_ppChannels;
/** The scratch buffer the rings compact through. */
extern unsigned char *g_pbChannelScratch;
/** The size of the scratch buffer. */
extern unsigned int g_uChannelScratchSize;

/**
 * Create a channel and its links in the first free entry of the table, growing it when it is full.
 *
 * @param puId Receives the channel.
 * @param nMode An RT_CHANNEL_MODE_ value.
 * @param uSendSize The size of the send ring of a link, halved for a host, at least 584.
 * @param uReceiveSize The size of the receive ring of a link, halved for a host, at least 584.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c2a28
 * @ghidraAddress PAL: 0x002cc998
 */
int RtChannelCreate(unsigned short *puId,
                    int nMode,
                    unsigned int uSendSize,
                    unsigned int uReceiveSize);

/**
 * Destroy a channel, its links, and its keys.
 *
 * @param uId The channel.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c2d70
 * @ghidraAddress PAL: 0x002ccce0
 */
int RtChannelDestroy(unsigned short uId);

/**
 * Check that a channel is connecting or connected and that its links are its own.
 *
 * @param pChannel The channel.
 * @return Non-zero when the channel is valid.
 * @ghidraAddress NTSC-U/C: 0x002c2ee8
 * @ghidraAddress PAL: 0x002cce58
 */
int RtChannelIsValid(const RtChannel *pChannel);

/**
 * Check a channel by its index, as RtChannelIsValid() does.
 *
 * @param uId The channel.
 * @return Non-zero when the channel is valid.
 * @ghidraAddress NTSC-U/C: 0x002c2f88
 * @ghidraAddress PAL: 0x002ccef8
 */
int RtChannelIsValidId(unsigned short uId);

/**
 * Look up a channel.
 *
 * @param uId The channel.
 * @param ppChannel Receives the channel, null when the entry is free.
 * @return 0, 1 when there is no table, or 2.
 * @ghidraAddress NTSC-U/C: 0x002c2fd8
 * @ghidraAddress PAL: 0x002ccf48
 */
int RtChannelGet(unsigned short uId, RtChannel **ppChannel);

/**
 * Look up a valid channel.
 *
 * @param uId The channel.
 * @param ppChannel Receives the channel.
 * @return 0, 2, or RT_CHANNEL_RESULT_CLOSED when the channel is missing or not valid.
 * @ghidraAddress NTSC-U/C: 0x002c3028
 * @ghidraAddress PAL: 0x002ccf98
 */
int RtChannelGetValid(unsigned short uId, RtChannel **ppChannel);

/**
 * Check a link pointer.
 *
 * @param pLink The link.
 * @return Non-zero when the link and its rings have addresses.
 * @ghidraAddress NTSC-U/C: 0x002c3078
 * @ghidraAddress PAL: 0x002ccfe8
 */
int RtChannelLinkIsValid(const RtChannelLink *pLink);

/**
 * Queue a record of the modulus of the stored key pair.
 *
 * @param pLink The link.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c30a0
 * @ghidraAddress PAL: 0x002cd010
 */
int RtChannelSendPublicKey(RtChannelLink *pLink);

/**
 * Queue a request record, signed with the key of the game when the channel is encrypted.
 *
 * @param nMode The mode of the channel.
 * @param pLink The link.
 * @param uClientValue The value the request carries.
 * @param pData The data after the signature, or null.
 * @param uSize The size of the data.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c3120
 * @ghidraAddress PAL: 0x002cd090
 */
int RtChannelSendRequest(int nMode,
                         RtChannelLink *pLink,
                         unsigned short uClientValue,
                         const void *pData,
                         unsigned short uSize);

/**
 * Send a disconnection record and wait up to 10 seconds for it to go out.
 *
 * @param pLink The link.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c3470
 * @ghidraAddress PAL: 0x002cd3e0
 */
int RtChannelSendDisconnect(RtChannelLink *pLink);

/**
 * Queue a control record. 0xffff sends nothing, and 0xfffe sends one without a value.
 *
 * @param pLink The link.
 * @param uValue The value.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c34f8
 * @ghidraAddress PAL: 0x002cd468
 */
int RtChannelSendControl(RtChannelLink *pLink, unsigned short uValue);

/**
 * Queue the connection request of a channel on one of its links, and note when it was sent.
 *
 * @param pChannel The channel.
 * @param pLink The link.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c35c0
 * @ghidraAddress PAL: 0x002cd530
 */
int RtChannelSendConnect(RtChannel *pChannel, RtChannelLink *pLink);

/**
 * Batch a record into the packet of a link, framing the packet first when the record does not join
 * it. Records of types other than 2 and 3 are framed at once.
 *
 * @param pLink The link.
 * @param nType The type of the record.
 * @param uPeer The peer the record goes to.
 * @param pData The data, or null.
 * @param uSize The size of the data.
 * @param bTrackPeer 1 to note the peer of the record.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c37a0
 * @ghidraAddress PAL: 0x002cd710
 */
int RtChannelQueueRecord(RtChannelLink *pLink,
                         int nType,
                         unsigned short uPeer,
                         const void *pData,
                         unsigned short uSize,
                         unsigned char bTrackPeer);

/**
 * Parse and handle the whole records in the receive ring of a link, while the state of the
 * channel holds.
 *
 * @param pChannel The channel.
 * @param pLink The link.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c3a50
 * @ghidraAddress PAL: 0x002cd9c0
 */
int RtChannelProcessRecords(RtChannel *pChannel, RtChannelLink *pLink);

/**
 * Send bytes on a link until all went out or the time ran out.
 *
 * @param pLink The link.
 * @param pData The bytes.
 * @param uSize The number of bytes.
 * @param uTimeout The milliseconds allowed.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c4cb0
 * @ghidraAddress PAL: 0x002cec20
 */
int RtChannelSendBlocking(RtChannelLink *pLink,
                          const void *pData,
                          unsigned int uSize,
                          unsigned int uTimeout);

/**
 * Send what the socket takes of the send ring of a link.
 *
 * @param pLink The link.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c4da0
 * @ghidraAddress PAL: 0x002ced10
 */
int RtChannelFlushSend(RtChannelLink *pLink);

/**
 * Read what the socket has into the receive ring of a link.
 *
 * @param pLink The link.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c4f30
 * @ghidraAddress PAL: 0x002ceea0
 */
int RtChannelReceive(RtChannelLink *pLink);

/**
 * Set the state of a channel.
 *
 * @param pChannel The channel.
 * @param nState The state, below 12.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c51f8
 * @ghidraAddress PAL: 0x002cf168
 */
int RtChannelSetState(RtChannel *pChannel, int nState);

/**
 * Report whether a record type carries data for the game.
 *
 * @param nType The type.
 * @return Non-zero for the types 2 to 7, 9, and 10.
 * @ghidraAddress NTSC-U/C: 0x002c5220
 * @ghidraAddress PAL: 0x002cf190
 */
int RtChannelIsDataRecord(int nType);

/**
 * Map the result of a socket routine.
 *
 * @param nResult The result.
 * @return 0, or RT_CHANNEL_RESULT_SOCKET.
 * @ghidraAddress NTSC-U/C: 0x002c5288
 * @ghidraAddress PAL: 0x002cf1f8
 */
int RtChannelMapSocketResult(int nResult);

/**
 * Make the scratch buffer at least a size.
 *
 * @param uSize The size.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c53e0
 * @ghidraAddress PAL: 0x002cf350
 */
int RtChannelReserveScratch(unsigned int uSize);

/**
 * Frame the batched records of a link as one packet in its send ring, encrypting them when the
 * channel is encrypted.
 *
 * @param pLink The link.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c5470
 * @ghidraAddress PAL: 0x002cf3e0
 */
int RtChannelFlushPacket(RtChannelLink *pLink);

/**
 * Report whether the packets of a channel are encrypted.
 *
 * @param uChannel The channel.
 * @param pbEncrypted Receives 1 when they are.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c5918
 * @ghidraAddress PAL: 0x002cf888
 */
int RtChannelIsEncrypted(unsigned short uChannel, unsigned char *pbEncrypted);

#ifdef __cplusplus
}
#endif

#endif
