#include "rt/rtupnpxml.h"

#include <ctype.h>
#include <string.h>

#include "rt/rtmemory.h"

// The library copies its text with strncpy() and bounds as large as the destinations, as retail
// does.
#pragma GCC diagnostic ignored "-Wstringop-truncation"

enum {
    kXmlResultNoMemory = 3,
    kXmlResultInvalid = 5,
    kXmlResultBadMode = 6,
    kXmlModeCount = 12,
    // The characters of the attribute name prefix the SOAP replies compare.
    kXmlnsLength = 5,
};

// The names of the elements of a device description the parser reads.
enum {
    kDescMajor,
    kDescMinor,
    kDescUrlBase,
    kDescDeviceType,
    kDescFriendlyName,
    kDescManufacturer,
    kDescManufacturerUrl,
    kDescModelDescription,
    kDescModelName,
    kDescModelNumber,
    kDescModelUrl,
    kDescSerialNumber,
    kDescUdn,
    kDescUpc,
    kDescServiceType,
    kDescServiceId,
    kDescControlUrl,
    kDescEventSubUrl,
    kDescScpdUrl,
    kDescElementCount,
};

// The names of the elements of a service description the parser reads.
enum {
    kScpdMajor,
    kScpdMinor,
    kScpdName,
};

// The attributes of the Envelope element of a SOAP reply, in order.
enum {
    kEnvelopeXmlns,
    kEnvelopeNamespace,
    kEnvelopeEncodingStyleName,
    kEnvelopeEncodingStyle,
};

// The attributes of the reply element of a SOAP reply that the parser checks.
enum {
    kReplyXmlns = 1,
    kReplyNamespace = 2,
};

// NTSC-U/C: 0x003b6d38
static const char *g_pszDeviceNamespace = "urn:schemas-upnp-org:device-1-0";
// NTSC-U/C: 0x003b6d3c
static const char *g_pszGatewayDeviceType = "urn:schemas-upnp-org:device:InternetGatewayDevice:1";
// NTSC-U/C: 0x003b6d40
static const char *g_pszWanDeviceType = "urn:schemas-upnp-org:device:WANDevice:1";
// NTSC-U/C: 0x003b6d44
static const char *g_pszWanConnectionDeviceType =
    "urn:schemas-upnp-org:device:WANConnectionDevice:1";
// NTSC-U/C: 0x003b6d48
static const char *g_pszLanDeviceType = "urn:schemas-upnp-org:device:LANDevice:1";
// NTSC-U/C: 0x003b6d4c
static const char *g_pszServiceNamespace = "urn:schemas-upnp-org:service-1-0";
// NTSC-U/C: 0x003b6d50
static const char *g_pszLayer3ForwardingType = "urn:schemas-upnp-org:service:Layer3Forwarding:1";
// NTSC-U/C: 0x003b6d54
static const char *g_pszWanCommonConfigType =
    "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1";
// NTSC-U/C: 0x003b6d58
static const char *g_pszWanPotsConfigType = "urn:schemas-upnp-org:service:WANPOTSLinkConfig:1";
// NTSC-U/C: 0x003b6d5c
static const char *g_pszWanDslConfigType = "urn:schemas-upnp-org:service:WANDSLLinkConfig:1";
// NTSC-U/C: 0x003b6d60
static const char *g_pszWanCableConfigType = "urn:schemas-upnp-org:service:WANCableLinkConfig:1";
// NTSC-U/C: 0x003b6d64
static const char *g_pszWanEthernetConfigType =
    "urn:schemas-upnp-org:service:WANEthernetLinkConfig:1";
// NTSC-U/C: 0x003b6d68
static const char *g_pszWanPppConnectionType = "urn:schemas-upnp-org:service:WANPPPConnection:1";
// NTSC-U/C: 0x003b6d6c
static const char *g_pszWanIpConnectionType = "urn:schemas-upnp-org:service:WANIPConnection:1";
// NTSC-U/C: 0x003b6d70
static const char *g_pszLanHostConfigType =
    "urn:schemas-upnp-org:service:LANHostConfigManagement:1";
