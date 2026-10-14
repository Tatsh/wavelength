#ifndef RTCHANNELAPI_H
#define RTCHANNELAPI_H

#include "rt/rtchannel.h"
#include "rt/rtsecurity.h"
#include "rt/rtstream.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The channel layer of the SCE-RT library over the channel records. Each entry point takes the lock
 * of the layer and reports 0 for success, an RT_CHANNEL_RESULT_ or RT_CHANNEL_API_RESULT_ code, or
 * RT_CHANNEL_API_RESULT_LOCK when the layer is not started or its lock failed.
 */

/** The results of the layer beside the RT_CHANNEL_RESULT_ codes. */
#define RT_CHANNEL_API_RESULT_NOT_SENT 14   /*!< The stream took none of the bytes. */
#define RT_CHANNEL_API_RESULT_NO_SERVICE 17 /*!< The datagram service is not started. */
#define RT_CHANNEL_API_RESULT_PARTIAL 19    /*!< The stream took part of the bytes. */
#define RT_CHANNEL_API_RESULT_STREAM 22     /*!< The datagram stream failed. */
#define RT_CHANNEL_API_RESULT_SILENT 25     /*!< The server sent nothing for 30 seconds. */
#define RT_CHANNEL_API_RESULT_LOCK 26       /*!< The layer is not started, or its lock failed. */

/** The states RtChannelGetSecurityState() reports. */
#define RT_CHANNEL_SECURITY_KEYS 1    /*!< A key pair is stored, so packets may be encrypted. */
#define RT_CHANNEL_SECURITY_NO_KEYS 2 /*!< No key pair is stored. */

/**
 * Receiver of a datagram of the datagram service.
 *
 * @param pszAddress The address of the sender.
 * @param uPort The port of the sender.
 * @param pData The datagram.
 * @param uSize The size of the datagram.
 */
typedef void (*RtChannelDatagramFunc)(const char *pszAddress,
                                      unsigned short uPort,
                                      const void *pData,
                                      unsigned short uSize);

/**
 * Receiver of a peer the listening socket accepted.
 *
 * @param pszAddress The address of the peer.
 * @param uPort The port of the peer.
 */
typedef void (*RtChannelAcceptFunc)(const char *pszAddress, unsigned short uPort);

/**
 * Receiver of the result of a port mapping on the gateway.
 *
 * @param pszAddress The address the gateway shows, or empty.
 * @param uPort The port the gateway shows, or 0.
 * @param nResult 0, or RT_CHANNEL_API_RESULT_STREAM when the mapping failed.
 */
typedef void (*RtChannelMapFunc)(const char *pszAddress, unsigned short uPort, int nResult);

/** The parameters of the datagram service. */
typedef struct {
    unsigned short uPort;              /*!< The port the service binds. */
    RtChannelDatagramFunc pfnDatagram; /*!< Receives the datagrams, or null. */
    unsigned int uSendSize;            /*!< The size of the send queue, at least 584. */
    unsigned int uReceiveSize;         /*!< The size of the receive queue, at least 584. */
    unsigned int uFlushInterval;       /*!< The milliseconds a datagram may wait. */
    unsigned int uPeerTimeout;         /*!< The milliseconds a silent peer is kept. */
    RtStreamTimeoutFunc pfnTimeout;    /*!< Receives the peers that timed out, or null. */
} RtChannelServiceConfig;

/**
 * Report whether a key pair is stored, checking once and remembering the answer.
 *
 * @return RT_CHANNEL_SECURITY_KEYS or RT_CHANNEL_SECURITY_NO_KEYS.
 * @ghidraAddress NTSC-U/C: 0x002c02e8
 * @ghidraAddress PAL: 0x002ca258
 */
int RtChannelGetSecurityState(void);

/**
 * Start the layer once, counting the starts, and store the key pair of the console.
 *
 * @param pKeyPair The key pair, or null.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c03e0
 * @ghidraAddress PAL: 0x002ca350
 */
int RtChannelApiStartup(const RtKeyPair *pKeyPair);

/**
 * Undo one start of the layer. The last closes every channel and the service.
 *
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c04b0
 * @ghidraAddress PAL: 0x002ca420
 */
int RtChannelApiShutdown(void);

/**
 * Close the listening socket.
 *
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0560
 * @ghidraAddress PAL: 0x002ca4d0
 */
int RtChannelApiCloseListener(void);

/**
 * Start the datagram service, or check that the one started has the same parameters.
 *
 * @param pConfig The parameters.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c05d8
 * @ghidraAddress PAL: 0x002ca548
 */
int RtChannelApiStartService(const RtChannelServiceConfig *pConfig);

/**
 * Open a channel to a server and send its first record.
 *
 * @param puId Receives the channel.
 * @param pConfig The parameters.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0670
 * @ghidraAddress PAL: 0x002ca5e0
 */
