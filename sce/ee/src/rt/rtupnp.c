#include "rt/rtupnp.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rt/rtmemory.h"
#include "rt/rttime.h"

// The library copies its text with strncpy() and bounds as large as the destinations, as retail
// does.
#pragma GCC diagnostic ignored "-Wstringop-truncation"

enum {
    kHttpOk = 200,
    kHttpDefaultPort = 80,
    kHttpVersionLength = 7,
    kHttpStatusOffset = 9,
    kHttpNumberBase = 10,
    kHttpUrlSize = 2048,
    kHttpBufferSize = 2048,
    kHttpSchemeLength = 4,
    // The slashes before the host, and before the path, of a URL.
    kUrlHostSlash = 2,
    kUrlPathSlash = 3,
    kUrlSlashCount = 3,
    // The bytes of the soap requests besides their variable parts.
    kQueryFixedLength = 265,
    kActionFixedLength = 203,
    kSsdpPort = 1900,
    kSocketBufferSize = 2048,
    kDiscoveryTimeout = 5000,
    kBodyTimeout = 30000,
    kProbeInterval = 800,
    kProbeAttempts = 5,
    kKeepAliveInterval = 5000,
    kProbeRequestSize = 4,
    kProbeReplySize = 6,
    kProbeOpcodeIndex = 2,
    kKeepAliveOpcodeIndex = 3,
    kMappingScanDefault = 32,
    kMappingIndexSize = 512,
    // The receive error of a stream the peer closed.
    kSocketClosed = 13,
};

// The requests a client runs, in RtUpnpContext::nRequest.
enum {
    kRequestNone,
    kRequestDiscovery,
    kRequestDescription,
    kRequestScpd,
    kRequestStatus,
    kRequestAddress,
    kRequestMappingScan,
    kRequestAddMapping,
    kRequestVerifyMapping,
    kRequestRemoveMapping,
    kRequestProbe,
    kRequestCount,
};

// The steps of a port mapping request, in RtUpnpRequest::nStep.
enum {
    kStepNone,
    kStepWaiting,
    kStepDescription,
    kStepScpd,
    kStepStatus,
    kStepAddress,
    kStepMappingScan,
    kStepAddMapping,
    kStepProbe,
    kStepCount,
};

// The index of the status Connected in the status names.
enum {
    kStatusConnected = 2,
    kStatusCount = 6,
};

// The fields of the header of an HTTP reply the parser reads.
enum {
    kHeaderCacheControl,
    kHeaderUsn,
    kHeaderLocation,
    kHeaderServer,
    kHeaderSt,
    kHeaderExt,
    kHeaderDate,
    kHeaderContentLength,
    kHeaderContentLanguage,
    kHeaderContentType,
    kHeaderCount,
};

// The opcode of the probe and of its keep-alive.
#define PROBE_OPCODE 0xe4
#define KEEP_ALIVE_OPCODE 0xd4

#define SSDP_ADDRESS "239.255.255.250"

// NTSC-U/C: 0x003b6d24
static unsigned char g_bDiscoveryDone;
// NTSC-U/C: 0x003b6d25
static unsigned char g_bDescriptionDone;
// NTSC-U/C: 0x003b6d26
static unsigned char g_bIpScpdDone;
// NTSC-U/C: 0x003b6d27
static unsigned char g_bPppScpdDone;
// NTSC-U/C: 0x003b6d28
static int g_nDiscoveryResult;
// NTSC-U/C: 0x003b6d2c
static int g_nDescriptionResult;
// NTSC-U/C: 0x003b6d30
static int g_nIpScpdResult;
// NTSC-U/C: 0x003b6d34
static int g_nPppScpdResult;
// NTSC-U/C: 0x00521a00
static RtUpnpDeviceInfo g_deviceInfo;
// NTSC-U/C: 0x00522d08
static unsigned int g_uIpActions;
// NTSC-U/C: 0x00522d10
static unsigned int g_uPppActions;

// The fields of the header of an HTTP reply the parser reads, in the order of the enumeration.
static const char *const g_apszHeaderFields[] = {
    "Cache-Control",
    "USN",
    "Location",
    "Server",
    "ST",
    "EXT",
    "Date",
    "Content-length",
    "Content-language",
    "Content-type",
    NULL,
};

// The values of ConnectionStatus.
static const char *const g_apszConnectionStatuses[] = {
    "Unconfigured",
    "Connecting",
    "Connected",
    "PendingDisconnect",
    "Disconnecting",
    "Disconnected",
    NULL,
};

// The reply of the gateway to the probe.
typedef struct {
    unsigned char abAddress[4]; // The address, most significant byte first.
    unsigned short uPort;       // The port, most significant byte first.
} RtUpnpProbeReply;

static int RtUpnpAdvanceStep(RtUpnpContext *pContext);
static int RtUpnpAllocRequest(RtUpnpContext *pContext);
static int RtUpnpFreeRequest(RtUpnpContext *pContext);

// Release the parser and the socket of a request and forget its header.
static inline void RtUpnpCloseRequest(RtUpnpRequest *pRequest) {
    RtXmlParserDestroy(pRequest->pParser);
    pRequest->pParser = NULL;
    RtSocketClose(pRequest->pSocket);
    pRequest->pSocket = NULL;
    memset(&pRequest->header, 0, sizeof(RtUpnpHttpHeader));
}

// NTSC-U/C: 0x002ca1c8, PAL: 0x002d4138
static char *RtUpnpSplitToken(char **ppszCursor, const char *pszDelimiters) {
    char *pszToken;
    char *pszChar;
    const char *pszDelimiter;

    pszToken = *ppszCursor;
    if (pszToken == NULL) {
        return NULL;
    }
    if (pszDelimiters == NULL) {
        return pszToken;
    }
    if (*pszToken == '\0') {
        *ppszCursor = NULL;
        return pszToken;
    }
    pszChar = pszToken;
    do {
        for (pszDelimiter = pszDelimiters; *pszDelimiter != '\0'; ++pszDelimiter) {
            if (*pszChar == *pszDelimiter) {
                *pszChar = '\0';
                ++*ppszCursor;
                return pszToken;
            }
        }
        *ppszCursor = ++pszChar;
    } while (*pszChar != '\0');
    *ppszCursor = NULL;
    return pszToken;
}

// NTSC-U/C: 0x002ca270, PAL: 0x002d41e0
static int RtUpnpCompareNoCase(const char *pszLeft, const char *pszRight, int nSize) {
    int nDifference;
    int nLeft;
    int nRight;
    int i;

    nDifference = 0;
    for (i = 0; i < nSize;) {
        nRight = pszRight[i];
        nLeft = pszLeft[i];
        ++i;
        // Yes, the binary sums the differences instead of stopping at the first.
        nDifference += (isupper(nLeft) ? nLeft + ('a' - 'A') : nLeft) -
                       (isupper(nRight) ? nRight + ('a' - 'A') : nRight);
        if (nLeft == '\0' || nRight == '\0') {
            break;
        }
    }
    return nDifference;
}

