#ifndef RTUPNPXML_H
#define RTUPNPXML_H

#include <stddef.h>

#include <xmlparse.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parsers of the XML replies of a UPnP gateway for the SCE-RT library, over Expat. The routines
 * report 0 for success, 3 when an allocation failed, 5 for a null parser, and 6 for an unknown
 * mode.
 */

/** The size of a name or a value of the replies. */
#define RT_UPNP_TEXT_SIZE 256

/** The most arguments an action reply keeps. */
#define RT_UPNP_MAX_ARGUMENTS 12

/** What a parser reads. */
#define RT_UPNP_XML_DESCRIPTION 0 /*!< The device description, into an RtUpnpDeviceInfo. */
#define RT_UPNP_XML_PPP_SCPD 7    /*!< The actions of WANPPPConnection, into a bit set. */
#define RT_UPNP_XML_IP_SCPD 8     /*!< The actions of WANIPConnection, into a bit set. */
#define RT_UPNP_XML_QUERY 10      /*!< The reply of QueryStateVariable, into a string. */
#define RT_UPNP_XML_ACTION 11 /*!< The arguments of an action reply, into an RtUpnpActionReply. */

/** Kinds of devices and services in RtUpnpDeviceInfo::uFlags. */
#define RT_UPNP_GATEWAY_DEVICE 0x0001        /*!< InternetGatewayDevice. */
#define RT_UPNP_WAN_DEVICE 0x0002            /*!< WANDevice. */
#define RT_UPNP_WAN_CONNECTION_DEVICE 0x0004 /*!< WANConnectionDevice. */
#define RT_UPNP_LAN_DEVICE 0x0010            /*!< LANDevice. */
#define RT_UPNP_LAYER3_FORWARDING 0x0020     /*!< Layer3Forwarding. */
#define RT_UPNP_WAN_COMMON_CONFIG 0x0040     /*!< WANCommonInterfaceConfig. */
#define RT_UPNP_WAN_POTS_CONFIG 0x0080       /*!< WANPOTSLinkConfig. */
#define RT_UPNP_WAN_DSL_CONFIG 0x0100        /*!< WANDSLLinkConfig. */
#define RT_UPNP_WAN_CABLE_CONFIG 0x0200      /*!< WANCableLinkConfig. */
#define RT_UPNP_WAN_ETHERNET_CONFIG 0x0400   /*!< WANEthernetLinkConfig. */
#define RT_UPNP_WAN_PPP_CONNECTION 0x0800    /*!< WANPPPConnection. */
#define RT_UPNP_WAN_IP_CONNECTION 0x1000     /*!< WANIPConnection. */
#define RT_UPNP_LAN_HOST_CONFIG 0x2000       /*!< LANHostConfigManagement. */

/** The addresses of a connection service of a gateway. */
typedef struct {
    char szServiceId[RT_UPNP_TEXT_SIZE];   /*!< The serviceId. */
    char szScpdUrl[RT_UPNP_TEXT_SIZE];     /*!< The SCPDURL. */
    char szControlUrl[RT_UPNP_TEXT_SIZE];  /*!< The controlURL. */
    char szEventSubUrl[RT_UPNP_TEXT_SIZE]; /*!< The eventSubURL. */
} RtUpnpService;

/** The device description of a gateway. */
typedef struct {
    unsigned int uFlags;                        /*!< The devices and services found. */
    char szUrlBase[RT_UPNP_TEXT_SIZE];          /*!< The URLBase. */
    char szFriendlyName[RT_UPNP_TEXT_SIZE];     /*!< The friendlyName of the connection device. */
    char szManufacturer[RT_UPNP_TEXT_SIZE];     /*!< The manufacturer. */
    char szManufacturerUrl[RT_UPNP_TEXT_SIZE];  /*!< The manufacturerURL. */
    char szModelDescription[RT_UPNP_TEXT_SIZE]; /*!< The modelDescription. */
    char szModelName[RT_UPNP_TEXT_SIZE];        /*!< The modelName. */
    char szModelNumber[RT_UPNP_TEXT_SIZE];      /*!< The modelNumber. */
    char szModelUrl[RT_UPNP_TEXT_SIZE];         /*!< The modelURL. */
    char szSerialNumber[RT_UPNP_TEXT_SIZE];     /*!< The serialNumber. */
    char szUdn[RT_UPNP_TEXT_SIZE];              /*!< The UDN. */
    char szUpc[RT_UPNP_TEXT_SIZE];              /*!< The UPC. */
    RtUpnpService ppp;                          /*!< The WANPPPConnection service. */
    RtUpnpService ip;                           /*!< The WANIPConnection service. */
} RtUpnpDeviceInfo;