int RtChannelApiOpen(unsigned short *puId, const RtChannelConfig *pConfig);

/**
 * Send what a connected channel still has, disconnect it, and destroy it.
 *
 * @param uId The channel.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0710
 * @ghidraAddress PAL: 0x002ca680
 */
int RtChannelApiClose(unsigned short uId);

/**
 * Send and receive on a channel.
 *
 * @param uId The channel.
 * @param bProcess 1 to handle the records received.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c07a0
 * @ghidraAddress PAL: 0x002ca710
 */
int RtChannelApiPumpOne(unsigned short uId, unsigned char bProcess);

/**
 * Advance the datagram service and hand its datagrams to the receiver.
 *
 * @param bDeliver 1 to deliver the datagrams.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0840
 * @ghidraAddress PAL: 0x002ca7b0
 */
int RtChannelApiReceiveDatagrams(unsigned char bDeliver);

/**
 * Hand the peers the listening socket accepts to the receiver.
 *
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c08d0
 * @ghidraAddress PAL: 0x002ca840
 */
int RtChannelApiAcceptPeers(void);

/**
 * Send and receive on every channel, the datagram service, and the listening socket.
 *
 * @param bProcess 1 to handle what was received.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0948
 * @ghidraAddress PAL: 0x002ca8b8
 */
int RtChannelApiPumpAll(unsigned char bProcess);

/**
 * Queue data for a peer on a connected channel.
 *
 * @param uId The channel.
 * @param uPeer The peer, RT_CHANNEL_ALL_PEERS, or RT_CHANNEL_MIXED_PEERS for the server.
 * @param nFlags RT_DME_SEND_ bits.
 * @param pData The data.
 * @param uSize The size of the data, at most 512.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c09d8
 * @ghidraAddress PAL: 0x002ca948
 */
int RtChannelApiSend(
    unsigned short uId, unsigned short uPeer, int nFlags, const void *pData, unsigned short uSize);

/**
 * Send a datagram through the datagram service.
 *
 * @param pszAddress The address.
 * @param uPort The port.
 * @param nFlags RT_DME_SEND_ bits.
 * @param pData The data.
 * @param uSize The size of the data, at most 512.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0ab0
 * @ghidraAddress PAL: 0x002caa20
 */
int RtChannelApiSendDatagram(const char *pszAddress,
                             unsigned short uPort,
                             int nFlags,
                             const void *pData,
                             unsigned short uSize);

/**
 * Check the listening socket for a peer at an address.
 *
 * @param pbReady Receives non-zero when the peer is there.
 * @param pszAddress The address.
 * @param uPort The port.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0b80
 * @ghidraAddress PAL: 0x002caaf0
 */
int RtChannelApiPollListener(unsigned char *pbReady, const char *pszAddress, unsigned short uPort);

/**
 * Write the address of this console as text.
 *
 * @param pszAddress Receives the address.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0c38
 * @ghidraAddress PAL: 0x002caba8
 */
int RtChannelApiGetLocalAddress(char *pszAddress);

/**
 * Copy the name the server of a connected channel sent.
 *
 * @param uId The channel.
 * @param pszName Receives the name.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0cd0
 * @ghidraAddress PAL: 0x002cac40
 */
int RtChannelApiGetServerName(unsigned short uId, char *pszName);

/**
 * Queue a flag record to the server of a channel.
 *
 * @param uId The channel.
 * @param bFlag The flag.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0d70
 * @ghidraAddress PAL: 0x002cace0
 */
int RtChannelApiSendFlag(unsigned short uId, unsigned char bFlag);

/**
 * Fill the parameters of a channel with their defaults.
 *
 * @param pConfig The parameters.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0e18
 * @ghidraAddress PAL: 0x002cad88
 */
int RtChannelApiDefaultConfig(RtChannelConfig *pConfig);

/**
 * Fill the parameters of the datagram service with their defaults.
 *
 * @param pConfig The parameters.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0ea8
 * @ghidraAddress PAL: 0x002cae18
 */
int RtChannelApiDefaultServiceConfig(RtChannelServiceConfig *pConfig);

/**
 * Drop a peer of the datagram service.
 *
 * @param pszAddress The address of the peer.
 * @param uPort The port of the peer.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0f40
 * @ghidraAddress PAL: 0x002caeb0
 */
int RtChannelApiDropPeer(const char *pszAddress, unsigned short uPort);

/**
 * Map the port of the datagram service on a gateway.
 *
 * @param pszGateway The address of the gateway.
 * @param uPort The port.
 * @param pfnDone Receives the result.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002c0fe0
 * @ghidraAddress PAL: 0x002caf50
 */
int RtChannelApiMapPort(const char *pszGateway, unsigned short uPort, RtChannelMapFunc pfnDone);

#ifdef __cplusplus
}
#endif

#endif