// NTSC-U/C: 0x002ca300, PAL: 0x002d4270
static int RtUpnpParseUrl(char *pszUrl, char *pszPath, char *pszHost, unsigned short *puPort) {
    char *pszCursor;
    char *pszChar;
    const char *pszToken;
    unsigned int i;
    int nSlashes;

    pszCursor = NULL;
    i = 0;
    nSlashes = 0;
    if (pszUrl == NULL || pszPath == NULL || pszHost == NULL || puPort == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    while (i < strlen(pszUrl)) {
        pszChar = &pszUrl[i];
        ++i;
        if (*pszChar != '/') {
            continue;
        }
        ++nSlashes;
        if (nSlashes == kUrlHostSlash) {
            pszCursor = pszChar + 1;
        } else if (nSlashes == kUrlPathSlash) {
            strncpy(pszPath, pszChar + 1, RT_UPNP_HEADER_SIZE);
            *pszChar = '\0';
            pszToken = RtUpnpSplitToken(&pszCursor, ":");
            if (!isdigit((int)pszToken[0])) {
                return RT_UPNP_RESULT_HTTP_ERROR;
            }
            strncpy(pszHost, pszToken, RT_SOCKET_ADDRESS_SIZE);
            if (pszCursor != NULL) {
                *puPort = strtol(pszCursor, NULL, kHttpNumberBase);
            } else {
                *puPort = kHttpDefaultPort;
            }
        }
    }
    return 0;
}

// NTSC-U/C: 0x002ca470, PAL: 0x002d43e0
static int RtUpnpParseHttpHeader(char *pszReply, RtUpnpHttpHeader *pHeader) {
    const char *apszFields[kHeaderCount + 1];
    char *pszLine;
    char *pszValue;
    const char *pszName;
    int nResult;
    int nLine;
    int i;

    pszValue = NULL;
    memcpy(apszFields, g_apszHeaderFields, sizeof(apszFields));
    nResult = -1;
    nLine = 0;
    for (pszLine = strtok(pszReply, "\r\n"); pszLine != NULL; pszLine = strtok(NULL, "\r\n")) {
        if (nLine == 0) {
            if (RtUpnpCompareNoCase(pszLine, "http/1.", kHttpVersionLength) != 0) {
                return nResult;
            }
            pszValue = &pszLine[kHttpStatusOffset];
            nResult = (short)strtol(pszValue, NULL, kHttpNumberBase);
            pHeader->nStatus = nResult;
            if (nResult != kHttpOk) {
                return nResult;
            }
        } else {
            pszValue = pszLine;
            RtUpnpSplitToken(&pszValue, ":");
            while (isspace((int)*pszValue)) {
                ++pszValue;
            }
            for (i = strlen(pszLine); i > 0; --i) {
                if (isspace((int)pszLine[i])) {
                    pszLine[i] = '\0';
                }
            }
            for (i = 0; (pszName = apszFields[i]) != NULL; ++i) {
                if (RtUpnpCompareNoCase(pszName, pszLine, strlen(pszName)) == 0) {
                    break;
                }
            }
            switch (i) {
            case kHeaderCacheControl:
                if (RtUpnpCompareNoCase(
                        RtUpnpSplitToken(&pszValue, "="), "max-age", kHttpVersionLength) == 0) {
                    pHeader->nMaxAge = strtol(pszValue, NULL, kHttpNumberBase);
                }
                break;
            case kHeaderUsn:
                strncpy(pHeader->szUsn, pszValue, RT_UPNP_HEADER_SIZE);
                break;
            case kHeaderLocation:
                strncpy(pHeader->szLocation, pszValue, RT_UPNP_HEADER_SIZE);
                break;
            case kHeaderServer:
                strncpy(pHeader->szServer, pszValue, RT_UPNP_HEADER_SIZE);
                break;
            case kHeaderSt:
                strncpy(pHeader->szSt, pszValue, RT_UPNP_HEADER_SIZE);
                break;
            case kHeaderExt:
                strncpy(pHeader->szExt, pszValue, RT_UPNP_HEADER_SIZE);
                break;
            case kHeaderDate:
                strncpy(pHeader->szDate, pszValue, RT_UPNP_HEADER_SIZE);
                break;
            case kHeaderContentLength:
                pHeader->nContentLength = strtol(pszValue, NULL, kHttpNumberBase);
                break;
            case kHeaderContentLanguage:
                strncpy(pHeader->szContentLanguage, pszValue, RT_UPNP_HEADER_SIZE);
                break;
            case kHeaderContentType:
                strncpy(pHeader->szContentType, pszValue, RT_UPNP_HEADER_SIZE);
                break;
            default:
                break;
            }
        }
        ++nLine;
    }
    return nResult;
}

// NTSC-U/C: 0x002ca7a0, PAL: 0x002d4710
static int RtUpnpGetScpdUrl(const RtUpnpDeviceInfo *pInfo, int nService, char *pszUrl) {
    const char *pszScpdUrl;

    if (pInfo == NULL || pszUrl == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (nService == RT_UPNP_WAN_IP_CONNECTION) {
        pszScpdUrl = pInfo->ip.szScpdUrl;
    } else if (nService == RT_UPNP_WAN_PPP_CONNECTION) {
        pszScpdUrl = pInfo->ppp.szScpdUrl;
    } else {
        return RT_UPNP_RESULT_INVALID;
    }
    if (RtUpnpCompareNoCase(pszScpdUrl, "http", kHttpSchemeLength) == 0) {
        strncpy(pszUrl, pszScpdUrl, kHttpUrlSize);
        return 0;
    }
    strncpy(pszUrl, pInfo->szUrlBase, kHttpUrlSize);
    strncat(pszUrl, pszScpdUrl, kHttpUrlSize);
    return 0;
}

// NTSC-U/C: 0x002ca858, PAL: 0x002d47c8
static int RtUpnpGetControlUrl(const RtUpnpDeviceInfo *pInfo, int nService, char *pszUrl) {
    const char *pszControlUrl;
    unsigned int i;
    int nSlashes;

    if (nService == RT_UPNP_WAN_IP_CONNECTION) {
        pszControlUrl = pInfo->ip.szControlUrl;
    } else if (nService == RT_UPNP_WAN_PPP_CONNECTION) {
        pszControlUrl = pInfo->ppp.szControlUrl;
    } else {
        return RT_UPNP_RESULT_INVALID;
    }
    if (RtUpnpCompareNoCase(pszControlUrl, "http", kHttpSchemeLength) == 0) {
        strncpy(pszUrl, pszControlUrl, kHttpUrlSize);
        return 0;
    }
    if (RtUpnpCompareNoCase(pInfo->szUrlBase, "http", kHttpSchemeLength) == 0) {
        strncpy(pszUrl, pInfo->szUrlBase, kHttpUrlSize);
        strncat(pszUrl, pszControlUrl, kHttpUrlSize);
        return 0;
    }
    // Take the scheme and the host from the URL of the service description.
    (void)RtUpnpGetScpdUrl(pInfo, nService, pszUrl); // Yes, the binary discards the result.
    nSlashes = 0;
    for (i = 0; i < strlen(pszUrl); ++i) {
        if (pszUrl[i] == '/') {
            ++nSlashes;
            if (nSlashes == kUrlSlashCount) {
                strncpy(&pszUrl[i], pszControlUrl, kHttpUrlSize - i);
                break;
            }
        }
    }
    return 0;
}

// NTSC-U/C: 0x002ca9a8, PAL: 0x002d4918
static int RtUpnpReceiveHttpBody(RtUpnpContext *pContext, unsigned char *pbDone) {
    char acBuffer[kHttpBufferSize];
    char szHeader[RT_UPNP_HEADER_SIZE];
    RtUpnpRequest *pRequest;
    unsigned short uSize;
    unsigned int uElapsed;
    char *pszBody;
    int nResult;

    pRequest = pContext->pRequest;
    nResult = RtSocketReceive(pRequest->pSocket, acBuffer, kHttpBufferSize, &uSize);
    if (nResult != 0) {
        *pbDone = 1;
        if (nResult == kSocketClosed) {
            return 0;
        }
        return RT_UPNP_RESULT_RECEIVE_FAILED;
    }
    if (uSize == 0) {
        (void)RtGetMillisecondsSince(&pContext->pRequest->startTime, &uElapsed);
        if (uElapsed <= kBodyTimeout) {
            return 0;
        }
        return RT_UPNP_RESULT_TIMEOUT;
    }
    acBuffer[uSize] = '\0';
    pszBody = acBuffer;
    if (!pContext->pRequest->bHeaderRead) {
        pszBody = strstr(acBuffer, "\r\n\r\n");
        if (pszBody != NULL) {
            *pszBody = '\0';
        } else {
            pszBody = strstr(acBuffer, "\n\n");
            if (pszBody == NULL) {
                return RT_UPNP_RESULT_HTTP_ERROR;
            }
            *pszBody = '\0';
        }
        pszBody += 4; // Yes, the binary skips 4 bytes after a bare "\n\n" too.
        uSize -= pszBody - acBuffer;
        strncpy(szHeader, acBuffer, RT_UPNP_HEADER_SIZE);
        RtUpnpParseHttpHeader(szHeader, &pContext->pRequest->header);
        if (pContext->pRequest->header.nStatus != kHttpOk) {
            return RT_UPNP_RESULT_HTTP_ERROR;
        }
        pContext->pRequest->bHeaderRead = 1;
    }
    if (RtXmlParserFeed(pContext->pRequest->pParser, pszBody, uSize, 0) != 0) {
        return RT_UPNP_RESULT_PARSE_FAILED;
    }
    pRequest = pContext->pRequest;
    pRequest->nBodyRead += uSize;
    if (pRequest->nBodyRead == pRequest->header.nContentLength && pRequest->nBodyRead > 0) {
        *pbDone = 1;
        return 0;
    }
    if (RtXmlParserTakeComplete(pRequest->pParser, pbDone) != 0) {
        return RT_UPNP_RESULT_PARSE_FAILED;
    }
    return 0;
}

// NTSC-U/C: 0x002cab60, PAL: 0x002d4ad0
static int RtUpnpReceiveDiscoveryReply(RtUpnpContext *pContext, unsigned char *pbDone) {
    char szSender[RT_SOCKET_ADDRESS_SIZE];
    char acBuffer[kHttpBufferSize];
    unsigned short uPort;
    // Yes, the binary reads the size the socket wrote as signed.
    union {
        unsigned short uSize;
        short nSize;
    } received;
    unsigned int uElapsed;
    const char *pszServiceType;

    uPort = 0;
    received.nSize = 0;
    if (RtSocketReceiveFrom(pContext->pRequest->pSocket,
                            szSender,
                            &uPort,
                            acBuffer,
                            kHttpBufferSize,
                            &received.uSize) != 0) {
        *pbDone = 1;
        return RT_UPNP_RESULT_NO_SOCKET;
    }
    if (received.nSize <= 0) {
        (void)RtGetMillisecondsSince(&pContext->pRequest->startTime, &uElapsed);
        if (uElapsed <= kDiscoveryTimeout) {
            return 0;
        }
        *pbDone = 1;
        return RT_UPNP_RESULT_TIMEOUT;
    }
    *pbDone = 1;
    acBuffer[received.nSize] = '\0';
    received.nSize = RtUpnpParseHttpHeader(acBuffer, &pContext->pRequest->discoveryHeader);
    if (received.nSize != kHttpOk) {
        return RT_UPNP_RESULT_HTTP_ERROR;
    }
    switch (pContext->pRequest->nService) {
    case RT_UPNP_LAYER3_FORWARDING:
        pszServiceType = "urn:schemas-upnp-org:service:Layer3Forwarding:1";
        break;
    case RT_UPNP_WAN_COMMON_CONFIG:
        pszServiceType = "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1";
        break;
    case RT_UPNP_WAN_IP_CONNECTION:
        pszServiceType = "urn:schemas-upnp-org:service:WANIPConnection:1";
        break;
    case RT_UPNP_WAN_PPP_CONNECTION:
        pszServiceType = "urn:schemas-upnp-org:service:WANPPPConnection:1";
        break;
    default:
        return RT_UPNP_RESULT_UNKNOWN;
    }
    if (strncmp(pszServiceType, pContext->pRequest->discoveryHeader.szSt, RT_UPNP_HEADER_SIZE) !=
        0) {
        return RT_UPNP_RESULT_WRONG_SERVICE;
    }
    return 0;
}

// Connect the stream of a request to the host of a URL, or release its parser.
static inline int
RtUpnpConnect(RtUpnpRequest *pRequest, const char *pszHost, unsigned short uPort) {
    RtSocketOptions options;

    memset(&options, 0, sizeof(options));
    options.nBufferSize = kSocketBufferSize;
    options.nType = RT_SOCKET_STREAM;
    if (RtSocketCreate(&pRequest->pSocket, pszHost, uPort, &options) != 0) {
        RtXmlParserDestroy(pRequest->pParser);
        return RT_UPNP_RESULT_NO_SOCKET;
    }
    return 0;
}

// Send the request built in a buffer on the stream of a request.
static inline int RtUpnpSendMessage(RtUpnpRequest *pRequest, const char *pszMessage) {
    // Yes, the binary reads the count the socket wrote as signed.
    union {
        unsigned short uSent;
        short nSent;
    } sent;

    if (RtSocketSend(pRequest->pSocket, pszMessage, strlen(pszMessage), &sent.uSent) == 0 &&
        strlen(pszMessage) == (unsigned int)sent.nSent) {
        return 0;
    }
    RtSocketClose(pRequest->pSocket);
    RtXmlParserDestroy(pRequest->pParser);
    return RT_UPNP_RESULT_SEND_FAILED;
}

// NTSC-U/C: 0x002cacf8, PAL: 0x002d4c68
static int RtUpnpSendQuery(RtUpnpContext *pContext, int nService, const char *pszVariable) {
    char szHost[RT_SOCKET_ADDRESS_SIZE];
    char szPath[RT_UPNP_HEADER_SIZE];
    char szMessage[kHttpUrlSize];
    unsigned short uPort;
    int nResult;

    pContext->pRequest->bAction = 0;
    pContext->pRequest->bHeaderRead = 0;
    pContext->pRequest->nBodyRead = 0;
    nResult = RtUpnpGetControlUrl(&g_deviceInfo, nService, szMessage);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtUpnpParseUrl(szMessage, szPath, szHost, &uPort);
    if (nResult != 0) {
        return nResult;
    }
    if ((nService & g_deviceInfo.uFlags) == 0) {
        return RT_UPNP_RESULT_NO_SERVICE;
    }
    if (nService != RT_UPNP_WAN_IP_CONNECTION && (nService & RT_UPNP_WAN_PPP_CONNECTION) == 0) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (RtXmlParserCreate(&pContext->pRequest->pParser, RT_UPNP_XML_QUERY) != 0 ||
        pContext->pRequest->pParser == NULL) {
        return RT_UPNP_RESULT_PARSE_FAILED;
    }
    nResult = RtUpnpConnect(pContext->pRequest, szHost, uPort);
    if (nResult != 0) {
        return nResult;
    }
    sprintf(szMessage,
            "M-POST /%s HTTP/1.1\r\nHost:%s:%d\r\nContent-length:%d\r\nConnection:close\r\n"
            "Content-type:text/xml;charset=\"utf-8\"\r\n"
            "Soapaction:\"urn:schemas-upnp-org:control-1-0#QueryStateVariable\"\r\n\r\n"
            "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope\" "
            "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/envelope\"><s:Body>"
            "<u:QueryStateVariable xmlns:u=\"urn:schemas-upnp-org:control-1-0\">"
            "<u:varName>%s</u:varName></u:QueryStateVariable></s:Body></s:Envelope>",
            szPath,
            szHost,
            uPort,
            strlen(pszVariable) + kQueryFixedLength,
            pszVariable);
    return RtUpnpSendMessage(pContext->pRequest, szMessage);
}

// NTSC-U/C: 0x002caec0, PAL: 0x002d4e30
static int
RtUpnpSendAction(RtUpnpContext *pContext, const char *pszAction, const char *pszArguments) {
    char szHost[RT_SOCKET_ADDRESS_SIZE];
    char szPath[RT_UPNP_HEADER_SIZE];
    char szService[RT_UPNP_HEADER_SIZE];
    char szMessage[kHttpUrlSize];
    unsigned short uPort;
    int nService;
    int nResult;

    pContext->pRequest->bAction = 1;
    pContext->pRequest->bHeaderRead = 0;
    pContext->pRequest->nBodyRead = 0;
    nResult = RtUpnpGetControlUrl(&g_deviceInfo, pContext->pRequest->nService, szMessage);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtUpnpParseUrl(szMessage, szPath, szHost, &uPort);
    if (nResult != 0) {
        return nResult;
    }
    nService = pContext->pRequest->nService;
    if ((nService & g_deviceInfo.uFlags) == 0) {
        return RT_UPNP_RESULT_NO_SERVICE;
    }
    if (nService != RT_UPNP_WAN_IP_CONNECTION && (nService & RT_UPNP_WAN_PPP_CONNECTION) == 0) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (RtXmlParserCreate(&pContext->pRequest->pParser, RT_UPNP_XML_ACTION) != 0 ||
        pContext->pRequest->pParser == NULL) {
        return RT_UPNP_RESULT_PARSE_FAILED;
    }
    if (pContext->pRequest->nService == RT_UPNP_WAN_IP_CONNECTION) {
        strncpy(szService, "WANIPConnection", RT_UPNP_HEADER_SIZE);
    } else {
        strncpy(szService, "WANPPPConnection", RT_UPNP_HEADER_SIZE);
    }
    nResult = RtUpnpConnect(pContext->pRequest, szHost, uPort);
    if (nResult != 0) {
        return nResult;
    }
    sprintf(szMessage,
            "M-POST /%s HTTP/1.1\r\nHost:%s:%d\r\nContent-length:%d\r\nConnection:close\r\n"
            "Content-type:text/xml;charset=\"utf-8\"\r\n"
            "Soapaction:\"urn:schemas-upnp-org:service:%s:1#%s\"\r\n\r\n"
            "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope\" "
            "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/envelope\"><s:Body>"
            "<u:%s xmlns:u=\"urn:schemas-upnp-org:%s:1\">%s</u:%s></s:Body></s:Envelope>",
            szPath,
            szHost,
            uPort,
            strlen(pszArguments) + kActionFixedLength + strlen(szService) + strlen(pszAction) * 2,
            szService,
            pszAction,
            pszAction,
            szService,
            pszArguments,
            pszAction);
    return RtUpnpSendMessage(pContext->pRequest, szMessage);
}

// NTSC-U/C: 0x002cb120, PAL: 0x002d5090
static int RtUpnpEndDescription(RtUpnpContext *pContext) {
    RtUpnpDeviceInfo *pInfo;
    RtUpnpRequest *pRequest;
    unsigned char bDone;
    int nResult;

    bDone = 0;
    nResult = RtUpnpReceiveHttpBody(pContext, &bDone);
    if (nResult == 0 && !bDone) {
        return 0;
    }
    if (pContext->pRequest->header.nStatus == kHttpOk && bDone) {
        pInfo = NULL;
        if (RtXmlParserFeed(pContext->pRequest->pParser, "", 0, 1) == 0 &&
            RtXmlParserGetResult(pContext->pRequest->pParser, (void **)&pInfo) == 0) {
            g_deviceInfo = *pInfo;
            pContext->pRequest->nService = g_deviceInfo.uFlags;
        }
    }
    g_nDescriptionResult = nResult;
    g_bDescriptionDone = 1;
    RtXmlParserDestroy(pContext->pRequest->pParser);
    pRequest = pContext->pRequest;
    pRequest->pParser = NULL;
    RtSocketClose(pRequest->pSocket);
    pRequest = pContext->pRequest;
    pRequest->pSocket = NULL;
    pRequest->pfnDescribed(pContext, nResult, pRequest->nService);
    pContext->pRequest->pfnDescribed = NULL;
    memset(&pContext->pRequest->header, 0, sizeof(RtUpnpHttpHeader));
    pContext->nRequest = kRequestNone;
    return nResult;
}

// NTSC-U/C: 0x002cb2f0, PAL: 0x002d5260
static int RtUpnpEndScpd(RtUpnpContext *pContext) {
    RtUpnpRequest *pRequest;
    unsigned int *puActions;
    unsigned char bDone;
    int nResult;

    bDone = 0;
    nResult = RtUpnpReceiveHttpBody(pContext, &bDone);
    if (nResult == 0 && !bDone) {
        return 0;
    }
    pRequest = pContext->pRequest;
    if (pRequest->header.nStatus == kHttpOk && bDone) {
        if ((pRequest->nService & RT_UPNP_WAN_IP_CONNECTION) != 0) {
            puActions = NULL;
            if (RtXmlParserFeed(pRequest->pParser, "", 0, 1) == 0 &&
                RtXmlParserGetResult(pContext->pRequest->pParser, (void **)&puActions) == 0) {
                g_bIpScpdDone = 1;
                g_uIpActions = *puActions;
                g_nIpScpdResult = nResult;
            }
        } else if ((pRequest->nService & RT_UPNP_WAN_PPP_CONNECTION) != 0) {
            puActions = NULL;
            if (RtXmlParserFeed(pRequest->pParser, "", 0, 1) == 0 &&
                RtXmlParserGetResult(pContext->pRequest->pParser, (void **)&puActions) == 0) {
                g_bPppScpdDone = 1;
                g_uPppActions = *puActions;
                g_nPppScpdResult = nResult;
            }
        }
    }
    RtXmlParserDestroy(pContext->pRequest->pParser);
    pRequest = pContext->pRequest;
    pRequest->pParser = NULL;
    RtSocketClose(pRequest->pSocket);
    pRequest = pContext->pRequest;
    pRequest->pSocket = NULL;
    pRequest->pfnScpdRead(pContext, nResult, pRequest->nService);
    pContext->pRequest->pfnScpdRead = NULL;
    memset(&pContext->pRequest->header, 0, sizeof(RtUpnpHttpHeader));
    pContext->nRequest = kRequestNone;
    return nResult;
}

// NTSC-U/C: 0x002cb488, PAL: 0x002d53f8
static int RtUpnpEndStatus(RtUpnpContext *pContext) {
    const char *apszStatuses[kStatusCount + 1];
    char szStatus[RT_UPNP_TEXT_SIZE];
    RtUpnpActionReply *pReply;
    char *pszValue;
    RtUpnpRequest *pRequest;
    unsigned char bDone;
    unsigned int i;
    int nStatus;
    int nResult;

    bDone = 0;
    memcpy(apszStatuses, g_apszConnectionStatuses, sizeof(apszStatuses));
    nStatus = 0;
    nResult = RtUpnpReceiveHttpBody(pContext, &bDone);
    if (nResult == 0 && !bDone) {
        return 0;
    }
    if (pContext->pRequest->header.nStatus == kHttpOk && bDone) {
        if (!pContext->pRequest->bAction) {
            pszValue = NULL;
            if (RtXmlParserFeed(pContext->pRequest->pParser, "", 0, 1) != 0 ||
                RtXmlParserGetResult(pContext->pRequest->pParser, (void **)&pszValue) != 0) {
                nResult = RT_UPNP_RESULT_PARSE_FAILED;
                goto close;
            }
            strncpy(szStatus, pszValue, RT_UPNP_TEXT_SIZE);
        } else {
            pReply = NULL;
            if (RtXmlParserFeed(pContext->pRequest->pParser, "", 0, 1) != 0 ||
                RtXmlParserGetResult(pContext->pRequest->pParser, (void **)&pReply) != 0) {
                nResult = RT_UPNP_RESULT_PARSE_FAILED;
                goto close;
            }
            // Yes, the binary leaves the status uninitialised when the reply lacks it.
            for (i = 0; i < pReply->uCount; ++i) {
                if (strncmp("NewConnectionStatus", pReply->aszNames[i], RT_UPNP_TEXT_SIZE) == 0) {
                    strncpy(szStatus, pReply->aszValues[i], RT_UPNP_TEXT_SIZE);
                }
            }
        }
        for (i = 0; apszStatuses[i] != NULL; ++i) {
            if (strncmp(szStatus, apszStatuses[i], RT_UPNP_TEXT_SIZE) == 0) {
                break;
            }
        }
        if (i < kStatusCount) {
            nStatus = i;
        }
    }
close:
    RtUpnpCloseRequest(pContext->pRequest);
    pRequest = pContext->pRequest;
    if (!pRequest->bAction && nResult != 0) {
        // The variable could not be queried. Ask with the action instead.
        nResult = RtUpnpSendAction(pContext, "GetStatusInfo", "");
        if (nResult == 0) {
            RtReadClock(&pContext->pRequest->startTime);
            return 0;
        }
        pRequest = pContext->pRequest;
    }
    pRequest->pfnStatusRead(pContext, nResult, pRequest->nService, nStatus);
    pContext->nRequest = kRequestNone;
    pContext->pRequest->pfnStatusRead = NULL;
    return nResult;
}

// NTSC-U/C: 0x002cb7d0, PAL: 0x002d5740
static int RtUpnpEndExternalAddress(RtUpnpContext *pContext) {
    RtUpnpActionReply *pReply;
    char *pszValue;
    RtUpnpRequest *pRequest;
    unsigned char bDone;
    unsigned int i;
    int nResult;

    bDone = 0;
    nResult = RtUpnpReceiveHttpBody(pContext, &bDone);
    if (nResult == 0 && !bDone) {
        return 0;
    }
    if (pContext->pRequest->header.nStatus == kHttpOk && bDone) {
        if (!pContext->pRequest->bAction) {
            pszValue = NULL;
            if (RtXmlParserFeed(pContext->pRequest->pParser, "", 0, 1) != 0 ||
                RtXmlParserGetResult(pContext->pRequest->pParser, (void **)&pszValue) != 0) {
                nResult = RT_UPNP_RESULT_PARSE_FAILED;
            } else {
                strncpy(pContext->szExternalAddress, pszValue, RT_SOCKET_ADDRESS_SIZE);
            }
        } else {
            pReply = NULL;
            if (RtXmlParserFeed(pContext->pRequest->pParser, "", 0, 1) != 0 ||
                RtXmlParserGetResult(pContext->pRequest->pParser, (void **)&pReply) != 0) {
                nResult = RT_UPNP_RESULT_PARSE_FAILED;
            } else {
                for (i = 0; i < pReply->uCount; ++i) {
                    if (strncmp("NewExternalIPAddress", pReply->aszNames[i], RT_UPNP_TEXT_SIZE) ==
                        0) {
                        strncpy(pContext->szExternalAddress,
                                pReply->aszValues[i],
                                RT_SOCKET_ADDRESS_SIZE);
                    }
                }
            }
        }
    }
    RtUpnpCloseRequest(pContext->pRequest);
    pRequest = pContext->pRequest;
    if (!pRequest->bAction && nResult != 0) {
        // The variable could not be queried. Ask with the action instead.
        nResult = RtUpnpSendAction(pContext, "GetExternalIPAddress", "");
        if (nResult == 0) {
            RtReadClock(&pContext->pRequest->startTime);
            return 0;
        }
        pRequest = pContext->pRequest;
    }
    pRequest->pfnAddressRead(pContext, nResult, pRequest->nService, pContext->szExternalAddress);
    pContext->nRequest = kRequestNone;
    pContext->pRequest->pfnAddressRead = NULL;
    return nResult;
}

// Start the scan of the mappings after the count of the mappings could not be read.
static inline void RtUpnpStartScanWithoutCount(RtUpnpContext *pContext) {
    RtUpnpRequest *pRequest;

    pRequest = pContext->pRequest;
    pContext->uExternalPort = 0;
    pRequest->bMappingFound = 0;
    pRequest->nMappingCount = kMappingScanDefault;
    pRequest->nMappingIndex = 0;
    pContext->pRequest->bAction = 1;
}

// Pick the external port when the scan did not find the mapping of this console.
static inline void RtUpnpPickExternalPort(RtUpnpContext *pContext) {
    if (pContext->uExternalPort < pContext->uPort) {
        pContext->uExternalPort = pContext->uPort;
    } else {
        ++pContext->uExternalPort;
    }
}

// NTSC-U/C: 0x002cba40, PAL: 0x002d59b0
static int RtUpnpEndMappingScan(RtUpnpContext *pContext) {
    char szArguments[kMappingIndexSize];
    RtUpnpActionReply *pReply;
    char *pszCount;
    RtUpnpRequest *pRequest;
    unsigned char bDone;
    unsigned char bClient;
    unsigned char bProtocol;
    unsigned char bPort;
    unsigned short uExternalPort;
    unsigned int i;
    int nCount;
    int nResult;

    bDone = 0;
    nResult = RtUpnpReceiveHttpBody(pContext, &bDone);
    if (nResult == 0 && !bDone) {
        return 0;
    }
    pRequest = pContext->pRequest;
    if (pRequest->header.nStatus != kHttpOk || !bDone) {
        if (!pRequest->bAction) {
            RtUpnpStartScanWithoutCount(pContext);
        } else {
            pRequest->nMappingIndex = pRequest->nMappingCount;
        }
    } else if (!pRequest->bAction) {
        pszCount = NULL;
        if (RtXmlParserFeed(pRequest->pParser, "", 0, 1) != 0 ||
            RtXmlParserGetResult(pContext->pRequest->pParser, (void **)&pszCount) != 0) {
            RtUpnpStartScanWithoutCount(pContext);
            nResult = 0;
        } else {
            nCount = strtol(pszCount, NULL, kHttpNumberBase);
            pRequest = pContext->pRequest;
            pContext->uExternalPort = 0;
            pRequest->bMappingFound = 0;
            pRequest->nMappingCount = nCount;
            pRequest->nMappingIndex = 0;
            pContext->pRequest->bAction = 1;
        }
    } else {
        pReply = NULL;
        if (RtXmlParserFeed(pRequest->pParser, "", 0, 1) != 0 ||
            RtXmlParserGetResult(pContext->pRequest->pParser, (void **)&pReply) != 0) {
            pRequest = pContext->pRequest;
            if (!pRequest->bAction) {
                RtUpnpStartScanWithoutCount(pContext);
            } else {
                pRequest->nMappingIndex = pRequest->nMappingCount;
            }
            nResult = 0;
        } else {
            bClient = 0;
            bProtocol = 0;
            bPort = 0;
            uExternalPort = 0;
            for (i = 0; i < pReply->uCount; ++i) {
                if (strncmp(pReply->aszNames[i], "NewInternalClient", RT_UPNP_TEXT_SIZE) == 0 &&
                    strncmp(pReply->aszValues[i], pContext->szLocalAddress, RT_UPNP_TEXT_SIZE) ==
                        0) {
                    bClient = 1;
                } else if (strncmp(pReply->aszNames[i], "NewInternalPort", RT_UPNP_TEXT_SIZE) ==
                           0) {
                    if ((unsigned short)strtol(pReply->aszValues[i], NULL, kHttpNumberBase) ==
                        pContext->uPort) {
                        bPort = 1;
                    }
                } else if (strncmp(pReply->aszNames[i], "NewProtocol", RT_UPNP_TEXT_SIZE) == 0 &&
                           strncmp(pReply->aszValues[i], "UDP", RT_UPNP_TEXT_SIZE) == 0) {
                    bProtocol = 1;
                } else if (strncmp(pReply->aszNames[i], "NewExternalPort", RT_UPNP_TEXT_SIZE) ==
                           0) {
                    uExternalPort = strtol(pReply->aszValues[i], NULL, kHttpNumberBase);
                }
            }
            if (bClient && bProtocol && bPort) {
                pContext->uExternalPort = uExternalPort;
                pContext->pRequest->bMappingFound = 1;
            } else if (!pContext->pRequest->bMappingFound &&
                       pContext->uExternalPort < uExternalPort) {
                pContext->uExternalPort = uExternalPort;
            }
        }
    }
    RtUpnpCloseRequest(pContext->pRequest);
    pRequest = pContext->pRequest;
    if (!pRequest->bAction) {
        // Yes, the binary keeps the request running without a callback here.
        return nResult;
    }
    if (pRequest->nMappingIndex < pRequest->nMappingCount) {
        sprintf(
            szArguments, "<NewPortMappingIndex>%d</NewPortMappingIndex>", pRequest->nMappingIndex);
        ++pContext->pRequest->nMappingIndex;
        nResult = RtUpnpSendAction(pContext, "GetGenericPortMappingEntry", szArguments);
        if (nResult == 0) {
            RtReadClock(&pContext->pRequest->startTime);
            return 0;
        }
        pRequest = pContext->pRequest;
        if (!pRequest->bMappingFound) {
            RtUpnpPickExternalPort(pContext);
        }
    } else {
        if (!pRequest->bMappingFound) {
            RtUpnpPickExternalPort(pContext);
        }
        if (nResult == RT_UPNP_RESULT_HTTP_ERROR) {
            nResult = 0;
        }
    }
    pRequest->pfnMappingsScanned(pContext, nResult, pRequest->nService, pContext->uExternalPort);
    pContext->nRequest = kRequestNone;
    pContext->pRequest->pfnMappingsScanned = NULL;
    return nResult;
}

// NTSC-U/C: 0x002cbf28, PAL: 0x002d5e98
static int RtUpnpEndAddMapping(RtUpnpContext *pContext) {
    RtUpnpRequest *pRequest;
    unsigned char bDone;
    int nResult;

    bDone = 0;
    nResult = RtUpnpReceiveHttpBody(pContext, &bDone);
    if (nResult == 0 && !bDone) {
        return 0;
    }
    pContext->bMapped = 1;
    if (pContext->pRequest->header.nStatus != kHttpOk || !bDone) {
        pContext->bMapped = 0;
        nResult = RT_UPNP_RESULT_HTTP_ERROR;
    }
    RtUpnpCloseRequest(pContext->pRequest);
    pRequest = pContext->pRequest;
    pRequest->pfnMappingAdded(pContext, nResult, pRequest->nService);
    pContext->nRequest = kRequestNone;
    pContext->pRequest->pfnMappingAdded = NULL;
    return nResult;
}

// NTSC-U/C: 0x002cbff8, PAL: 0x002d5f68
static int RtUpnpEndVerifyMapping(RtUpnpContext *pContext) {
    RtUpnpActionReply *pReply;
    RtUpnpRequest *pRequest;
    unsigned char bDone;
    unsigned char bClient;
    unsigned char bPort;
    unsigned char bEnabled;
    unsigned int i;
    int nResult;

    bDone = 0;
    nResult = RtUpnpReceiveHttpBody(pContext, &bDone);
    if (nResult == 0 && !bDone) {
        return 0;
    }
    if (pContext->pRequest->header.nStatus != kHttpOk || !bDone) {
        nResult = RT_UPNP_RESULT_HTTP_ERROR;
    } else if (RtXmlParserFeed(pContext->pRequest->pParser, "", 0, 1) == 0) {
        pReply = NULL;
        if (RtXmlParserGetResult(pContext->pRequest->pParser, (void **)&pReply) == 0) {
            bClient = 0;
            bPort = 0;
            bEnabled = 0;
            for (i = 0; i < pReply->uCount; ++i) {
                if (strncmp(pReply->aszNames[i], "NewInternalClient", RT_UPNP_TEXT_SIZE) == 0 &&
                    strncmp(pReply->aszValues[i], pContext->szLocalAddress, RT_UPNP_TEXT_SIZE) ==
                        0) {
                    bClient = 1;
                } else if (strncmp(pReply->aszNames[i], "NewInternalPort", RT_UPNP_TEXT_SIZE) ==
                           0) {
                    if ((unsigned short)strtol(pReply->aszValues[i], NULL, kHttpNumberBase) ==
                        pContext->uPort) {
                        bPort = 1;
                    }
                } else if (strncmp(pReply->aszNames[i], "NewEnabled", RT_UPNP_TEXT_SIZE) == 0) {
                    bEnabled = strtol(pReply->aszValues[i], NULL, kHttpNumberBase);
                }
            }
            if (!bClient) {
                nResult = RT_UPNP_RESULT_UNKNOWN;
            } else if (bPort && bEnabled) {
                nResult = 0;
            } else {
                // Yes, the binary reports the offset of the last argument it read.
                nResult = pReply->uCount * RT_UPNP_TEXT_SIZE;
            }
        }
    }
    RtUpnpCloseRequest(pContext->pRequest);
    pRequest = pContext->pRequest;
    pRequest->pfnMappingVerified(pContext, nResult, pRequest->nService);
    pContext->nRequest = kRequestNone;
    pContext->pRequest->pfnMappingVerified = NULL;
    return nResult;
}

// NTSC-U/C: 0x002cc260, PAL: 0x002d61d0
static int RtUpnpEndRemoveMapping(RtUpnpContext *pContext) {
    RtUpnpRequest *pRequest;
    unsigned char bDone;
    int nResult;

    bDone = 0;
    nResult = 0;
    if (!pContext->bMapped) {
        pRequest = pContext->pRequest;
        pRequest->pfnMappingRemoved(pContext, 0, pRequest->nService);
        pContext->nRequest = kRequestNone;
        pContext->pRequest->pfnMappingRemoved = NULL;
        return nResult;
    }
    nResult = RtUpnpReceiveHttpBody(pContext, &bDone);
    if (nResult == 0 && !bDone) {
        return 0;
    }
    if (pContext->pRequest->header.nStatus != kHttpOk || !bDone) {
        nResult = RT_UPNP_RESULT_HTTP_ERROR;
    }
    RtUpnpCloseRequest(pContext->pRequest);
    pRequest = pContext->pRequest;
    pRequest->pfnMappingRemoved(pContext, nResult, pRequest->nService);
    pContext->nRequest = kRequestNone;
    pContext->pRequest->pfnMappingRemoved = NULL;
    return nResult;
}

// End the probe and report its result.
static inline void RtUpnpEndProbe(RtUpnpContext *pContext) {
    pContext->pRequest->pfnProbed(
        pContext, pContext->probe.nResult, pContext->szExternalAddress, pContext->uExternalPort);
    RtUpnpFreeRequest(pContext);
    pContext->nRequest = kRequestNone;
}

// NTSC-U/C: 0x002cc350, PAL: 0x002d62c0
static int RtUpnpCheckNatProbe(RtUpnpContext *pContext) {
    RtUpnpProbeReply reply;
    char szSender[RT_SOCKET_ADDRESS_SIZE];
    unsigned char abRequest[kProbeRequestSize];
    unsigned short uSenderPort;
    unsigned short uSize;
    unsigned short uSent;
    unsigned int uElapsed;
    int nResult;

    nResult = 0;
    uSenderPort = 0;
    uSize = 0;
    if (RtSocketReceiveFrom(
            pContext->pSocket, szSender, &uSenderPort, &reply, kProbeReplySize, &uSize) == 0 &&
        uSize == kProbeReplySize) {
        if (strncmp(szSender, pContext->probe.szGateway, RT_SOCKET_ADDRESS_SIZE) != 0 ||
            uSenderPort != pContext->probe.uPort) {
            nResult = RT_UPNP_RESULT_RECEIVE_FAILED;
        } else {
            sprintf(pContext->szExternalAddress,
                    "%u.%u.%u.%u",
                    reply.abAddress[0],
                    reply.abAddress[1],
                    reply.abAddress[2],
                    reply.abAddress[3]);
            pContext->probe.bKeepAlive = 1;
            pContext->uExternalPort = (unsigned short)((reply.uPort >> 8) | (reply.uPort << 8));
            pContext->probe.nResult = 0;
            RtReadClock(&pContext->probe.time);
            RtUpnpEndProbe(pContext);
        }
    }
    if (pContext->nRequest == kRequestNone) {
        return nResult;
    }
    (void)RtGetMillisecondsSince(&pContext->probe.time, &uElapsed);
    if (uElapsed <= kProbeInterval) {
        return nResult;
    }
    if (pContext->probe.nAttempts >= kProbeAttempts) {
        pContext->probe.nResult = RT_UPNP_RESULT_TIMEOUT;
        RtUpnpEndProbe(pContext);
        return nResult;
    }
    // Yes, the binary sends the bytes other than the opcode uninitialised.
    abRequest[kProbeOpcodeIndex] = PROBE_OPCODE;
    uSent = 0;
    if (RtSocketSendTo(pContext->pSocket,
                       pContext->probe.szGateway,
                       pContext->probe.uPort,
                       abRequest,
                       kProbeRequestSize,
                       &uSent) == 0 &&
        uSent == kProbeRequestSize) {
        RtReadClock(&pContext->probe.time);
        ++pContext->probe.nAttempts;
        return nResult;
    }
    pContext->probe.nResult = RT_UPNP_RESULT_SEND_FAILED;
    RtUpnpEndProbe(pContext);
    return nResult;
}

// NTSC-U/C: 0x002cc540, PAL: 0x002d64b0
static int RtUpnpSendNatKeepAlive(RtUpnpContext *pContext) {
    unsigned char abRequest[kProbeRequestSize];
    unsigned short uSent;
    unsigned int uElapsed;

    uSent = 0;
    if (!pContext->probe.bKeepAlive) {
        return 0;
    }
    (void)RtGetMillisecondsSince(&pContext->probe.time, &uElapsed);
    if (uElapsed <= kKeepAliveInterval) {
        return 0;
    }
    // Yes, the binary sends the bytes other than the opcode uninitialised.
    abRequest[kKeepAliveOpcodeIndex] = KEEP_ALIVE_OPCODE;
    RtSocketSendTo(pContext->pSocket,
                   pContext->probe.szGateway,
                   pContext->probe.uPort,
                   abRequest,
                   kProbeRequestSize,
                   &uSent);
    RtReadClock(&pContext->probe.time);
    return 0;
}

// NTSC-U/C: 0x002cc848, PAL: 0x002d67b8
static void RtUpnpOnDiscovered(RtUpnpContext *pContext, int nResult, int nService) {
    (void)nService;
    if (pContext == NULL) {
        return;
    }
    pContext->pRequest->nStep = nResult == 0 ? kStepDescription : kStepProbe;
}

// NTSC-U/C: 0x002cc878, PAL: 0x002d67e8
static void RtUpnpOnDescribed(RtUpnpContext *pContext, int nResult, int nService) {
    if (pContext == NULL) {
        return;
    }
    if (nResult != 0) {
        pContext->pRequest->nStep = kStepProbe;
    } else if ((nService & RT_UPNP_WAN_IP_CONNECTION) != 0) {
        pContext->pRequest->nStep = kStepScpd;
    } else if ((nService & RT_UPNP_WAN_PPP_CONNECTION) != 0) {
        pContext->pRequest->bUsePpp = 1;
        pContext->pRequest->nStep = kStepScpd;
    } else {
        pContext->pRequest->nStep = kStepProbe;
    }
}

// Go on to the next step, or fall back to the PPP service once, or give up.
static inline void RtUpnpNextStep(RtUpnpContext *pContext, int nResult, int nNextStep) {
    RtUpnpRequest *pRequest;

    pRequest = pContext->pRequest;
    if (nResult == 0) {
        pRequest->nStep = nNextStep;
    } else if (!pRequest->bUsePpp) {
        pRequest->bUsePpp = 1;
        pContext->pRequest->nStep = kStepScpd;
    } else {
        pRequest->nStep = kStepProbe;
    }
}

// NTSC-U/C: 0x002cc8e8, PAL: 0x002d6858
static void RtUpnpOnScpdLoaded(RtUpnpContext *pContext, int nResult, int nService) {
    (void)nService;
    if (pContext == NULL) {
        return;
    }
    RtUpnpNextStep(pContext, nResult, kStepStatus);
}

// NTSC-U/C: 0x002cc940, PAL: 0x002d68b0
static void RtUpnpOnStatusRead(RtUpnpContext *pContext, int nResult, int nService, int nStatus) {
    (void)nService;
    if (pContext == NULL) {
        return;
    }
    RtUpnpNextStep(pContext, nResult == 0 && nStatus == kStatusConnected ? 0 : 1, kStepAddress);
}

// NTSC-U/C: 0x002cc9a8, PAL: 0x002d6918
static void
RtUpnpOnAddressRead(RtUpnpContext *pContext, int nResult, int nService, const char *pszAddress) {
    (void)nService;
    (void)pszAddress;
    if (pContext == NULL) {
        return;
    }
    RtUpnpNextStep(pContext, nResult, kStepMappingScan);
}

// NTSC-U/C: 0x002cca00, PAL: 0x002d6970
static void
RtUpnpOnMappingsScanned(RtUpnpContext *pContext, int nResult, int nService, unsigned short uPort) {
    (void)nService;
    (void)uPort;
    if (pContext == NULL) {
        return;
    }
    RtUpnpNextStep(pContext, nResult, kStepAddMapping);
}

// NTSC-U/C: 0x002cca58, PAL: 0x002d69c8
static void RtUpnpOnMappingAdded(RtUpnpContext *pContext, int nResult, int nService) {
    (void)nService;
    if (pContext == NULL) {
        return;
    }
    RtUpnpNextStep(pContext, nResult, kStepProbe);
}

// NTSC-U/C: 0x002ccab0, PAL: 0x002d6a20
static void RtUpnpOnProbeFinished(RtUpnpContext *pContext,
                                  int nResult,
                                  const char *pszAddress,
                                  unsigned short uPort) {
    RtUpnpRequest *pRequest;

    if (pContext == NULL) {
        return;
    }
    pRequest = pContext->pRequest;
    pRequest->nStep = kStepNone;
    pRequest->pfnDone(pContext, nResult, pRequest->nService, pszAddress, uPort, pContext->pUser);
    pContext->pRequest->pfnDone = NULL;
}

// NTSC-U/C: 0x002ccc48, PAL: 0x002d6bb8
static int
RtUpnpStartDiscovery(RtUpnpContext *pContext, int nService, RtUpnpStepCallback pfnDiscovered) {
    char szMessage[kHttpUrlSize];
    RtSocketOptions options;
    const char *pszServiceType;
    // Yes, the binary reads the count the socket wrote as signed.
    union {
        unsigned short uSent;
        short nSent;
    } sent;

    sent.nSent = 0;
    if (pContext == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (pContext->nRequest != kRequestNone) {
        return RT_UPNP_RESULT_BUSY;
    }
    if (pfnDiscovered == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (RtUpnpAllocRequest(pContext) != 0) {
        return RT_UPNP_RESULT_NO_MEMORY;
    }
    if (!g_bDiscoveryDone) {
        memset(&pContext->pRequest->discoveryHeader, 0, sizeof(RtUpnpHttpHeader));
        switch (nService) {
        case RT_UPNP_LAYER3_FORWARDING:
            pszServiceType = "urn:schemas-upnp-org:service:Layer3Forwarding:1";
            break;
        case RT_UPNP_WAN_COMMON_CONFIG:
            pszServiceType = "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1";
            break;
        case RT_UPNP_WAN_IP_CONNECTION:
            pszServiceType = "urn:schemas-upnp-org:service:WANIPConnection:1";
            break;
        case RT_UPNP_WAN_PPP_CONNECTION:
            pszServiceType = "urn:schemas-upnp-org:service:WANPPPConnection:1";
            break;
        default:
            return RT_UPNP_RESULT_INVALID;
        }
        sprintf(szMessage,
                "M-SEARCH * HTTP/1.1\r\nHost: %s:%d\r\nMan: \"ssdp:discover\"\r\nST: %s\r\nMX: "
                "1\r\n\r\n",
                SSDP_ADDRESS,
                kSsdpPort,
                pszServiceType);
        memset(&options, 0, sizeof(options));
        options.nType = RT_SOCKET_DATAGRAM;
        options.nBufferSize = kSocketBufferSize;
        if (RtSocketCreate(&pContext->pRequest->pSocket, NULL, kSsdpPort, &options) != 0) {
            return RT_UPNP_RESULT_NO_SOCKET;
        }
        if (RtSocketSendTo(pContext->pRequest->pSocket,
                           SSDP_ADDRESS,
                           kSsdpPort,
                           szMessage,
                           strlen(szMessage),
                           &sent.uSent) != 0 ||
            strlen(szMessage) != (unsigned int)sent.nSent) {
            RtSocketClose(pContext->pRequest->pSocket);
            return RT_UPNP_RESULT_SEND_FAILED;
        }
    }
    pContext->nRequest = kRequestDiscovery;
    RtReadClock(&pContext->pRequest->startTime);
    pContext->pRequest->nService = nService;
    pContext->pRequest->pfnDiscovered = pfnDiscovered;
    return 0;
}

// NTSC-U/C: 0x002cce38, PAL: 0x002d6da8
static int RtUpnpStartDescription(RtUpnpContext *pContext, RtUpnpStepCallback pfnDescribed) {
    char szHost[RT_SOCKET_ADDRESS_SIZE];
    char szPath[RT_UPNP_HEADER_SIZE];
    char szMessage[kHttpUrlSize];
    unsigned short uPort;
    int nResult;

    if (pContext == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (pContext->nRequest != kRequestNone) {
        return RT_UPNP_RESULT_BUSY;
    }
    if (pfnDescribed == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (RtUpnpAllocRequest(pContext) != 0) {
        return RT_UPNP_RESULT_NO_MEMORY;
    }
    if (!g_bDescriptionDone) {
        memset(&g_deviceInfo, 0, sizeof(RtUpnpDeviceInfo));
        pContext->pRequest->bHeaderRead = 0;
        pContext->pRequest->nBodyRead = 0;
        strncpy(szMessage, pContext->pRequest->discoveryHeader.szLocation, RT_UPNP_HEADER_SIZE);
        nResult = RtUpnpParseUrl(szMessage, szPath, szHost, &uPort);
        if (nResult != 0) {
            return nResult;
        }
        if (RtXmlParserCreate(&pContext->pRequest->pParser, RT_UPNP_XML_DESCRIPTION) != 0 ||
            pContext->pRequest->pParser == NULL) {
            return RT_UPNP_RESULT_PARSE_FAILED;
        }
        nResult = RtUpnpConnect(pContext->pRequest, szHost, uPort);
        if (nResult != 0) {
            return nResult;
        }
        sprintf(szMessage,
                "GET /%s HTTP/1.1\r\nHost:%s:%d\r\nConnection:close\r\n\r\n",
                szPath,
                szHost,
                uPort);
        nResult = RtUpnpSendMessage(pContext->pRequest, szMessage);
        if (nResult != 0) {
            return nResult;
        }
    }
    pContext->nRequest = kRequestDescription;
    RtReadClock(&pContext->pRequest->startTime);
    pContext->pRequest->pfnDescribed = pfnDescribed;
    return 0;
}

// NTSC-U/C: 0x002cd018, PAL: 0x002d6f88
static int RtUpnpStartScpd(RtUpnpContext *pContext, int nService, RtUpnpStepCallback pfnScpdRead) {
    char szHost[RT_SOCKET_ADDRESS_SIZE];
    char szPath[RT_UPNP_HEADER_SIZE];
    char szMessage[kHttpUrlSize];
    unsigned short uPort;
    int nMode;
    int nResult;

    if (pContext == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (pContext->nRequest != kRequestNone) {
        return RT_UPNP_RESULT_BUSY;
    }
    if (pfnScpdRead == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (RtUpnpAllocRequest(pContext) != 0) {
        return RT_UPNP_RESULT_NO_MEMORY;
    }
    if (((nService & RT_UPNP_WAN_IP_CONNECTION) != 0 && !g_bIpScpdDone) ||
        ((nService & RT_UPNP_WAN_PPP_CONNECTION) != 0 && !g_bPppScpdDone)) {
        nResult = RtUpnpGetScpdUrl(&g_deviceInfo, nService, szMessage);
        if (nResult != 0) {
            return nResult;
        }
        nResult = RtUpnpParseUrl(szMessage, szPath, szHost, &uPort);
        if (nResult != 0) {
            return nResult;
        }
        if ((nService & g_deviceInfo.uFlags) == 0) {
            return RT_UPNP_RESULT_NO_SERVICE;
        }
        if (nService == RT_UPNP_WAN_IP_CONNECTION) {
            g_uIpActions = 0;
            nMode = RT_UPNP_XML_IP_SCPD;
        } else if (nService == RT_UPNP_WAN_PPP_CONNECTION) {
            g_uPppActions = 0;
            nMode = RT_UPNP_XML_PPP_SCPD;
        } else {
            return RT_UPNP_RESULT_INVALID;
        }
        pContext->pRequest->bHeaderRead = 0;
        pContext->pRequest->nBodyRead = 0;
        if (RtXmlParserCreate(&pContext->pRequest->pParser, nMode) != 0 ||
            pContext->pRequest->pParser == NULL) {
            return RT_UPNP_RESULT_PARSE_FAILED;
        }
        nResult = RtUpnpConnect(pContext->pRequest, szHost, uPort);
        if (nResult != 0) {
            return nResult;
        }
        sprintf(szMessage,
                "GET /%s HTTP/1.1\r\nHost:%s:%d\r\nConnection:close\r\n\r\n",
                szPath,
                szHost,
                uPort);
        nResult = RtUpnpSendMessage(pContext->pRequest, szMessage);
        if (nResult != 0) {
            return nResult;
        }
    }
    pContext->nRequest = kRequestScpd;
    RtReadClock(&pContext->pRequest->startTime);
    pContext->pRequest->nService = nService;
    pContext->pRequest->pfnScpdRead = pfnScpdRead;
    return 0;
}

// NTSC-U/C: 0x002cd268, PAL: 0x002d71d8
static int
RtUpnpStartStatusQuery(RtUpnpContext *pContext, int nService, RtUpnpStatusCallback pfnStatusRead) {
    int nResult;

    if (pContext == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (pContext->nRequest != kRequestNone) {
        return RT_UPNP_RESULT_BUSY;
    }
    if (pfnStatusRead == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (RtUpnpAllocRequest(pContext) != 0) {
        return RT_UPNP_RESULT_NO_MEMORY;
    }
    nResult = RtUpnpSendQuery(pContext, nService, "ConnectionStatus");
    if (nResult != 0) {
        return nResult;
    }
    pContext->nRequest = kRequestStatus;
    RtReadClock(&pContext->pRequest->startTime);
    pContext->pRequest->nService = nService;
    pContext->pRequest->pfnStatusRead = pfnStatusRead;
    return nResult;
}

// NTSC-U/C: 0x002cd328, PAL: 0x002d7298
static int RtUpnpStartAddressQuery(RtUpnpContext *pContext,
                                   int nService,
                                   RtUpnpAddressCallback pfnAddressRead) {
    int nResult;

    if (pContext == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (pContext->nRequest != kRequestNone) {
        return RT_UPNP_RESULT_BUSY;
    }
    if (pfnAddressRead == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (RtUpnpAllocRequest(pContext) != 0) {
        return RT_UPNP_RESULT_NO_MEMORY;
    }
    nResult = RtUpnpSendQuery(pContext, nService, "ExternalIPAddress");
    if (nResult != 0) {
        return nResult;
    }
    pContext->nRequest = kRequestAddress;
    RtReadClock(&pContext->pRequest->startTime);
    pContext->pRequest->nService = nService;
    pContext->pRequest->pfnAddressRead = pfnAddressRead;
    return nResult;
}

// NTSC-U/C: 0x002cd3e8, PAL: 0x002d7358
static int RtUpnpStartMappingScan(RtUpnpContext *pContext,
                                  int nService,
                                  RtUpnpPortCallback pfnMappingsScanned) {
    int nResult;

    if (pContext == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (pContext->nRequest != kRequestNone) {
        return RT_UPNP_RESULT_BUSY;
    }
    if (pfnMappingsScanned == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (RtUpnpAllocRequest(pContext) != 0) {
        return RT_UPNP_RESULT_NO_MEMORY;
    }
    nResult = RtUpnpSendQuery(pContext, nService, "PortMappingNumberOfEntries");
    if (nResult != 0) {
        return nResult;
    }
    pContext->nRequest = kRequestMappingScan;
    RtReadClock(&pContext->pRequest->startTime);
    pContext->pRequest->nService = nService;
    pContext->pRequest->pfnMappingsScanned = pfnMappingsScanned;
    return nResult;
}

// NTSC-U/C: 0x002cd4a8, PAL: 0x002d7418
static int RtUpnpStartAddMapping(RtUpnpContext *pContext,
                                 int nService,
                                 unsigned short uExternalPort,
                                 RtUpnpStepCallback pfnMappingAdded) {
    char szArguments[RT_UPNP_HEADER_SIZE];
    int nResult;

    (void)uExternalPort; // Yes, the binary maps the port the context holds instead.
    if (pContext == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (pContext->nRequest != kRequestNone) {
        return RT_UPNP_RESULT_BUSY;
    }
    if (pfnMappingAdded == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (RtUpnpAllocRequest(pContext) != 0) {
        return RT_UPNP_RESULT_NO_MEMORY;
    }
    pContext->pRequest->nService = nService;
    sprintf(szArguments,
            "<NewRemoteHost></NewRemoteHost><NewExternalPort>%d</NewExternalPort><NewProtocol>UDP"
            "</NewProtocol><NewInternalPort>%d</NewInternalPort><NewInternalClient>%s"
            "</NewInternalClient><NewEnabled>1</NewEnabled><NewPortMappingDescription>SCE-RT"
            "</NewPortMappingDescription><NewLeaseDuration>0</NewLeaseDuration>",
            pContext->uExternalPort,
            pContext->uPort,
            pContext->szLocalAddress);
    nResult = RtUpnpSendAction(pContext, "AddPortMapping", szArguments);
    if (nResult != 0) {
        return nResult;
    }
    pContext->nRequest = kRequestAddMapping;
    RtReadClock(&pContext->pRequest->startTime);
    pContext->pRequest->pfnMappingAdded = pfnMappingAdded;
    return nResult;
}

// NTSC-U/C: 0x002cd580, PAL: 0x002d74f0
static int RtUpnpStartNatProbe(RtUpnpContext *pContext,
                               const char *pszGateway,
                               unsigned short uPort,
                               RtUpnpProbeCallback pfnProbed) {
    unsigned char abRequest[kProbeRequestSize];
    unsigned short uSent;

    uSent = 0;
    if (pContext == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (pContext->nRequest != kRequestNone) {
        return RT_UPNP_RESULT_BUSY;
    }
    if (pszGateway == NULL || pfnProbed == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (RtUpnpAllocRequest(pContext) != 0) {
        return RT_UPNP_RESULT_NO_MEMORY;
    }
    memset(&pContext->probe, 0, sizeof(RtUpnpProbe));
    strncpy(pContext->probe.szGateway, pszGateway, RT_SOCKET_ADDRESS_SIZE);
    pContext->probe.uPort = uPort;
    pContext->probe.nResult = 0;
    pContext->pRequest->pfnProbed = pfnProbed;
    // Yes, the binary sends the bytes other than the opcode uninitialised.
    abRequest[kProbeOpcodeIndex] = PROBE_OPCODE;
    if (RtSocketSendTo(pContext->pSocket,
                       pContext->probe.szGateway,
                       uPort,
                       abRequest,
                       kProbeRequestSize,
                       &uSent) != 0 ||
        uSent != kProbeRequestSize) {
        RtUpnpFreeRequest(pContext);
        return RT_UPNP_RESULT_SEND_FAILED;
    }
    RtReadClock(&pContext->probe.time);
    pContext->nRequest = kRequestProbe;
    ++pContext->probe.nAttempts;
    return 0;
}

// NTSC-U/C: 0x002cc5c0, PAL: 0x002d6530
static int RtUpnpAdvanceStep(RtUpnpContext *pContext) {
    char szGateway[RT_SOCKET_ADDRESS_SIZE];
    RtUpnpRequest *pRequest;
    unsigned short uGatewayPort;
    int nService;
    int nResult;

    pRequest = pContext->pRequest;
    nResult = 0;
    if (pRequest->nStep >= kStepCount) {
        return nResult;
    }
    nService = pRequest->bUsePpp ? RT_UPNP_WAN_PPP_CONNECTION : RT_UPNP_WAN_IP_CONNECTION;
    switch (pRequest->nStep) {
    case kStepDescription:
        nResult = RtUpnpStartDescription(pContext, RtUpnpOnDescribed);
        break;
    case kStepScpd:
        nResult = RtUpnpStartScpd(pContext, nService, RtUpnpOnScpdLoaded);
        break;
    case kStepStatus:
        nResult = RtUpnpStartStatusQuery(pContext, nService, RtUpnpOnStatusRead);
        break;
    case kStepAddress:
        nResult = RtUpnpStartAddressQuery(pContext, nService, RtUpnpOnAddressRead);
        break;
    case kStepMappingScan:
        nResult = RtUpnpStartMappingScan(pContext, nService, RtUpnpOnMappingsScanned);
        break;
    case kStepAddMapping:
        nResult = RtUpnpStartAddMapping(
            pContext, nService, pContext->uExternalPort, RtUpnpOnMappingAdded);
        if (nResult == 0) {
            pContext->pRequest->nStep = kStepWaiting;
            return nResult;
        }
        pRequest = pContext->pRequest;
        if (!pRequest->bRetried) {
            pRequest->bRetried = 1;
            pRequest->nStep = kStepMappingScan;
        } else {
            pRequest->nStep = kStepProbe;
        }
        return nResult;
    case kStepProbe:
        uGatewayPort = pContext->probe.uPort;
        strncpy(szGateway, pContext->probe.szGateway, RT_SOCKET_ADDRESS_SIZE);
        nResult = RtUpnpStartNatProbe(pContext, szGateway, uGatewayPort, RtUpnpOnProbeFinished);
        if (nResult != 0) {
            // Yes, the binary writes through the state the failed probe released.
            pContext->pRequest->nStep = kStepNone;
        } else {
            pContext->pRequest->nStep = kStepWaiting;
        }
        return nResult;
    default:
        return nResult;
    }
    if (nResult != 0) {
        pContext->pRequest->nStep = kStepProbe;
    } else {
        pContext->pRequest->nStep = kStepWaiting;
    }
    return nResult;
}

int RtUpnpCreateContext(RtUpnpContext **ppContext,
                        RtSocket *pSocket,
                        unsigned short uPort,
                        void *pUser) {
    RtUpnpContext *pContext;

    pContext = NULL;
    if (ppContext == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    *ppContext = NULL;
    if (pSocket == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (RtAllocZeroedBlock((void **)&pContext, sizeof(RtUpnpContext)) != 0) {
        return RT_UPNP_RESULT_NO_MEMORY;
    }
    if (RtAllocZeroedBlock((void **)&pContext->pRequest, sizeof(RtUpnpRequest)) != 0) {
        RtFreeBlock((void **)&pContext);
        return RT_UPNP_RESULT_NO_MEMORY;
    }
    if (RtSocketGetLocalAddress(pContext->szLocalAddress) != 0) {
        RtFreeBlock((void **)&pContext->pRequest);
        RtFreeBlock((void **)&pContext);
        return RT_UPNP_RESULT_NO_SOCKET;
    }
    pContext->uPort = uPort;
    pContext->pSocket = pSocket;
    pContext->pUser = pUser;
    *ppContext = pContext;
    return 0;
}

int RtUpnpDestroyContext(RtUpnpContext *pContext) {
    if (pContext == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (pContext->pRequest != NULL) {
        RtSocketClose(pContext->pRequest->pSocket);
        RtFreeBlock((void **)&pContext->pRequest);
    }
    RtFreeBlock((void **)&pContext);
    return 0;
}

int RtUpnpBeginMapping(RtUpnpContext *pContext,
                       const char *pszGateway,
                       unsigned short uGatewayPort,
                       RtUpnpMappingCallback pfnDone) {
    int nResult;

    if (pContext == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    if (pContext->nRequest != kRequestNone) {
        return RT_UPNP_RESULT_BUSY;
    }
    if (RtUpnpAllocRequest(pContext) != 0) {
        return RT_UPNP_RESULT_NO_MEMORY;
    }
    nResult = RtUpnpStartDiscovery(pContext, RT_UPNP_WAN_IP_CONNECTION, RtUpnpOnDiscovered);
    if (nResult == 0) {
        pContext->probe.uPort = uGatewayPort;
        pContext->pRequest->pfnDone = pfnDone;
        pContext->pRequest->nStep = kStepWaiting;
        strncpy(pContext->probe.szGateway, pszGateway, RT_SOCKET_ADDRESS_SIZE);
    }
    return nResult;
}

// NTSC-U/C: 0x002cd760, PAL: 0x002d76d0
static int RtUpnpAllocRequest(RtUpnpContext *pContext) {
    if (pContext->pRequest == NULL &&
        RtAllocZeroedBlock((void **)&pContext->pRequest, sizeof(RtUpnpRequest)) != 0) {
        return RT_UPNP_RESULT_NO_MEMORY;
    }
    return 0;
}

// NTSC-U/C: 0x002cd798, PAL: 0x002d7708
static int RtUpnpFreeRequest(RtUpnpContext *pContext) {
    RtFreeBlock((void **)&pContext->pRequest);
    pContext->pRequest = NULL;
    return 0;
}

int RtUpnpPollRequest(RtUpnpContext *pContext) {
    RtUpnpRequest *pRequest;
    unsigned char bDone;
    int nResult;

    nResult = 0;
    if (pContext == NULL) {
        return RT_UPNP_RESULT_INVALID;
    }
    switch (pContext->nRequest) {
    case kRequestNone:
        nResult = RtUpnpSendNatKeepAlive(pContext);
        break;
    case kRequestDiscovery:
        if (!g_bDiscoveryDone) {
            bDone = 0;
            nResult = RtUpnpReceiveDiscoveryReply(pContext, &bDone);
            if (nResult == 0 && !bDone) {
                break;
            }
            g_nDiscoveryResult = nResult;
            g_bDiscoveryDone = 1;
            RtSocketClose(pContext->pRequest->pSocket);
            pRequest = pContext->pRequest;
            pRequest->pSocket = NULL;
            pRequest->pfnDiscovered(pContext, nResult, pRequest->nService);
        } else {
            pRequest = pContext->pRequest;
            pRequest->pfnDiscovered(pContext, g_nDiscoveryResult, pRequest->nService);
        }
        pContext->nRequest = kRequestNone;
        pContext->pRequest->pfnDiscovered = NULL;
        break;
    case kRequestDescription:
        if (!g_bDescriptionDone) {
            nResult = RtUpnpEndDescription(pContext);
            break;
        }
        pRequest = pContext->pRequest;
        pRequest->pfnDescribed(pContext, g_nDescriptionResult, pRequest->nService);
        pContext->nRequest = kRequestNone;
        pContext->pRequest->pfnDescribed = NULL;
        break;
    case kRequestScpd:
        pRequest = pContext->pRequest;
        if ((pRequest->nService & RT_UPNP_WAN_IP_CONNECTION) != 0 && !g_bIpScpdDone) {
            nResult = RtUpnpEndScpd(pContext);
            break;
        }
        if ((pRequest->nService & RT_UPNP_WAN_PPP_CONNECTION) != 0 && !g_bPppScpdDone) {
            nResult = RtUpnpEndScpd(pContext);
            break;
        }
        if ((pRequest->nService & RT_UPNP_WAN_IP_CONNECTION) != 0) {
            pRequest->pfnScpdRead(pContext, g_nIpScpdResult, pRequest->nService);
        } else {
            pRequest->pfnScpdRead(pContext, g_nPppScpdResult, pRequest->nService);
        }
        pContext->nRequest = kRequestNone;
        pContext->pRequest->pfnScpdRead = NULL;
        break;
    case kRequestStatus:
        nResult = RtUpnpEndStatus(pContext);
        break;
    case kRequestAddress:
        nResult = RtUpnpEndExternalAddress(pContext);
        break;
    case kRequestMappingScan:
        nResult = RtUpnpEndMappingScan(pContext);
        break;
    case kRequestAddMapping:
        nResult = RtUpnpEndAddMapping(pContext);
        break;
    case kRequestVerifyMapping:
        nResult = RtUpnpEndVerifyMapping(pContext);
        break;
    case kRequestRemoveMapping:
        nResult = RtUpnpEndRemoveMapping(pContext);
        break;
    case kRequestProbe:
        nResult = RtUpnpCheckNatProbe(pContext);
        break;
    default:
        break;
    }
    if (nResult != 0 || pContext->pRequest == NULL) {
        return nResult;
    }
    return RtUpnpAdvanceStep(pContext);
}