/** The arguments of an action reply. */
typedef struct {
    char aszNames[RT_UPNP_MAX_ARGUMENTS][RT_UPNP_TEXT_SIZE];  /*!< The names. */
    char aszValues[RT_UPNP_MAX_ARGUMENTS][RT_UPNP_TEXT_SIZE]; /*!< The values. */
    unsigned int uCount;                                      /*!< The arguments read. */
} RtUpnpActionReply;

/** A parser of one reply. */
typedef struct {
    XML_Parser parser;                 /*!< The Expat parser. */
    int nMode;                         /*!< What the parser reads, an RT_UPNP_XML_ value. */
    char szContext[RT_UPNP_TEXT_SIZE]; /*!< The element or the device the text belongs to. */
    char szPath[RT_UPNP_TEXT_SIZE];    /*!< The names of the open elements, each terminated. */
    unsigned int uPathLength;          /*!< The bytes of szPath in use. */
    unsigned char bFailed;             /*!< Non-zero once the reply proved invalid. */
    unsigned char bComplete;           /*!< Non-zero once the root element closed. */
    void *pResult;                     /*!< What the parser fills, of the type its mode reads. */
} RtUpnpXmlParser;

/**
 * Create a parser.
 *
 * @param ppParser Receives the parser.
 * @param nMode What the parser reads, an RT_UPNP_XML_ value.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002cf490
 * @ghidraAddress PAL: 0x002d9400
 */
int RtXmlParserCreate(RtUpnpXmlParser **ppParser, int nMode);

/**
 * Destroy a parser and what it filled.
 *
 * @param pParser The parser.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002cf708
 * @ghidraAddress PAL: 0x002d9678
 */
int RtXmlParserDestroy(RtUpnpXmlParser *pParser);

/**
 * Parse more of a reply.
 *
 * @param pParser The parser.
 * @param pData The bytes.
 * @param nSize The number of bytes.
 * @param bFinal Non-zero for the last bytes of the reply.
 * @return 0, 1 when Expat reported an error, or 5 for a null parser.
 * @ghidraAddress NTSC-U/C: 0x002cf768
 * @ghidraAddress PAL: 0x002d96d8
 */
int RtXmlParserFeed(RtUpnpXmlParser *pParser, const char *pData, int nSize, unsigned char bFinal);

/**
 * Report whether the root element closed, and clear the report.
 *
 * @param pParser The parser.
 * @param pbComplete Receives non-zero when the root element closed.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002cf798
 * @ghidraAddress PAL: 0x002d9708
 */
int RtXmlParserTakeComplete(RtUpnpXmlParser *pParser, unsigned char *pbComplete);

/**
 * Report what a parser filled.
 *
 * @param pParser The parser.
 * @param ppResult Receives the result, of the type the mode reads.
 * @return 0, or an error.
 * @ghidraAddress NTSC-U/C: 0x002cf7c8
 * @ghidraAddress PAL: 0x002d9738
 */
int RtXmlParserGetResult(RtUpnpXmlParser *pParser, void **ppResult);

/**
 * Allocate memory for Expat from the SCE-RT allocator.
 *
 * @param uSize The size in bytes.
 * @return The memory, or null.
 * @ghidraAddress NTSC-U/C: 0x002cf7e8
 * @ghidraAddress PAL: 0x002d9758
 */
void *RtXmlMalloc(size_t uSize);

/**
 * Allocate zeroed memory for Expat from the SCE-RT allocator.
 *
 * @param uCount The number of elements.
 * @param uSize The size of an element in bytes.
 * @return The memory, or null.
 * @ghidraAddress NTSC-U/C: 0x002cf830
 * @ghidraAddress PAL: 0x002d97a0
 */
void *RtXmlCalloc(size_t uCount, size_t uSize);

/**
 * Resize memory for Expat. The new block takes uSize bytes from the old one.
 *
 * @param pBlock The memory.
 * @param uSize The new size in bytes.
 * @return The new memory, or the old memory when the allocation failed.
 * @ghidraAddress NTSC-U/C: 0x002cf878
 * @ghidraAddress PAL: 0x002d97e8
 */
void *RtXmlRealloc(void *pBlock, size_t uSize);

/**
 * Release memory of Expat.
 *
 * @param pBlock The memory.
 * @ghidraAddress NTSC-U/C: 0x002cf8e0
 * @ghidraAddress PAL: 0x002d9850
 */
void RtXmlFree(void *pBlock);

#ifdef __cplusplus
}
#endif

#endif