// NTSC-U/C: 0x003b6d74
static const char *g_pszSoapEnvelopeNamespace = "http://schemas.xmlsoap.org/soap/envelope/";
// NTSC-U/C: 0x003b6d78
static const char *g_pszSoapEncodingNamespace = "http://schemas.xmlsoap.org/soap/encoding/";
// NTSC-U/C: 0x003b6d7c
static const char *g_pszServiceTypePrefix = "urn:schemas-upnp-org:service:";
// NTSC-U/C: 0x003b6d80
static const char *g_pszControlNamespace = "urn:schemas-upnp-org:control-1-0";

// The elements of a device description the parser reads, in the order of the enumeration.
static const char *const g_apszDescriptionElements[] = {
    "major",        "minor",        "URLBase",         "deviceType",
    "friendlyName", "manufacturer", "manufacturerURL", "modelDescription",
    "modelName",    "modelNumber",  "modelURL",        "serialNumber",
    "UDN",          "UPC",          "serviceType",     "serviceId",
    "controlURL",   "eventSubURL",  "SCPDURL",         NULL,
};

// The elements of a service description the parser reads, in the order of the enumeration.
static const char *const g_apszScpdElements[] = {"major", "minor", "name", NULL};

// The actions of a connection service. An action sets the bit of its index in the result.
static const char *const g_apszConnectionActions[] = {
    "SetConnectionType",
    "GetConnectionTypeInfo",
    "RequestConnection",
    "RequestTermination",
    "ForceTermination",
    "SetAutoDisconnectTime",
    "SetIdleDisconnectTime",
    "SetWarnDisconnectDelay",
    "GetStatusInfo",
    "GetAutoDisconnectTime",
    "GetIdleDisconnectTime",
    "GetWarnDisconnectDelay",
    "GetNATRSIPStatus",
    "GetGenericPortMappingEntry",
    "GetSpecificPortMappingEntry",
    "AddPortMapping",
    "DeletePortMapping",
    "GetExternalIPAddress",
    "ConfigureConnection",
    "GetLinkLayerMaxBitRates",
    "GetPPPEncryptionProtocol",
    "GetPPPCompressionProtocol",
    "GetPPPAuthenticationProtocol",
    "GetUserName",
    "GetPassword",
    NULL,
};

// Skip the namespace prefix of a name. Yes, the binary searches past the end for a name without
// one.
static inline const char *RtXmlSkipPrefix(const char *pszName) {
    while (*pszName != ':') {
        ++pszName;
    }
    return pszName + 1;
}

// Copy text of a known length and terminate it.
static inline void RtXmlCopyText(char *pszDest, const char *pText, int nSize) {
    strncpy(pszDest, pText, nSize);
    pszDest[nSize] = '\0';
}

// NTSC-U/C: 0x002cda50, PAL: 0x002d79c0
static int RtXmlPushTag(char *pszPath, unsigned int *puLength, const char *pszName) {
    unsigned int uSize;

    uSize = strlen(pszName) + 1;
    if (*puLength + uSize >= RT_UPNP_TEXT_SIZE) {
        return 0;
    }
    strncpy(&pszPath[*puLength], pszName, uSize);
    *puLength += uSize;
    return 1;
}

// NTSC-U/C: 0x002cdb68, PAL: 0x002d7ad8
static char *RtXmlLastTag(char *pszPath, unsigned int uLength) {
    char *pszTag;

    if (pszPath == NULL) {
        return NULL;
    }
    if (uLength != 0) {
        --uLength;
    }
    if (uLength == 0) {
        return pszPath;
    }
    pszTag = &pszPath[uLength];
    while (pszTag[-1] != '\0') {
        --pszTag;
        if (--uLength == 0) {
            return pszPath;
        }
    }
    return pszTag;
}

// NTSC-U/C: 0x002cdad0, PAL: 0x002d7a40
static int RtXmlPopTag(char *pszPath, unsigned int *puLength, const char *pszName) {
    const char *pszTag;
    unsigned int uSize;

    pszTag = RtXmlLastTag(pszPath, *puLength);
    uSize = strlen(pszName);
    if (strncmp(pszName, pszTag, uSize) != 0) {
        return 0;
    }
    *puLength = *puLength - uSize - 1;
    pszPath[*puLength] = '\0';
    return 1;
}

