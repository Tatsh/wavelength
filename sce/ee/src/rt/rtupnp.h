#ifndef RTUPNP_H
#define RTUPNP_H

#include "rt/rtclock.h"
#include "rt/rtsocket.h"
#include "rt/rtupnpxml.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Port mapping on a UPnP gateway for the SCE-RT library, followed by a query of the address the
 * gateway shows the internet. One request runs at a time, and RtUpnpPollRequest() advances it. The
 * routines report 0 for success or one of the RT_UPNP_RESULT_ codes.
 */

/** The results of the routines. */
#define RT_UPNP_RESULT_INVALID 2        /*!< An argument is null or unknown. */
#define RT_UPNP_RESULT_NO_MEMORY 3      /*!< An allocation failed. */
#define RT_UPNP_RESULT_PARSE_FAILED 4   /*!< The reply could not be parsed. */
#define RT_UPNP_RESULT_NO_SOCKET 5      /*!< A socket could not be created or read. */
#define RT_UPNP_RESULT_TIMEOUT 6        /*!< The gateway did not reply in time. */
#define RT_UPNP_RESULT_BUSY 7           /*!< Another request is running. */
#define RT_UPNP_RESULT_SEND_FAILED 8    /*!< A request could not be sent whole. */
#define RT_UPNP_RESULT_RECEIVE_FAILED 9 /*!< A reply could not be received. */
#define RT_UPNP_RESULT_WRONG_SERVICE 10 /*!< The gateway replied for another service. */
#define RT_UPNP_RESULT_HTTP_ERROR 11    /*!< The reply had no status 200 or no header end. */
#define RT_UPNP_RESULT_NO_SERVICE 12    /*!< The gateway has no such service. */
#define RT_UPNP_RESULT_UNKNOWN 13       /*!< The service or the reply was not recognized. */

/** The size of a field of the header of an HTTP reply. */
#define RT_UPNP_HEADER_SIZE 512

/** The header of an HTTP reply. */
typedef struct {
    int nStatus;                                 /*!< The status code. */
    int nMaxAge;                                 /*!< The max-age of Cache-Control. */
    int nContentLength;                          /*!< The Content-length. */
    char szDate[RT_UPNP_HEADER_SIZE];            /*!< The Date. */
    char szUsn[RT_UPNP_HEADER_SIZE];             /*!< The USN. */
    char szExt[RT_UPNP_HEADER_SIZE];             /*!< The EXT. */
    char szLocation[RT_UPNP_HEADER_SIZE];        /*!< The Location. */
    char szServer[RT_UPNP_HEADER_SIZE];          /*!< The Server. */
    char szSt[RT_UPNP_HEADER_SIZE];              /*!< The ST. */
    char szContentType[RT_UPNP_HEADER_SIZE];     /*!< The Content-type. */
    char szContentLanguage[RT_UPNP_HEADER_SIZE]; /*!< The Content-language. */
} RtUpnpHttpHeader;

struct RtUpnpContext;

/**
 * Receiver of the end of a step.
 *
 * @param pContext The context.
 * @param nResult 0 for success, otherwise an RT_UPNP_RESULT_ code.
 * @param nService The service of the step.
 */
typedef void (*RtUpnpStepCallback)(struct RtUpnpContext *pContext, int nResult, int nService);

/**
 * Receiver of the connection status.
 *
 * @param pContext The context.
 * @param nResult 0 for success, otherwise an RT_UPNP_RESULT_ code.
 * @param nService The service of the step.
 * @param nStatus The index of the status, from Unconfigured to Disconnected.
 */
typedef void (*RtUpnpStatusCallback)(struct RtUpnpContext *pContext,
                                     int nResult,
                                     int nService,
                                     int nStatus);

/**
 * Receiver of the external address.
 *
 * @param pContext The context.
 * @param nResult 0 for success, otherwise an RT_UPNP_RESULT_ code.
 * @param nService The service of the step.
 * @param pszAddress The address.
 */
typedef void (*RtUpnpAddressCallback)(struct RtUpnpContext *pContext,
                                      int nResult,
                                      int nService,
                                      const char *pszAddress);

/**
 * Receiver of the external port the scan of the mappings chose.
 *
 * @param pContext The context.
 * @param nResult 0 for success, otherwise an RT_UPNP_RESULT_ code.
 * @param nService The service of the step.
 * @param uPort The port.
 */
typedef void (*RtUpnpPortCallback)(struct RtUpnpContext *pContext,
                                   int nResult,
                                   int nService,
                                   unsigned short uPort);

/**
 * Receiver of the address and the port the gateway reported for this console.
 *
 * @param pContext The context.
 * @param nResult 0 for success, otherwise an RT_UPNP_RESULT_ code.
 * @param pszAddress The address.
 * @param uPort The port.
 */
typedef void (*RtUpnpProbeCallback)(struct RtUpnpContext *pContext,
                                    int nResult,
                                    const char *pszAddress,
                                    unsigned short uPort);

/**
 * Receiver of the end of a port mapping request.
 *
 * @param pContext The context.
 * @param nResult 0 for success, otherwise an RT_UPNP_RESULT_ code.
 * @param nService The service of the mapping.
 * @param pszAddress The external address.
 * @param uPort The external port.
 * @param pUser The value given to RtUpnpCreateContext().
 */
typedef void (*RtUpnpMappingCallback)(struct RtUpnpContext *pContext,
                                      int nResult,
                                      int nService,
                                      const char *pszAddress,
                                      unsigned short uPort,
                                      void *pUser);