// NTSC-U/C: 0x002cdbd0, PAL: 0x002d7b40
static void
RtXmlDescStartElement(void *pUserData, const XML_Char *pszName, const XML_Char **apszAtts) {
    RtUpnpXmlParser *pParser;
    int i;

    pParser = pUserData;
    if (pParser->bFailed) {
        return;
    }
    if (!RtXmlPushTag(pParser->szPath, &pParser->uPathLength, pszName)) {
        pParser->bFailed = 1;
    }
    if (strncmp("root", pszName, RT_UPNP_TEXT_SIZE) == 0) {
        for (i = 0; apszAtts[i] != NULL; ++i) {
            if (i == 0) {
                if (strncmp("xmlns", apszAtts[0], RT_UPNP_TEXT_SIZE) != 0) {
                    pParser->bFailed = 1;
                }
            } else if (i == 1) {
                if (strncmp(g_pszDeviceNamespace, apszAtts[1], RT_UPNP_TEXT_SIZE) != 0) {
                    pParser->bFailed = 1;
                }
            }
        }
        return;
    }
    if (strncmp("specVersion", pszName, RT_UPNP_TEXT_SIZE) == 0) {
        strncpy(pParser->szContext, pszName, RT_UPNP_TEXT_SIZE);
    }
}

// NTSC-U/C: 0x002cdd18, PAL: 0x002d7c88
static void RtXmlDescEndElement(void *pUserData, const XML_Char *pszName) {
    RtUpnpXmlParser *pParser;

    pParser = pUserData;
    if (pParser->bFailed) {
        return;
    }
    if (!RtXmlPopTag(pParser->szPath, &pParser->uPathLength, pszName)) {
        pParser->bFailed = 1;
    }
    if (strncmp("service", pszName, RT_UPNP_TEXT_SIZE) == 0) {
        pParser->szContext[0] = '\0';
    }
    if (strncmp("root", pszName, RT_UPNP_TEXT_SIZE) == 0) {
        pParser->bComplete = 1;
    }
}

// Store text of the connection device in a field of the description.
static inline void
RtXmlStoreDeviceText(RtUpnpXmlParser *pParser, char *pszField, const char *pText, int nSize) {
    if (strncmp(g_pszWanConnectionDeviceType, pParser->szContext, RT_UPNP_TEXT_SIZE) == 0) {
        RtXmlCopyText(pszField, pText, nSize);
    }
}

// Store text of the connection service in a field of the PPP or the IP service of the description.
static inline void RtXmlStoreServiceText(
    RtUpnpXmlParser *pParser, char *pszPppField, char *pszIpField, const char *pText, int nSize) {
    if (strncmp(g_pszWanPppConnectionType, pParser->szContext, RT_UPNP_TEXT_SIZE) == 0) {
        RtXmlCopyText(pszPppField, pText, nSize);
    } else if (strncmp(g_pszWanIpConnectionType, pParser->szContext, RT_UPNP_TEXT_SIZE) == 0) {
        RtXmlCopyText(pszIpField, pText, nSize);
    }
}

// NTSC-U/C: 0x002cddb0, PAL: 0x002d7d20
static void RtXmlDescCharacterData(void *pUserData, const XML_Char *pText, int nSize) {
    const char *apszElements[kDescElementCount + 1];
    RtUpnpXmlParser *pParser;
    RtUpnpDeviceInfo *pInfo;
    const char *pszTag;
    unsigned int i;

    pParser = pUserData;
    memcpy(apszElements, g_apszDescriptionElements, sizeof(apszElements));
    pInfo = pParser->pResult;
    if (pParser->bFailed) {
        return;
    }
    if (nSize <= 0 || isspace((int)pText[0])) {
        return;
    }
    pszTag = RtXmlLastTag(pParser->szPath, pParser->uPathLength);
    for (i = 0; apszElements[i] != NULL; ++i) {
        if (strncmp(apszElements[i], pszTag, RT_UPNP_TEXT_SIZE) == 0) {
            break;
        }
    }
    switch (i) {
    case kDescMajor:
        if (strncmp("specVersion", pParser->szContext, RT_UPNP_TEXT_SIZE) != 0 || pText[0] != '1') {
            pParser->bFailed = 1;
        }
        break;
    case kDescMinor:
        if (strncmp("specVersion", pParser->szContext, RT_UPNP_TEXT_SIZE) != 0 || pText[0] != '0') {
            pParser->bFailed = 1;
        }
        break;
    case kDescUrlBase:
        RtXmlCopyText(pInfo->szUrlBase, pText, nSize);
        break;
    case kDescDeviceType:
        if (strncmp(g_pszGatewayDeviceType, pText, nSize) == 0) {
            pInfo->uFlags |= RT_UPNP_GATEWAY_DEVICE;
        } else if (strncmp(g_pszWanDeviceType, pText, nSize) == 0) {
            pInfo->uFlags |= RT_UPNP_WAN_DEVICE;
        } else if (strncmp(g_pszWanConnectionDeviceType, pText, nSize) == 0) {
            pInfo->uFlags |= RT_UPNP_WAN_CONNECTION_DEVICE;
            RtXmlCopyText(pParser->szContext, pText, nSize);
        } else if (strncmp(g_pszLanDeviceType, pText, nSize) == 0) {
            pInfo->uFlags |= RT_UPNP_LAN_DEVICE;
        }
        break;
    case kDescFriendlyName:
        RtXmlStoreDeviceText(pParser, pInfo->szFriendlyName, pText, nSize);
        break;
    case kDescManufacturer:
        RtXmlStoreDeviceText(pParser, pInfo->szManufacturer, pText, nSize);
        break;
    case kDescManufacturerUrl:
        RtXmlStoreDeviceText(pParser, pInfo->szManufacturerUrl, pText, nSize);
        break;
    case kDescModelDescription:
        RtXmlStoreDeviceText(pParser, pInfo->szModelDescription, pText, nSize);
        break;
    case kDescModelName:
        RtXmlStoreDeviceText(pParser, pInfo->szModelName, pText, nSize);
        break;
    case kDescModelNumber:
        RtXmlStoreDeviceText(pParser, pInfo->szModelNumber, pText, nSize);
        break;
    case kDescModelUrl:
        RtXmlStoreDeviceText(pParser, pInfo->szModelUrl, pText, nSize);
        break;
    case kDescSerialNumber:
        RtXmlStoreDeviceText(pParser, pInfo->szSerialNumber, pText, nSize);
        break;
    case kDescUdn:
        RtXmlStoreDeviceText(pParser, pInfo->szUdn, pText, nSize);
        break;
    case kDescUpc:
        RtXmlStoreDeviceText(pParser, pInfo->szUpc, pText, nSize);
        break;
    case kDescServiceType:
        if (strncmp(g_pszLayer3ForwardingType, pText, nSize) == 0) {
            pInfo->uFlags |= RT_UPNP_LAYER3_FORWARDING;
        } else if (strncmp(g_pszWanCommonConfigType, pText, nSize) == 0) {
            pInfo->uFlags |= RT_UPNP_WAN_COMMON_CONFIG;
        } else if (strncmp(g_pszWanPotsConfigType, pText, nSize) == 0) {
            pInfo->uFlags |= RT_UPNP_WAN_POTS_CONFIG;
        } else if (strncmp(g_pszWanDslConfigType, pText, nSize) == 0) {
            pInfo->uFlags |= RT_UPNP_WAN_DSL_CONFIG;
        } else if (strncmp(g_pszWanCableConfigType, pText, nSize) == 0) {
            pInfo->uFlags |= RT_UPNP_WAN_CABLE_CONFIG;
        } else if (strncmp(g_pszWanEthernetConfigType, pText, nSize) == 0) {
            pInfo->uFlags |= RT_UPNP_WAN_ETHERNET_CONFIG;
        } else if (strncmp(g_pszWanPppConnectionType, pText, nSize) == 0) {
            pInfo->uFlags |= RT_UPNP_WAN_PPP_CONNECTION;
            RtXmlCopyText(pParser->szContext, pText, nSize);
        } else if (strncmp(g_pszWanIpConnectionType, pText, nSize) == 0) {
            pInfo->uFlags |= RT_UPNP_WAN_IP_CONNECTION;
            RtXmlCopyText(pParser->szContext, pText, nSize);
        } else if (strncmp(g_pszLanHostConfigType, pText, nSize) == 0) {
            pInfo->uFlags |= RT_UPNP_LAN_HOST_CONFIG;
        }
        break;
    case kDescServiceId:
        RtXmlStoreServiceText(pParser, pInfo->ppp.szServiceId, pInfo->ip.szServiceId, pText, nSize);
        break;
    case kDescControlUrl:
        RtXmlStoreServiceText(
            pParser, pInfo->ppp.szControlUrl, pInfo->ip.szControlUrl, pText, nSize);
        break;
    case kDescEventSubUrl:
        RtXmlStoreServiceText(
            pParser, pInfo->ppp.szEventSubUrl, pInfo->ip.szEventSubUrl, pText, nSize);
        break;
    case kDescScpdUrl:
        RtXmlStoreServiceText(pParser, pInfo->ppp.szScpdUrl, pInfo->ip.szScpdUrl, pText, nSize);
        break;
    default:
        break;
    }
}