/** The state of the request in progress. */
typedef struct {
    int nSocket;                           /*!< The socket of the request. */
    RtUpnpXmlParser *pParser;              /*!< The parser of the reply. */
    int nService;                          /*!< The service the request is for. */
    int nStep;                             /*!< The next step of a port mapping request. */
    RtTime startTime;                      /*!< When the request was sent. */
    RtUpnpHttpHeader discoveryHeader;      /*!< The header of the discovery reply. */
    RtUpnpHttpHeader header;               /*!< The header of the reply. */
    unsigned char bAction;                 /*!< Non-zero when the request is a SOAP action. */
    unsigned char bHeaderRead;             /*!< Non-zero once the header of the reply is read. */
    unsigned char bMappingFound;           /*!< Non-zero once the scan found the mapping. */
    unsigned char bUsePpp;                 /*!< Non-zero to use WANPPPConnection. */
    unsigned char bRetried;                /*!< Non-zero once the mapping was retried. */
    int nBodyRead;                         /*!< The bytes of the body read. */
    int nMappingCount;                     /*!< The mappings the gateway has. */
    int nMappingIndex;                     /*!< The next mapping to read. */
    RtUpnpStepCallback pfnDiscovered;      /*!< Receives the end of the discovery. */
    RtUpnpStepCallback pfnDescribed;       /*!< Receives the end of the description. */
    RtUpnpStepCallback pfnScpdRead;        /*!< Receives the end of the service description. */
    RtUpnpStatusCallback pfnStatusRead;    /*!< Receives the connection status. */
    RtUpnpAddressCallback pfnAddressRead;  /*!< Receives the external address. */
    RtUpnpPortCallback pfnMappingsScanned; /*!< Receives the external port the scan chose. */
    RtUpnpStepCallback pfnMappingAdded;    /*!< Receives the end of AddPortMapping. */
    RtUpnpStepCallback pfnMappingVerified; /*!< Receives the end of the check of a mapping. */
    RtUpnpStepCallback pfnMappingRemoved;  /*!< Receives the end of the removal of a mapping. */
    RtUpnpProbeCallback pfnProbed;         /*!< Receives the reply of the gateway to the probe. */
    RtUpnpMappingCallback pfnDone;         /*!< Receives the end of the port mapping request. */
} RtUpnpRequest;

/** The probe of the gateway for the address and the port it shows the internet. */
typedef struct {
    char szGateway[RT_SOCKET_ADDRESS_SIZE]; /*!< The address of the gateway. */
    unsigned short uPort;                   /*!< The port of the gateway. */
    int nResult;                            /*!< The result of the probe. */
    RtTime time;                            /*!< When the probe or the last keep-alive was sent. */
    int nAttempts;                          /*!< The probes sent. */
    unsigned char bKeepAlive;               /*!< Non-zero once keep-alives are due. */
} RtUpnpProbe;

/** A port mapping client. */
typedef struct RtUpnpContext {
    int nSocket;                                    /*!< The datagram socket to map. */
    char szLocalAddress[RT_SOCKET_ADDRESS_SIZE];    /*!< The address of this console. */
    char szExternalAddress[RT_SOCKET_ADDRESS_SIZE]; /*!< The address the gateway shows. */
    unsigned short uPort;                           /*!< The port of the socket. */
    unsigned short uExternalPort;                   /*!< The port the gateway shows. */
    RtUpnpProbe probe;                              /*!< The probe of the gateway. */
    int nRequest;                                   /*!< The request in progress, or 0. */
    unsigned char bMapped;                          /*!< Non-zero once a mapping was added. */
    RtUpnpRequest *pRequest;                        /*!< The state of the request, or null. */
    void *pUser;                                    /*!< The value for RtUpnpMappingCallback. */
} RtUpnpContext;

/**
 * Create a port mapping client for a socket.
 *
 * @param ppContext Receives the client.
 * @param nSocket The datagram socket to map.
 * @param uPort The port of the socket.
 * @param pUser The value for the RtUpnpMappingCallback.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ccb08
 * @ghidraAddress PAL: 0x002d6a78
 */
int RtUpnpCreateContext(RtUpnpContext **ppContext, int nSocket, unsigned short uPort, void *pUser);

/**
 * Destroy a port mapping client.
 *
 * @param pContext The client.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002ccbf8
 * @ghidraAddress PAL: 0x002d6b68
 */
int RtUpnpDestroyContext(RtUpnpContext *pContext);

/**
 * Start a port mapping request: discover the gateway, read its description and its service, map
 * the port, and probe the gateway for the address it shows.
 *
 * @param pContext The client.
 * @param pszGateway The address of the gateway to probe.
 * @param uGatewayPort The port of the gateway to probe.
 * @param pfnDone Receives the end of the request.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002cd6a0
 * @ghidraAddress PAL: 0x002d7610
 */
int RtUpnpBeginMapping(RtUpnpContext *pContext,
                       const char *pszGateway,
                       unsigned short uGatewayPort,
                       RtUpnpMappingCallback pfnDone);

/**
 * Advance the request in progress, or send the keep-alive of the probe when none runs.
 *
 * @param pContext The client.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002cd7c8
 * @ghidraAddress PAL: 0x002d7738
 */
int RtUpnpPollRequest(RtUpnpContext *pContext);

#ifdef __cplusplus
}
#endif

#endif