// NTSC-U/C: 0x002ce640, PAL: 0x002d85b0
static void
RtXmlScpdStartElement(void *pUserData, const XML_Char *pszName, const XML_Char **apszAtts) {
    RtUpnpXmlParser *pParser;
    int i;

    pParser = pUserData;
    if (pParser->bFailed) {
        return;
    }
    if (!RtXmlPushTag(pParser->szPath, &pParser->uPathLength, pszName)) {
        pParser->bFailed = 1;
    }
    if (strncmp("scpd", pszName, RT_UPNP_TEXT_SIZE) == 0) {
        for (i = 0; apszAtts[i] != NULL; ++i) {
            if (i == 0) {
                if (strncmp("xmlns", apszAtts[0], RT_UPNP_TEXT_SIZE) != 0) {
                    pParser->bFailed = 1;
                }
            } else if (i == 1) {
                if (strncmp(g_pszServiceNamespace, apszAtts[1], RT_UPNP_TEXT_SIZE) != 0) {
                    pParser->bFailed = 1;
                }
            }
        }
        return;
    }
    if (strncmp("specVersion", pszName, RT_UPNP_TEXT_SIZE) == 0 ||
        strncmp("action", pszName, RT_UPNP_TEXT_SIZE) == 0 ||
        strncmp("argument", pszName, RT_UPNP_TEXT_SIZE) == 0) {
        strncpy(pParser->szContext, pszName, RT_UPNP_TEXT_SIZE);
    }
}

// NTSC-U/C: 0x002ce7d0, PAL: 0x002d8740
static void RtXmlScpdEndElement(void *pUserData, const XML_Char *pszName) {
    RtUpnpXmlParser *pParser;

    pParser = pUserData;
    if (pParser->bFailed) {
        return;
    }
    if (!RtXmlPopTag(pParser->szPath, &pParser->uPathLength, pszName)) {
        pParser->bFailed = 1;
    }
    if (strncmp("scpd", pszName, RT_UPNP_TEXT_SIZE) == 0) {
        pParser->bComplete = 1;
    }
}

// NTSC-U/C: 0x002ce9f0, PAL: 0x002d8960
static void RtXmlAddActionFlag(RtUpnpXmlParser *pParser, const char *pText, int nSize) {
    const char *apszActions[sizeof(g_apszConnectionActions) / sizeof(g_apszConnectionActions[0])];
    unsigned int *puActions;
    unsigned char i;

    memcpy(apszActions, g_apszConnectionActions, sizeof(apszActions));
    puActions = NULL;
    i = 0;
    if (pParser->nMode == RT_UPNP_XML_IP_SCPD || pParser->nMode == RT_UPNP_XML_PPP_SCPD) {
        puActions = pParser->pResult;
    }
    while (apszActions[i] != NULL && strncmp(apszActions[i], pText, nSize) != 0) {
        ++i;
    }
    if (i < sizeof(apszActions) / sizeof(apszActions[0]) - 1) {
        // Yes, the binary writes through a null result for the other modes.
        *puActions |= 1U << i;
    }
}

// NTSC-U/C: 0x002ce850, PAL: 0x002d87c0
static void RtXmlScpdCharacterData(void *pUserData, const XML_Char *pText, int nSize) {
    const char *apszElements[sizeof(g_apszScpdElements) / sizeof(g_apszScpdElements[0])];
    RtUpnpXmlParser *pParser;
    const char *pszTag;
    unsigned int i;

    pParser = pUserData;
    memcpy(apszElements, g_apszScpdElements, sizeof(apszElements));
    if (pParser->bFailed) {
        return;
    }
    if (nSize <= 0 || isspace((int)pText[0])) {
        return;
    }
    pszTag = RtXmlLastTag(pParser->szPath, pParser->uPathLength);
    for (i = 0; apszElements[i] != NULL; ++i) {
        if (strncmp(apszElements[i], pszTag, RT_UPNP_TEXT_SIZE) == 0) {
            break;
        }
    }
    switch (i) {
    case kScpdMajor:
        if (strncmp("specVersion", pParser->szContext, RT_UPNP_TEXT_SIZE) != 0 || pText[0] != '1') {
            pParser->bFailed = 1;
        }
        break;
    case kScpdMinor:
        if (strncmp("specVersion", pParser->szContext, RT_UPNP_TEXT_SIZE) != 0 || pText[0] != '0') {
            pParser->bFailed = 1;
        }
        break;
    case kScpdName:
        if (strncmp("action", pParser->szContext, RT_UPNP_TEXT_SIZE) == 0) {
            RtXmlAddActionFlag(pParser, pText, nSize);
            pParser->szContext[0] = '\0';
        }
        break;
    default:
        break;
    }
}

// Check the attributes of the Envelope element of a SOAP reply.
static inline void RtXmlCheckEnvelope(RtUpnpXmlParser *pParser, const XML_Char **apszAtts) {
    int i;

    for (i = 0; apszAtts[i] != NULL; ++i) {
        switch (i) {
        case kEnvelopeXmlns:
            if (strncmp("xmlns", apszAtts[0], kXmlnsLength) != 0) {
                pParser->bFailed = 1;
            }
            break;
        case kEnvelopeNamespace:
            if (strncmp(g_pszSoapEnvelopeNamespace, apszAtts[1], RT_UPNP_TEXT_SIZE) != 0) {
                pParser->bFailed = 1;
            }
            break;
        case kEnvelopeEncodingStyleName:
            if (strncmp("encodingStyle", RtXmlSkipPrefix(apszAtts[2]), RT_UPNP_TEXT_SIZE) != 0) {
                pParser->bFailed = 1;
            }
            break;
        case kEnvelopeEncodingStyle:
            if (strncmp(g_pszSoapEncodingNamespace, apszAtts[3], RT_UPNP_TEXT_SIZE) != 0) {
                pParser->bFailed = 1;
            }
            break;
        default:
            break;
        }
    }
}

// NTSC-U/C: 0x002cecf0, PAL: 0x002d8c60
static void
RtXmlQueryStartElement(void *pUserData, const XML_Char *pszName, const XML_Char **apszAtts) {
    RtUpnpXmlParser *pParser;
    int i;

    pParser = pUserData;
    if (pParser->bFailed) {
        return;
    }
    if (!RtXmlPushTag(pParser->szPath, &pParser->uPathLength, pszName)) {
        pParser->bFailed = 1;
    }
    if (strncmp("Envelope", RtXmlSkipPrefix(pszName), RT_UPNP_TEXT_SIZE) == 0) {
        RtXmlCheckEnvelope(pParser, apszAtts);
        return;
    }
    // Yes, the binary compares the name with its prefix here.
    if (strncmp("QueryStateVariableResponse", pszName, RT_UPNP_TEXT_SIZE) != 0) {
        return;
    }
    for (i = 0; apszAtts[i] != NULL; ++i) {
        if (i == kReplyXmlns) {
            if (strncmp("xmlns", apszAtts[1], kXmlnsLength) != 0) {
                pParser->bFailed = 1;
            }
        } else if (i == kReplyNamespace) {
            if (strncmp(g_pszControlNamespace, apszAtts[2], RT_UPNP_TEXT_SIZE) != 0) {
                pParser->bFailed = 1;
            }
        }
    }
}

// NTSC-U/C: 0x002cef60, PAL: 0x002d8ed0
static void RtXmlQueryEndElement(void *pUserData, const XML_Char *pszName) {
    RtUpnpXmlParser *pParser;

    pParser = pUserData;
    if (pParser->bFailed) {
        return;
    }
    if (!RtXmlPopTag(pParser->szPath, &pParser->uPathLength, pszName)) {
        pParser->bFailed = 1;
    }
    // Yes, the binary waits for a root element a SOAP reply does not have.
    if (strncmp("root", pszName, RT_UPNP_TEXT_SIZE) == 0) {
        pParser->bComplete = 1;
    }
}

// NTSC-U/C: 0x002cefe0, PAL: 0x002d8f50
static void RtXmlQueryCharacterData(void *pUserData, const XML_Char *pText, int nSize) {
    RtUpnpXmlParser *pParser;
    char *pszValue;

    pParser = pUserData;
    pszValue = pParser->pResult;
    if (pParser->bFailed) {
        return;
    }
    if (nSize <= 0 || isspace((int)pText[0])) {
        return;
    }
    if (strncmp(RtXmlLastTag(pParser->szPath, pParser->uPathLength), "return", RT_UPNP_TEXT_SIZE) ==
        0) {
        strncpy(pszValue, pText, nSize);
    }
}

// NTSC-U/C: 0x002cf080, PAL: 0x002d8ff0
static void
RtXmlActionStartElement(void *pUserData, const XML_Char *pszName, const XML_Char **apszAtts) {
    RtUpnpXmlParser *pParser;
    int i;

    pParser = pUserData;
    if (pParser->bFailed) {
        return;
    }
    if (!RtXmlPushTag(pParser->szPath, &pParser->uPathLength, pszName)) {
        pParser->bFailed = 1;
    }
    if (strncmp("Envelope", RtXmlSkipPrefix(pszName), RT_UPNP_TEXT_SIZE) == 0) {
        RtXmlCheckEnvelope(pParser, apszAtts);
        return;
    }
    // Yes, the binary searches the name for "Response" with the arguments swapped.
    if (strstr("Response", pszName) == NULL) {
        return;
    }
    strncpy(pParser->szContext, pszName, RT_UPNP_TEXT_SIZE);
    for (i = 0; apszAtts[i] != NULL; ++i) {
        if (i == kReplyXmlns) {
            if (strncmp("xmlns", apszAtts[1], kXmlnsLength) != 0) {
                pParser->bFailed = 1;
            }
        } else if (i == kReplyNamespace) {
            if (strncmp(g_pszServiceTypePrefix, apszAtts[2], strlen(g_pszServiceTypePrefix)) != 0) {
                pParser->bFailed = 1;
            }
        }
    }
}

// NTSC-U/C: 0x002cf308, PAL: 0x002d9278
static void RtXmlActionEndElement(void *pUserData, const XML_Char *pszName) {
    RtUpnpXmlParser *pParser;

    pParser = pUserData;
    if (pParser->bFailed) {
        return;
    }
    if (!RtXmlPopTag(pParser->szPath, &pParser->uPathLength, pszName)) {
        pParser->bFailed = 1;
    }
    if (strncmp("root", pszName, RT_UPNP_TEXT_SIZE) == 0) {
        pParser->bComplete = 1;
    }
    if (strstr("Response", pszName) != NULL) {
        pParser->szContext[0] = '\n'; // Yes, the binary stores a line feed here.
    }
}

// NTSC-U/C: 0x002cf3a0, PAL: 0x002d9310
static void RtXmlActionCharacterData(void *pUserData, const XML_Char *pText, int nSize) {
    RtUpnpXmlParser *pParser;
    RtUpnpActionReply *pReply;
    const char *pszTag;

    pParser = pUserData;
    pReply = pParser->pResult;
    if (pParser->bFailed) {
        return;
    }
    if (nSize <= 0 || isspace((int)pText[0])) {
        return;
    }
    pszTag = RtXmlLastTag(pParser->szPath, pParser->uPathLength);
    if (strstr("Response", pParser->szContext) == NULL) {
        return;
    }
    strncpy(pReply->aszNames[pReply->uCount], pszTag, RT_UPNP_TEXT_SIZE);
    RtXmlCopyText(pReply->aszValues[pReply->uCount], pText, nSize);
    ++pReply->uCount;
}

// Allocate the result of a parser and attach its handlers.
static inline int RtXmlParserSetUp(RtUpnpXmlParser *pParser,
                                   int nResultSize,
                                   XML_StartElementHandler pfnStart,
                                   XML_EndElementHandler pfnEnd,
                                   XML_CharacterDataHandler pfnText) {
    if (RtAllocZeroedBlock(&pParser->pResult, nResultSize) != 0) {
        return kXmlResultNoMemory;
    }
    XML_SetUserData(pParser->parser, pParser);
    XML_SetElementHandler(pParser->parser, pfnStart, pfnEnd);
    XML_SetCharacterDataHandler(pParser->parser, pfnText);
    return 0;
}

int RtXmlParserCreate(RtUpnpXmlParser **ppParser, int nMode) {
    RtUpnpXmlParser *pParser;
    int nResult;

    pParser = NULL;
    nResult = 0;
    if (ppParser == NULL) {
        return kXmlResultInvalid;
    }
    if (RtAllocZeroedBlock((void **)&pParser, sizeof(RtUpnpXmlParser)) != 0) {
        nResult = kXmlResultNoMemory;
    } else {
        pParser->parser = XML_ParserCreate(NULL);
        if (pParser->parser == NULL) {
            nResult = kXmlResultNoMemory;
        } else {
            pParser->nMode = nMode;
            if (nMode >= kXmlModeCount) {
                nResult = kXmlResultBadMode;
            } else {
                switch (nMode) {
                case RT_UPNP_XML_DESCRIPTION:
                    nResult = RtXmlParserSetUp(pParser,
                                               sizeof(RtUpnpDeviceInfo),
                                               RtXmlDescStartElement,
                                               RtXmlDescEndElement,
                                               RtXmlDescCharacterData);
                    break;
                case RT_UPNP_XML_PPP_SCPD:
                case RT_UPNP_XML_IP_SCPD:
                    nResult = RtXmlParserSetUp(pParser,
                                               sizeof(unsigned int),
                                               RtXmlScpdStartElement,
                                               RtXmlScpdEndElement,
                                               RtXmlScpdCharacterData);
                    break;
                case RT_UPNP_XML_QUERY:
                    nResult = RtXmlParserSetUp(pParser,
                                               RT_UPNP_TEXT_SIZE,
                                               RtXmlQueryStartElement,
                                               RtXmlQueryEndElement,
                                               RtXmlQueryCharacterData);
                    break;
                case RT_UPNP_XML_ACTION:
                    nResult = RtXmlParserSetUp(pParser,
                                               sizeof(RtUpnpActionReply),
                                               RtXmlActionStartElement,
                                               RtXmlActionEndElement,
                                               RtXmlActionCharacterData);
                    break;
                default:
                    nResult = kXmlResultBadMode;
                    break;
                }
            }
            if (nResult == 0) {
                *ppParser = pParser;
                return 0;
            }
        }
    }
    RtXmlParserDestroy(pParser);
    return nResult;
}

int RtXmlParserDestroy(RtUpnpXmlParser *pParser) {
    if (pParser == NULL) {
        return kXmlResultInvalid;
    }
    if (pParser->pResult != NULL) {
        RtFreeBlock(&pParser->pResult);
    }
    if (pParser->parser != NULL) {
        XML_ParserFree(pParser->parser);
    }
    RtFreeBlock((void **)&pParser);
    return 0;
}

int RtXmlParserFeed(RtUpnpXmlParser *pParser, const char *pData, int nSize, unsigned char bFinal) {
    if (pParser == NULL) {
        return kXmlResultInvalid;
    }
    return XML_Parse(pParser->parser, pData, nSize, bFinal) == 0;
}

int RtXmlParserTakeComplete(RtUpnpXmlParser *pParser, unsigned char *pbComplete) {
    if (pParser == NULL || pbComplete == NULL) {
        return kXmlResultInvalid;
    }
    *pbComplete = pParser->bComplete;
    pParser->bComplete = 0;
    return 0;
}

int RtXmlParserGetResult(RtUpnpXmlParser *pParser, void **ppResult) {
    if (pParser == NULL) {
        return kXmlResultInvalid;
    }
    *ppResult = pParser->pResult;
    return 0;
}

void *RtXmlMalloc(size_t uSize) {
    void *pBlock;
    int nResult;

    pBlock = NULL;
    nResult = RtAllocBlock(&pBlock, uSize);
    if (pBlock == NULL || nResult != 0) {
        return NULL;
    }
    return pBlock;
}

void *RtXmlCalloc(size_t uCount, size_t uSize) {
    void *pBlock;
    int nResult;

    pBlock = NULL;
    nResult = RtAllocZeroedBlock(&pBlock, uCount * uSize);
    if (pBlock == NULL || nResult != 0) {
        return NULL;
    }
    return pBlock;
}

void *RtXmlRealloc(void *pBlock, size_t uSize) {
    void *pNewBlock;
    int nResult;

    pNewBlock = NULL;
    nResult = RtAllocBlock(&pNewBlock, uSize);
    if (pNewBlock == NULL || nResult != 0) {
        return pBlock;
    }
    // Yes, the binary copies the new size from the old block, past its end when it grows.
    memcpy(pNewBlock, pBlock, uSize);
    RtFreeBlock(&pBlock);
    return pNewBlock;
}

void RtXmlFree(void *pBlock) {
    RtFreeBlock(&pBlock);
}
