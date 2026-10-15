#include "rt/rtdmedevice.h"

#include <stddef.h>
#include <string.h>

#include <eekernel.h>
#include <librtdme.h>

#include "rt/rtcodec.h"
#include "rt/rtimage.h"
#include "rt/rtmemory.h"
#include "rt/rtreassembler.h"
#include "rt/rtsocket.h"
#include "rt/rtupnp.h"

// Bits of rtDmeDevice::flags.
enum {
    kDeviceFlagInitialized = 0x1,
    // Every known peer answered and none is unresolved.
    kDeviceFlagPeersResolved = 0x2,
    // A peer of the active connection answered recently.
    kDeviceFlagConnectionAlive = 0x4,
    kDeviceFlagTalkAllowed = 0x8,
};

// Values of rtDmeSlotTable::state.
enum {
    kSlotTablePlaying = 0,
    kSlotTableFilling = 1,
    // A fragment was dropped, and the table skips the missing fragments.
    kSlotTableSkipping = 2,
};

enum {
    // The peer slots of a connection that takes any number of peers, before it grows.
    kDefaultSlotCount = 32,
    kSlotGrowth = 2,
    kSlotRetries = 16,

    // Times in milliseconds.
    kSlotTimeout = 1500,
    kTalkTimeout = 1500,
    kSelectRetryInterval = 2000,
    kOfferDelay = 2000,

    // This console may talk over another peer while the connection has fewer peers.
    kTalkPeerLimit = 3,

    kVideoFragmentSize = 504,
    // Bits of the frame sequence number of a video record.
    kFrameFirst = 0x20000000,
    kFrameLast = 0x40000000,
    // A whole frame up to this many frames older than the expected one is a repeat.
    kVideoSequenceWindow = 16,

    kFragmentMask = RT_DME_AUDIO_FRAGMENT_COUNT - 1,
    kFragmentWindow = RT_DME_AUDIO_FRAGMENT_COUNT,
    kFragmentDropWindow = 2 * RT_DME_AUDIO_FRAGMENT_COUNT,
    // Fragments a filling table buffers before playback starts.
    kPrebufferFragments = 5,
    // Fragments below which a skipping table plays again.
    kRecoverFragments = 48,

    kRandMultiplier = 214013,
    kRandIncrement = 2531011,
    kRandShift = 16,
    kRandMask = 0x7fff,

    // The rate of the cycle counter is 294.912 MHz.
    kCyclesPerMillisecond = 294912,
};

enum {
    // The maximum peers of the device connection and of the connections peers create.
    kDefaultMaxPeers = 3,

    kWireHeaderSize = sizeof(rtDmeWireHeader),
    // Audio and video records add a frame sequence number after the header.
    kStreamHeaderSize = kWireHeaderSize + sizeof(unsigned int),
    kMaxDatagramSize = 0x500,
    kRecordBufferSize = 0x800,
    kReceiveSize = 0x1000,
    kVideoBufferSize = 0x7e00,
    kHistoryMask = RT_DME_RECORD_HISTORY_SIZE - 1,

    // Times in milliseconds a peer may stay silent before the device pings it.
    kResolvedPeerTimeout = 1500,
    kUnresolvedPeerTimeout = 200,
    // Pings a peer may miss before the device removes it.
    kResolvedPeerRetries = 30,
    kUnresolvedPeerRetries = 100,

    // The device captures video only after less captured audio than this many bytes.
    kVideoAudioBudget = 8000,
    kPortMappingPollDelay = 20,

    kSocketType = 1,
    kSocketBufferSize = 0x500,

    // Uncompressed audio goes out in frames of this many bytes.
    kPcmFrameBytes = 0xf0,
    // The encoders take two halves of a frame at a time.
    kHalvesPerFrame = 2,
};

// NTSC-U/C: 0x003b6e5c, PAL: 0x00435d1c
static unsigned int g_nProfileTimerBase;

// NTSC-U/C: 0x004357d8
static unsigned int g_dwDeviceBytesUnsent;

// NTSC-U/C: 0x004357dc
static unsigned int g_dwDeviceBytesSent;

// NTSC-U/C: 0x004357e0
static unsigned int g_dwDeviceBytesRecv;

// NTSC-U/C: 0x003b6e60, PAL: 0x00435d20
static unsigned int g_dwRandSeed;

// NTSC-U/C: 0x003b6e64, PAL: 0x00435d24
static unsigned int g_nMonotonicTickCount = 1;

// NTSC-U/C: 0x003b6e68, PAL: 0x00435d28
static unsigned int g_nTickClockBase;

// A wire record as a connection sends it.
typedef struct {
    rtDmeWireHeader header;
    unsigned char payload[kRecordBufferSize - kWireHeaderSize];
} rtDmeRecordBuffer;

static unsigned int rtDmeGetTime(void);
static void rtDmeSleep(int ticks);
static int rtDmeDeviceSendVideoFrame(rtDmeDevice *device, const unsigned char *frame, int length);
static rtDmeConnection *rtDmeDeviceGetActiveConnection(rtDmeDevice *device);
static rtDmeConnection *rtDmeDeviceGetSelectedConnection(rtDmeDevice *device);
static void rtDmeDeviceOnPortMapped(RtUpnpContext *context,
                                    int status,
                                    int service,
                                    const char *address,
                                    unsigned short port,
                                    void *owner);
static rtDmeConnection *rtDmeConnectionCreate(rtDmeDevice *device, int id, int maxPeers);
static void rtDmeConnectionDestroy(rtDmeConnection *connection);
static unsigned int rtDmeConnectionNextSequence(rtDmeConnection *connection);
static int rtDmeConnectionSend(rtDmeConnection *connection,
                               int senderId,
                               const rtDmeWireHeader *header,
                               const void *payload,
                               int length);
static void rtDmeConnectionInitHeader(rtDmeConnection *connection,
                                      rtDmeWireHeader *header,
                                      int type,
                                      int length);
static int rtDmeConnectionUpdate(rtDmeConnection *connection);
static int rtDmeConnectionHandleAudioRecord(rtDmeConnection *connection,
                                            const rtDmeWireHeader *header,
                                            rtDmePeer *peer,
                                            const unsigned char *record,
                                            int length);
static int rtDmeConnectionHandleVideoRecord(rtDmeConnection *connection,
                                            const rtDmeWireHeader *header,
                                            rtDmePeer *peer,
                                            const unsigned char *record,
                                            int length);
static int rtDmeConnectionHandleControlRecord(rtDmeConnection *connection,
                                              const rtDmeWireHeader *header,
                                              rtDmePeer *peer,
                                              const unsigned char *record,
                                              int length);
static int rtDmeConnectionCheckTalk(rtDmeConnection *connection);
static int rtDmeConnectionSendSlotsFree(rtDmeConnection *connection, int peerId, int force);
static rtDmeConnectionSlot *rtDmeConnectionFindSlot(rtDmeConnection *connection, int peerId);
static int rtDmeConnectionSendSlotReleased(rtDmeConnection *connection);
static void rtDmeConnectionDisconnect(rtDmeConnection *connection);
static void rtDmeConnectionResetSlots(rtDmeConnection *connection);
static void rtDmeConnectionAddPeer(rtDmeConnection *connection, rtDmePeer *peer);
static void rtDmeConnectionRemovePeer(rtDmeConnection *connection, int peerId);
static void rtDmeConnectionGetTalker(rtDmeConnection *connection, rtDmePeerInfo *info);
static void rtDmePeerSetConnection(rtDmePeer *peer, int connectionId);
static rtDmePeer *rtDmePeerListAdd(rtDmePeerList *list, const rtDmePeerInfo *info);
static void rtDmePeerListRemove(rtDmePeerList *list, int id);
static rtDmePeer *rtDmePeerListFind(rtDmePeerList *list, int id);
static void rtDmePeerListClear(rtDmePeerList *list);
static void rtDmeEndpointListAdd(rtDmeEndpointList *list, rtDmePeer *peer);
static int rtDmeEndpointListCount(const rtDmeEndpointList *list);
static void rtDmeEndpointListRemove(rtDmeEndpointList *list, int id);
static void rtDmeEndpointListClear(rtDmeEndpointList *list);
static rtDmeSlotTable *rtDmeSlotTableCreate(rtDmeDevice *device);
static void rtDmeSlotTableDestroy(rtDmeSlotTable *table);
static void rtDmeSlotTableStore(rtDmeSlotTable *table,
                                int codec,
                                int senderId,
                                int sequence,
                                const unsigned char *data,
                                int length);
static int rtDmeSlotTableRead(rtDmeSlotTable *table, int *codec, void *buffer, int size);
static rtSubbandCodec *rtDmeSubbandCodecCreate(void);
static void rtDmeSubbandCodecDestroy(rtSubbandCodec *codec);
static void rtDmeSubbandCodecReset(rtSubbandCodec *codec);

// NTSC-U/C: 0x002e4584, PAL: 0x002ee4f4
static void rtDmeStartProfileTimer(void) {
    g_nProfileTimerBase = rtDmeGetTime();
}

// NTSC-U/C: 0x002e45a8, PAL: 0x002ee518
static unsigned int rtDmeGetProfileTimerElapsed(void) {
    return rtDmeGetTime() - g_nProfileTimerBase;
}

// NTSC-U/C: 0x002e45d0, PAL: 0x002ee540
static int rtDmeDeviceCaptureVideo(rtDmeDevice *device, unsigned char *bitstream, int size) {
    int length = 0;

    (void)size; // Yes, the binary ignores the size of the buffer.
    if (device->captureVideo == NULL) {
        return length;
    }
    if (device->captureVideo(device, device->captureY, device->captureU, device->captureV) <= 0) {
        return length;
    }
    if (device->activeConnectionId <= 0) {
        return length;
    }
    rtDmeStartProfileTimer();
    length = rtImageEncode(device->width,
                           device->height,
                           device->captureY,
                           device->captureU,
                           device->captureV,
                           bitstream);
    device->videoEncodeTime += rtDmeGetProfileTimerElapsed();
    ++device->videoFramesEncoded;
    return length;
}

// NTSC-U/C: 0x002e46c4, PAL: 0x002ee634
static int rtDmeDeviceDisplayPeerVideo(rtDmeDevice *device, rtDmePeer *peer) {
    unsigned char buffer[kVideoBufferSize];
    const unsigned char *frame;
    int length;

    length = peer->frameLength;
    frame = peer->frame;
    if (length != 0) {
        peer->frameLength = 0;
    } else if (peer->videoFragments != NULL) {
        frame = buffer;
        length = rtReassemblerRead(peer->videoFragments, buffer, kVideoBufferSize);
    }
    if (length == 0) {
        return 0;
    }
    if (device->displayVideo == NULL) {
        return 0;
    }
    if (peer->surface == NULL) {
        peer->surface = rtImageCreateSurface(device->width, device->height);
    }
    if (length != 0) { // Yes, the binary tests the length again.
        rtDmeStartProfileTimer();
        rtImageDecode(device->width,
                      device->height,
                      device->displayY,
                      device->displayU,
                      device->displayV,
                      frame);
        device->videoDecodeTime += rtDmeGetProfileTimerElapsed();
        ++device->videoFramesDecoded;
        rtImageConvertToRgb(device->displayY, device->displayU, device->displayV, peer->surface);
    } else {
        memset(peer->surface->pixels, 0, peer->surface->size);
    }
    device->displayVideo(device, peer->info.id, peer->surface);
    return 0;
}

rtDmeDevice *rtDmeDeviceCreate(void) {
    void *block = NULL;
    rtDmeDevice *device;

    RtAllocZeroedBlock(&block, sizeof(rtDmeDevice));
    device = block;
    if (device != NULL) {
        memset(device, 0, sizeof(rtDmeDevice));
        device->audioCodec = 0;
        device->upnpState = RT_DME_UPNP_IDLE;
        device->upnp = NULL;
        device->onPortMapped = NULL;
    }
    return device;
}

// NTSC-U/C: 0x002e5e1c, PAL: 0x002efd8c
static void rtDmeDeviceDestroySpeechCodecs(rtDmeDevice *device) {
    if (device->speechCodecs[0] != NULL) {
        rtSpeechDestroy(device->speechCodecs[0]);
        device->speechCodecs[0] = NULL;
    }
    if (device->speechCodecs[1] != NULL) {
        rtSpeechDestroy(device->speechCodecs[1]);
        device->speechCodecs[1] = NULL;
    }
}

// NTSC-U/C: 0x002e5e6c, PAL: 0x002efddc
static void rtDmeDeviceDestroySubbandCodecs(rtDmeDevice *device) {
    if (device->subbandEncoder != NULL) {
        rtDmeSubbandCodecDestroy(device->subbandEncoder);
        device->subbandEncoder = NULL;
    }
    if (device->subbandDecoder != NULL) {
        rtDmeSubbandCodecDestroy(device->subbandDecoder);
        device->subbandDecoder = NULL;
    }
}

// NTSC-U/C: 0x002e5de4, PAL: 0x002efd54
static rtDmeConnection *rtDmeDeviceRemoveConnection(rtDmeDevice *device,
                                                    rtDmeConnection *connection) {
    if (connection == NULL) {
        return connection;
    }
    if (connection == device->connections) {
        // Yes, the binary drops the whole list when the first connection goes.
        device->connections = NULL;
        return connection;
    }
    connection->prev->next = connection->next;
    connection->next->prev = connection->prev;
    return connection;
}

// NTSC-U/C: 0x002e4c30, PAL: 0x002eeba0
static void rtDmeDeviceFree(rtDmeDevice *device) {
    void *block = device;

    if (device == NULL) {
        return;
    }
    while (device->connections != NULL) {
        rtDmeConnectionDestroy(rtDmeDeviceRemoveConnection(device, device->connections->prev));
    }
    rtDmePeerListClear(&device->peers);
    if (device->socket != NULL) {
        RtSocketClose(device->socket);
        device->socket = NULL;
    }
    RtSocketShutdown();
    rtDmeSlotTableDestroy(device->slotTable);
    device->slotTable = NULL;
    if (device->captureY != NULL) {
        rtImageDestroy(device->captureY);
        device->captureY = NULL;
    }
    if (device->captureU != NULL) {
        rtImageDestroy(device->captureU);
        device->captureU = NULL;
    }
    if (device->captureV != NULL) {
        rtImageDestroy(device->captureV);
        device->captureV = NULL;
    }
    if (device->displayY != NULL) {
        rtImageDestroy(device->displayY);
        device->displayY = NULL;
    }
    if (device->displayU != NULL) {
        rtImageDestroy(device->displayU);
        device->displayU = NULL;
    }
    if (device->displayV != NULL) {
        rtImageDestroy(device->displayV);
        device->displayV = NULL;
    }
    if (device->upnp != NULL) {
        RtUpnpDestroyContext(device->upnp);
        device->upnp = NULL;
    }
    device->flags = 0;
    RtFreeBlock(&block);
}

void rtDmeDeviceDestroy(rtDmeDevice *device) {
    rtDmeDeviceDestroySpeechCodecs(device);
    rtDmeDeviceDestroySubbandCodecs(device);
    rtDmeDeviceFree(device);
}

void rtDmeDeviceResetAudioSequence(rtDmeDevice *device) {
    device->audioSequence = 0;
}

void rtDmeDeviceResetVideoSequence(rtDmeDevice *device) {
    device->videoSequence = 0;
}

unsigned int rtDmeDeviceNextAudioSequence(rtDmeDevice *device) {
    return device->audioSequence++;
}

unsigned int rtDmeDeviceNextVideoSequence(rtDmeDevice *device) {
    return device->videoSequence++;
}

int rtDmeDeviceInit(rtDmeDevice *device, const rtDmeDeviceParams *params) {
    char localAddress[RT_NET_ADDRESS_SIZE];
    RtSocketOptions options;
    rtDmePeerInfo info;

    if (device->flags & kDeviceFlagInitialized) {
        return 1;
    }
    rtDmeDeviceAddConnection(device, 0, kDefaultMaxPeers);
    device->reserved08 = params->reserved08;
    device->reserved0C = params->reserved14;
    device->reserved18 = params->reserved04;
    device->reserved1C = params->reserved10;
    device->reserved10 = params->reserved0C;
    device->reserved14 = params->reserved18;
    device->captureAudio = params->captureAudio;
    device->playAudio = params->playAudio;
    device->captureVideo = params->captureVideo;
    device->displayVideo = params->displayVideo;
    device->width = params->width;
    device->height = params->height;
    if (device->captureVideo != NULL) {
        device->captureY = rtImageCreate(device->width, device->height);
        device->captureU = rtImageCreate(device->width / 2, device->height / 2);
        device->captureV = rtImageCreate(device->width / 2, device->height / 2);
    }
    if (device->displayVideo != NULL) {
        device->displayY = rtImageCreate(device->width, device->height);
        device->displayU = rtImageCreate(device->width / 2, device->height / 2);
        device->displayV = rtImageCreate(device->width / 2, device->height / 2);
    }
    if (RtSocketStartup() != 0) {
        return -1;
    }
    options.nType = kSocketType;
    options.nBufferSize = kSocketBufferSize;
    localAddress[0] = '\0';
    RtSocketGetLocalAddress(localAddress);
    device->localAddress = rtNetParseAddress(localAddress);
    device->localPort = params->port;
    if (RtSocketCreate(&device->socket, NULL, (unsigned short)device->localPort, &options) != 0) {
        return -1;
    }
    memset(&info, 0, sizeof(info));
    info.id = device->localId;
    info.localAddress = device->localAddress;
    info.localPort = device->localPort;
    device->localPeer = rtDmeDeviceAddPeer(device, &info);
    rtDmeConnectionAddPeer(device->connections, device->localPeer);
    device->subbandEncoder = rtDmeSubbandCodecCreate();
    device->subbandDecoder = rtDmeSubbandCodecCreate();
    device->slotTable = rtDmeSlotTableCreate(device);
    rtLpcReset();
    rtSpeechInit();
    device->flags |= kDeviceFlagInitialized;
    return 0;
}

int rtDmeDeviceMapPort(rtDmeDevice *device,
                       const char *host,
                       unsigned short port,
                       rtDmePortMappedFunc onPortMapped) {
    if (device == NULL) {
        return -1;
    }
    if (host == NULL) {
        return -1;
    }
    if (strlen(host) == 0) {
        return -1;
    }
    if (port == 0) {
        return -1;
    }
    strncpy(device->upnpHost, host, RT_DME_UPNP_HOST_SIZE);
    device->upnpPort = port;
    if (RtUpnpCreateContext(
            &device->upnp, device->socket, (unsigned short)device->localPort, device) != 0) {
        return -1;
    }
    device->onPortMapped = onPortMapped;
    device->upnpState = RT_DME_UPNP_PENDING;
    if (RtUpnpBeginMapping(
            device->upnp, device->upnpHost, device->upnpPort, rtDmeDeviceOnPortMapped) != 0) {
        device->upnpState = RT_DME_UPNP_IDLE;
        return -1;
    }
    while (device->upnpState == RT_DME_UPNP_PENDING && device->onPortMapped == NULL) {
        rtDmeSleep(kPortMappingPollDelay);
        rtDmeDeviceUpdate(device);
    }
    return 0;
}

void rtDmeDeviceStop(rtDmeDevice *device) {
    if (device->activeConnectionId > 0) {
        rtDmeDeviceDeselectConnection(device, device->activeConnectionId);
    }
    if (device->connections != NULL) {
        rtDmeConnectionDisconnect(device->connections);
    }
    device->flags &= ~kDeviceFlagPeersResolved;
}

// NTSC-U/C: 0x002e5c74, PAL: 0x002efbe4
static int
rtDmeDeviceReceive(rtDmeDevice *device, void *buffer, int size, unsigned int *address, int *port) {
    char text[RT_NET_ADDRESS_SIZE];
    unsigned short received = 0;

    *address = 0;
    *port = 0;
    // Yes, the binary has the receive write a 16-bit port into the low half of the int.
    if (RtSocketReceiveFrom(device->socket,
                            text,
                            (unsigned short *)port,
                            buffer,
                            (unsigned short)size,
                            &received) != 0) {
        return -1;
    }
    if (received == 0) {
        return 0;
    }
    if (device->upnpPort == (unsigned short)*port && strcmp(text, device->upnpHost) == 0) {
        return 0;
    }
    g_dwDeviceBytesRecv += received;
    *address = rtNetParseAddress(text);
    return received;
}

// NTSC-U/C: 0x002e5abc, PAL: 0x002efa2c
static int rtDmeDeviceSendDatagram(
    rtDmeDevice *device, const void *data, int length, unsigned int address, int port) {
    char text[RT_NET_ADDRESS_SIZE];
    unsigned short sent = 0;

    rtNetFormatAddress(address, text);
    if (RtSocketSendTo(
            device->socket, text, (unsigned short)port, data, (unsigned short)length, &sent) != 0) {
        g_dwDeviceBytesUnsent += length;
        return -1;
    }
    g_dwDeviceBytesSent += sent;
    g_dwDeviceBytesUnsent += length - sent;
    return sent;
}

int rtDmeDeviceSendRecord(rtDmeDevice *device,
                          int peerId,
                          const rtDmeWireHeader *header,
                          const void *payload,
                          unsigned int length) {
    unsigned char record[kRecordBufferSize];
    rtDmePeer *peer;
    unsigned int total;
    int result = 0;

    peer = rtDmePeerListFind(&device->peers, peerId);
    if (length + kWireHeaderSize > kMaxDatagramSize) {
        return -1;
    }
    if (length + kWireHeaderSize > kRecordBufferSize) { // Yes, the binary tests both limits.
        return -1;
    }
    if (peer == NULL) {
        return -1;
    }
    memcpy(record, header, kWireHeaderSize);
    memcpy(&record[kWireHeaderSize], payload, length);
    total = length + kWireHeaderSize;
    if (peer->info.localAddress != 0 && peer->state < RT_DME_PEER_PUBLIC) {
        if (peer->info.localPort == 0) {
            return -1;
        }
        if (rtDmeDeviceSendDatagram(
                device, record, total, peer->info.localAddress, peer->info.localPort) !=
            (int)total) {
            result = -1;
        }
    }
    if (peer->info.publicAddress == 0) {
        return result;
    }
    if (peer->state != RT_DME_PEER_PUBLIC && peer->state != RT_DME_PEER_UNRESOLVED) {
        return result;
    }
    if (peer->info.publicPort == 0) {
        return -1;
    }
    if (rtDmeDeviceSendDatagram(
            device, record, total, peer->info.publicAddress, peer->info.publicPort) != (int)total) {
        result = -1;
    }
    return result;
}

int rtDmeDeviceSendToPeer(rtDmeDevice *device,
                          const void *record,
                          int length,
                          const rtDmePeer *peer) {
    unsigned int state = peer->state;
    int result = length;
    int sent;

    if (peer->info.publicAddress != 0 &&
        (state == RT_DME_PEER_PUBLIC || state == RT_DME_PEER_UNRESOLVED)) {
        if (peer->info.publicPort == 0) {
            return -1;
        }
        sent = rtDmeDeviceSendDatagram(
            device, record, length, peer->info.publicAddress, peer->info.publicPort);
        if (sent != length) {
            result = sent;
        }
    }
    if (peer->info.localAddress == 0) {
        return result;
    }
    if (state >= RT_DME_PEER_PUBLIC) {
        return result;
    }
    if (peer->info.localPort == 0) {
        return -1;
    }
    sent = rtDmeDeviceSendDatagram(
        device, record, length, peer->info.localAddress, peer->info.localPort);
    if (sent != length) {
        result = sent;
    }
    return result;
}

// NTSC-U/C: 0x002e55c4, PAL: 0x002ef534
static int rtDmeDeviceDispatchRecord(rtDmeDevice *device,
                                     unsigned int address,
                                     int port,
                                     unsigned char *record,
                                     unsigned int length) {
    unsigned int now = rtDmeGetTime();
    rtDmeWireHeader header;
    rtDmeConnection *connection;
    rtDmePeer *peer;
    unsigned int expected;
    unsigned int i;
    unsigned int last;

    if (length < kWireHeaderSize) {
        return -1;
    }
    memcpy(&header, record, kWireHeaderSize);
    if (header.type & RT_DME_RECORD_IDENTIFIED) {
        header.type &= ~RT_DME_RECORD_IDENTIFIED;
        memcpy(record, &header, kWireHeaderSize);
        peer = rtDmePeerListFind(&device->peers, header.senderId);
        if (peer == NULL) {
            return 0;
        }
        if (peer->state == RT_DME_PEER_UNRESOLVED) {
            if (peer->info.publicAddress == address && peer->info.publicPort == port) {
                peer->state = RT_DME_PEER_PUBLIC;
            } else if (peer->info.localAddress == address && peer->info.localPort == port) {
                peer->state = RT_DME_PEER_LOCAL;
            } else {
                peer = NULL;
            }
        }
    } else {
        peer = device->peers.head;
        while (peer != NULL) {
            if (peer->info.publicAddress == address && peer->info.publicPort == port) {
                if (peer->state == RT_DME_PEER_UNRESOLVED) {
                    peer->state = RT_DME_PEER_PUBLIC;
                }
                break;
            }
            if (peer->info.localAddress == address && peer->info.localPort == port) {
                if (peer->state == RT_DME_PEER_UNRESOLVED) {
                    peer->state = RT_DME_PEER_LOCAL;
                }
                break;
            }
            peer = peer->next;
            if (peer == device->peers.head) {
                peer = NULL;
            }
        }
    }
    if (peer == NULL) {
        return 0;
    }
    peer->lastHeard = now;
    peer->retries = 0;

    if ((header.type >= RT_DME_RECORD_AUDIO_PCM && header.type <= RT_DME_RECORD_AUDIO_SPEECH) ||
        header.type == RT_DME_RECORD_AUDIO_RAW || header.type == RT_DME_RECORD_VIDEO) {
        expected = length - kStreamHeaderSize;
    } else {
        expected = length - kWireHeaderSize;
    }
    if (header.length != expected) {
        return -1;
    }
    if (header.senderId == device->localId) {
        return 0;
    }
    connection = rtDmeDeviceFindConnection(device, header.connectionId);
    if (connection == NULL) {
        return -1;
    }

    last = device->historyIndex;
    for (i = (device->historyIndex - 1) & kHistoryMask; i != last; i = (i - 1) & kHistoryMask) {
        const rtDmeWireHeader *seen = &device->history[i];

        if (seen->sequence == header.sequence && seen->senderId == header.senderId &&
            seen->type == header.type && seen->connectionId == header.connectionId &&
            seen->length == header.length) {
            return 0;
        }
    }
    device->history[device->historyIndex] = header;
    device->historyIndex = (device->historyIndex + 1) & kHistoryMask;

    if (header.connectionId != 0) {
        if ((unsigned short)(header.type - RT_DME_RECORD_AUDIO_PCM) <=
                RT_DME_RECORD_AUDIO_SPEECH - RT_DME_RECORD_AUDIO_PCM ||
            header.type == RT_DME_RECORD_AUDIO_RAW) {
            return rtDmeConnectionHandleAudioRecord(connection, &header, peer, record, length);
        }
        if (header.type == RT_DME_RECORD_VIDEO) {
            return rtDmeConnectionHandleVideoRecord(connection, &header, peer, record, length);
        }
    }
    return rtDmeConnectionHandleControlRecord(connection, &header, peer, record, length);
}

// NTSC-U/C: 0x002e5288, PAL: 0x002ef1f8
static int rtDmeDeviceServicePeers(rtDmeDevice *device) {
    rtDmePeer *peer = device->peers.head;
    unsigned int now = rtDmeGetTime();
    int anyResolved = 0;
    int anyUnresolved = 0;
    rtDmeWireHeader header;

    peer = peer->next;
    while (peer != NULL && peer != device->localPeer) {
        unsigned int timeout;
        unsigned int maxRetries;

        if (peer->state != RT_DME_PEER_UNRESOLVED) {
            anyResolved = 1;
            timeout = kResolvedPeerTimeout;
            maxRetries = kResolvedPeerRetries;
        } else {
            anyUnresolved = 1;
            timeout = kUnresolvedPeerTimeout;
            maxRetries = kUnresolvedPeerRetries;
        }
        if (now - peer->lastHeard < timeout) {
            peer = peer->next;
            continue;
        }
        if (peer->retries < maxRetries) {
            peer->lastHeard = now;
            ++peer->retries;
            header.type = RT_DME_RECORD_PING | RT_DME_RECORD_IDENTIFIED;
            header.length = 0;
            header.sequence = rtDmeConnectionNextSequence(device->connections);
            header.connectionId = 0;
            header.senderId = device->localId;
            rtDmeDeviceSendRecord(device, peer->info.id, &header, NULL, 0);
            peer = peer->next;
            continue;
        }
        if (peer->info.connectionId > 0) {
            rtDmeConnectionRemovePeer(rtDmeDeviceFindConnection(device, peer->info.connectionId),
                                      peer->info.id);
        }
        peer = peer->prev;
        rtDmeDeviceRemovePeer(device, peer->next->info.id);
        peer = peer->next;
    }
    if (device->flags & kDeviceFlagPeersResolved) {
        return 0;
    }
    if (anyResolved && !anyUnresolved) {
        device->flags |= kDeviceFlagPeersResolved;
    }
    return 0;
}

// The frame and packed sizes of an audio record type.
typedef struct {
    unsigned int frameBytes;
    unsigned int packedBytes;
} AudioFrameFormat;

// Encodes one frame of captured audio and sends it on the connection. Returns the bytes sent.
static inline unsigned int rtDmeDeviceSendAudioFrame(rtDmeDevice *device,
                                                     rtDmeConnection *connection,
                                                     rtDmeWireHeader *header,
                                                     const unsigned char *input,
                                                     const AudioFrameFormat *format) {
    unsigned char packed[kRecordBufferSize];
    const unsigned char *payload = packed;

    switch (header->type) {
    case RT_DME_RECORD_AUDIO_SUBBAND:
        rtDmeStartProfileTimer();
        rtSubbandEncode(device->subbandEncoder, (const short *)input, packed);
        rtSubbandEncode(device->subbandEncoder,
                        (const short *)&input[RT_SUBBAND_FRAME_BYTES],
                        &packed[RT_SUBBAND_PACKED_SIZE]);
        device->audioEncodeTime += rtDmeGetProfileTimerElapsed();
        break;
    case RT_DME_RECORD_AUDIO_LPC:
        rtDmeStartProfileTimer();
        rtLpcEncode((const short *)input, packed);
        rtLpcEncode((const short *)&input[RT_LPC_FRAME_BYTES], &packed[RT_LPC_PACKED_SIZE]);
        device->audioEncodeTime += rtDmeGetProfileTimerElapsed();
        break;
    case RT_DME_RECORD_AUDIO_SPEECH:
        rtDmeStartProfileTimer();
        rtSpeechEncode((const short *)input, packed);
        rtSpeechEncode((const short *)&input[RT_SPEECH_FRAME_BYTES],
                       &packed[RT_SPEECH_PACKED_SIZE]);
        device->audioEncodeTime += rtDmeGetProfileTimerElapsed();
        break;
    default:
        payload = input;
        break;
    }
    device->audioBytesEncoded += format->frameBytes;
    header->sequence = rtDmeConnectionNextSequence(connection);
    header->length = format->packedBytes;
    rtDmeConnectionSend(connection, device->localId, header, payload, format->packedBytes);
    return format->packedBytes;
}

// NTSC-U/C: 0x002e638c, PAL: 0x002f02fc
static int rtDmeDeviceEncodeAudio(rtDmeDevice *device,
                                  int codec,
                                  const unsigned char *data,
                                  unsigned int length) {
    unsigned char empty[kRecordBufferSize];
    rtDmeWireHeader header;
    rtDmeConnection *connection;
    AudioFrameFormat format;
    unsigned int total = 0;

    connection = rtDmeDeviceGetActiveConnection(device);
    if (connection == NULL) {
        return 0;
    }
    if (device->audioCodec != codec) {
        device->audioCodec = codec;
        device->audioEncodeLength = 0;
    }
    switch (codec) {
    case RT_DME_AUDIO_PCM:
        header.type = RT_DME_RECORD_AUDIO_PCM;
        break;
    case RT_DME_AUDIO_SUBBAND:
        header.type = RT_DME_RECORD_AUDIO_SUBBAND;
        break;
    case RT_DME_AUDIO_LPC:
        header.type = RT_DME_RECORD_AUDIO_LPC;
        break;
    case RT_DME_AUDIO_SPEECH:
        header.type = RT_DME_RECORD_AUDIO_SPEECH;
        break;
    default:
        header.type = RT_DME_RECORD_AUDIO_RAW;
        break;
    }
    header.senderId = device->localId;
    header.connectionId = (unsigned short)connection->id;

    if (length == 0) {
        header.sequence = rtDmeConnectionNextSequence(connection);
        header.length = 0;
        rtDmeConnectionSend(connection, device->localId, &header, empty, 0);
        rtDmeDeviceResetAudioSequence(device);
        return total;
    }
    switch (header.type) {
    case RT_DME_RECORD_AUDIO_RAW:
        header.sequence = rtDmeConnectionNextSequence(connection);
        header.length = length;
        rtDmeConnectionSend(connection, device->localId, &header, data, length);
        return length;
    case RT_DME_RECORD_AUDIO_SUBBAND:
        format.frameBytes = kHalvesPerFrame * RT_SUBBAND_FRAME_BYTES;
        format.packedBytes = kHalvesPerFrame * RT_SUBBAND_PACKED_SIZE;
        break;
    case RT_DME_RECORD_AUDIO_LPC:
        format.frameBytes = kHalvesPerFrame * RT_LPC_FRAME_BYTES;
        format.packedBytes = kHalvesPerFrame * RT_LPC_PACKED_SIZE;
        break;
    case RT_DME_RECORD_AUDIO_SPEECH:
        format.frameBytes = kHalvesPerFrame * RT_SPEECH_FRAME_BYTES;
        format.packedBytes = kHalvesPerFrame * RT_SPEECH_PACKED_SIZE;
        break;
    default:
        format.frameBytes = kPcmFrameBytes;
        format.packedBytes = kPcmFrameBytes;
        break;
    }

    while (length != 0) {
        unsigned int buffered = device->audioEncodeLength;
        unsigned int needed;

        if (buffered == 0) {
            if (length < format.frameBytes) {
                memcpy(device->audioEncodeBuffer, data, length);
                device->audioEncodeLength = length;
                return total;
            }
            length -= format.frameBytes;
            total += rtDmeDeviceSendAudioFrame(device, connection, &header, data, &format);
            data += format.frameBytes;
            continue;
        }
        needed = format.frameBytes - buffered;
        if (length < needed) {
            memcpy(&device->audioEncodeBuffer[buffered], data, length);
            device->audioEncodeLength += length;
            return total;
        }
        memcpy(&device->audioEncodeBuffer[device->audioEncodeLength], data, needed);
        data += needed;
        length -= needed;
        device->audioEncodeLength += needed;
        total += rtDmeDeviceSendAudioFrame(
            device, connection, &header, device->audioEncodeBuffer, &format);
        device->audioEncodeLength -= format.frameBytes;
    }
    return total;
}

// Decodes one frame of a peer and plays it. Returns the bytes of samples played.
static inline unsigned int
rtDmeDeviceDecodeAudioFrame(rtDmeDevice *device, int codec, const unsigned char *frame) {
    short samples[kReceiveSize / sizeof(short)];
    unsigned int bytes;

    rtDmeStartProfileTimer();
    switch (codec) {
    case RT_DME_AUDIO_SUBBAND:
        rtSubbandDecode(device->subbandDecoder, frame, samples);
        bytes = RT_SUBBAND_FRAME_BYTES;
        break;
    case RT_DME_AUDIO_LPC:
        rtLpcDecode(frame, samples);
        bytes = RT_LPC_FRAME_BYTES;
        break;
    default:
        rtSpeechDecode(frame, samples);
        bytes = RT_SPEECH_FRAME_BYTES;
        break;
    }
    device->audioDecodeTime += rtDmeGetProfileTimerElapsed();
    device->audioBytesDecoded += bytes;
    if (device->playAudio != NULL) {
        device->playAudio(device, codec, samples, bytes);
    }
    return bytes;
}

// NTSC-U/C: 0x002e5ebc, PAL: 0x002efe2c
static unsigned int rtDmeDeviceDecodeAudio(rtDmeDevice *device,
                                           int codec,
                                           const unsigned char *data,
                                           unsigned int length) {
    unsigned int frameBytes;
    unsigned int total = 0;

    switch (codec) {
    case RT_DME_AUDIO_SUBBAND:
        frameBytes = RT_SUBBAND_PACKED_SIZE;
        break;
    case RT_DME_AUDIO_LPC:
        frameBytes = RT_LPC_PACKED_SIZE;
        break;
    case RT_DME_AUDIO_SPEECH:
        frameBytes = RT_SPEECH_PACKED_SIZE;
        break;
    default:
        device->audioBytesDecoded += length;
        if (device->playAudio != NULL) {
            device->playAudio(device, codec, data, length);
        }
        return total + length;
    }

    while (length != 0) {
        unsigned int buffered = device->audioDecodeLength;
        unsigned int needed;

        if (buffered == 0) {
            if (length < frameBytes) {
                memcpy(device->audioDecodeBuffer, data, length);
                device->audioDecodeLength = length;
                return total;
            }
            total += rtDmeDeviceDecodeAudioFrame(device, codec, data);
            data += frameBytes;
            length -= frameBytes;
            continue;
        }
        needed = frameBytes - buffered;
        if (length < needed) {
            memcpy(&device->audioDecodeBuffer[buffered], data, length);
            device->audioDecodeLength += length;
            return total;
        }
        memcpy(&device->audioDecodeBuffer[device->audioDecodeLength], data, needed);
        data += needed;
        length -= needed;
        total += rtDmeDeviceDecodeAudioFrame(device, codec, device->audioDecodeBuffer);
        if (codec == RT_DME_AUDIO_SUBBAND) {
            // Yes, the binary omits the bytes it just copied for this codec.
            device->audioDecodeLength -= frameBytes;
        } else {
            device->audioDecodeLength = needed + device->audioDecodeLength - frameBytes;
        }
    }
    return total;
}

int rtDmeDeviceUpdate(rtDmeDevice *device) {
    unsigned char buffer[kVideoBufferSize];
    unsigned int address;
    int port = 0;
    int codec;
    int result = 0;
    unsigned int captured;
    int length;
    rtDmeConnection *connection;

    (void)rtDmeGetTime(); // Yes, the binary discards the time.
    codec = 0;
    if (device->upnp != NULL) {
        RtUpnpPollRequest(device->upnp);
        if (device->upnpState == RT_DME_UPNP_PENDING) {
            return 0;
        }
    }
    for (;;) {
        length = rtDmeDeviceReceive(device, buffer, kReceiveSize, &address, &port);
        if (length < 0) {
            result = -1;
            break;
        }
        if (length == 0) {
            break;
        }
        if (rtDmeDeviceDispatchRecord(device, address, port, buffer, length) != 0) {
            result = -1;
            break;
        }
    }
    if (result == 0) {
        result = rtDmeDeviceServicePeers(device);
        if (result == 0 && device->activeConnectionId != 0) {
            connection = rtDmeDeviceFindConnection(device, device->activeConnectionId);
            if (connection != NULL) {
                result = rtDmeConnectionUpdate(connection);
            }
        }
    }

    captured = 0;
    if (device->captureAudio != NULL) {
        do {
            length = device->captureAudio(device, &codec, buffer, kReceiveSize);
            if (length <= 0) {
                break;
            }
            captured += length;
            rtDmeDeviceEncodeAudio(device, codec, buffer, length);
        } while ((unsigned int)length >= kReceiveSize);
    }
    codec = 0;
    while ((length = rtDmeSlotTableRead(device->slotTable, &codec, buffer, kReceiveSize)) > 0) {
        rtDmeDeviceDecodeAudio(device, codec, buffer, length);
    }

    if (captured <= kVideoAudioBudget) {
        length = rtDmeDeviceCaptureVideo(device, buffer, kVideoBufferSize);
        if (length > 0) {
            rtDmeDeviceSendVideoFrame(device, buffer, length);
        }
    }

    if (device->activeConnectionId == 0) {
        return result;
    }
    connection = rtDmeDeviceFindConnection(device, device->activeConnectionId);
    if (connection != NULL) {
        rtDmeEndpointNode *node = connection->peers.head;

        while (node != NULL) {
            rtDmePeer *peer = node->peer;

            if (peer->flags & RT_DME_PEER_VIDEO_READY) {
                rtDmeDeviceDisplayPeerVideo(device, peer);
                peer->flags &= ~RT_DME_PEER_VIDEO_READY;
            }
            node = node->next;
            if (node == connection->peers.head) {
                node = NULL;
            }
        }
    }
    return result;
}

int rtDmeDeviceOpenConnection(rtDmeDevice *device,
                              int maxPeers,
                              int *connectionId,
                              const char *name) {
    rtDmeConnection *connection = device->connections;
    rtDmeOpenConnectionRecord record;
    rtDmeWireHeader header;
    rtDmePeer *localPeer;

    if (connectionId == NULL) {
        return 1;
    }
    if (name == NULL) {
        return 1;
    }
    if (name[0] == '\0') {
        return 1;
    }
    *connectionId = 1;
    while (rtDmeDeviceFindConnection(device, *connectionId) != NULL) {
        *connectionId = *connectionId + 1;
    }
    for (connection = connection->next; connection != NULL && connection != device->connections;
         connection = connection->next) {
        if (strcmp(connection->name, name) == 0) {
            return 1;
        }
    }
    if (device->activeConnectionId > 0) {
        rtDmeDeviceDeselectConnection(device, device->activeConnectionId);
        device->activeConnectionId = 0;
    }
    connection = rtDmeDeviceAddConnection(device, *connectionId, maxPeers);
    strncpy(connection->name, name, RT_DME_CONNECTION_NAME_SIZE);
    connection->name[RT_DME_CONNECTION_NAME_SIZE - 1] = '\0';
    localPeer = rtDmePeerListFind(&device->peers, device->localId);
    rtDmeConnectionAddPeer(connection, localPeer);
    record.size = sizeof(record);
    record.connectionId = (unsigned short)connection->id;
    record.maxPeers = (unsigned short)connection->maxPeers;
    record.reserved06 = (unsigned short)connection->maxPeers;
    strncpy(record.name, connection->name, RT_DME_OPEN_CONNECTION_NAME_SIZE);
    record.name[RT_DME_OPEN_CONNECTION_NAME_SIZE - 1] = '\0';
    rtDmeConnectionInitHeader(
        device->connections, &header, RT_DME_RECORD_OPEN_CONNECTION, sizeof(record));
    device->activeConnectionId = record.connectionId;
    rtDmePeerSetConnection(localPeer, record.connectionId);
    return rtDmeConnectionSend(
        device->connections, device->localId, &header, &record, sizeof(record));
}

rtDmeConnection *rtDmeDeviceAddConnection(rtDmeDevice *device, int id, int maxPeers) {
    rtDmeConnection *connection = rtDmeConnectionCreate(device, id, maxPeers);
    rtDmeConnection *head = device->connections;

    if (head != NULL) {
        connection->next = head;
        connection->prev = device->connections->prev;
        head->prev = connection;
        connection->prev->next = connection;
    } else {
        connection->next = connection;
        connection->prev = connection;
        device->connections = connection;
    }
    return connection;
}

rtDmeConnection *rtDmeDeviceFindConnection(rtDmeDevice *device, int id) {
    rtDmeConnection *connection = device->connections;

    while (connection != NULL) {
        if (connection->id == id) {
            return connection;
        }
        connection = connection->next;
        if (connection == device->connections) {
            return NULL;
        }
    }
    return connection;
}

// NTSC-U/C: 0x002e6b50, PAL: 0x002f0ac0
static int rtDmeDeviceSendVideoFrame(rtDmeDevice *device, const unsigned char *frame, int length) {
    rtDmeConnection *connection = rtDmeDeviceGetSelectedConnection(device);
    rtDmeWireHeader header;

    if (connection == NULL) {
        return 0;
    }
    header.type = RT_DME_RECORD_VIDEO;
    header.senderId = device->localId;
    header.connectionId = (unsigned short)connection->id;
    header.length = (unsigned short)length;
    header.sequence = rtDmeConnectionNextSequence(connection);
    rtDmeConnectionSend(connection, device->localId, &header, frame, header.length);
    if (length == 0) {
        rtDmeDeviceResetVideoSequence(device);
    }
    return length;
}

// NTSC-U/C: 0x002e6c04, PAL: 0x002f0b74
static rtDmeConnection *rtDmeDeviceGetActiveConnection(rtDmeDevice *device) {
    rtDmeConnection *connection = NULL;

    if (device->activeConnectionId > 0) {
        connection = rtDmeDeviceFindConnection(device, device->activeConnectionId);
        if (connection != NULL && rtDmeConnectionCheckTalk(connection) != 0) {
            connection = NULL;
        }
    }
    return connection;
}

// NTSC-U/C: 0x002e6c58, PAL: 0x002f0bc8
static rtDmeConnection *rtDmeDeviceGetSelectedConnection(rtDmeDevice *device) {
    if (device->activeConnectionId <= 0) {
        return NULL;
    }
    return rtDmeDeviceFindConnection(device, device->activeConnectionId);
}

// NTSC-U/C: 0x002e6c88, PAL: 0x002f0bf8
static int rtDmeDeviceSelectConnection(rtDmeDevice *device, int connectionId) {
    rtDmeWireHeader header;
    unsigned short payload;
    rtDmeConnection *connection;
    rtDmePeer *localPeer;
    int result = 0;

    if (device->activeConnectionId > 0 && device->activeConnectionId != connectionId &&
        (device->flags & kDeviceFlagConnectionAlive)) {
        rtDmeDeviceDeselectConnection(device, device->activeConnectionId);
    }
    if (connectionId <= 0) {
        return result;
    }
    connection = rtDmeDeviceFindConnection(device, connectionId);
    payload = (unsigned short)connectionId;
    rtDmeConnectionInitHeader(
        device->connections, &header, RT_DME_RECORD_SELECT_CONNECTION, sizeof(payload));
    result = rtDmeConnectionSend(
        device->connections, device->localId, &header, &payload, sizeof(payload));
    if (result != 0) {
        return result;
    }
    localPeer = rtDmePeerListFind(&device->peers, device->localId);
    device->activeConnectionId = connectionId;
    rtDmePeerSetConnection(localPeer, connectionId);
    if (connection != NULL) {
        rtDmeConnectionAddPeer(connection, localPeer);
        connection->selectTime = rtDmeGetTime();
    }
    return result;
}

int rtDmeDeviceDeselectConnection(rtDmeDevice *device, int connectionId) {
    rtDmeWireHeader header;
    unsigned short payload;
    rtDmeConnection *connection;
    int result;

    if (connectionId == 0 || connectionId != device->activeConnectionId) {
        return -1;
    }
    payload = (unsigned short)connectionId;
    rtDmeConnectionInitHeader(
        device->connections, &header, RT_DME_RECORD_DESELECT_CONNECTION, sizeof(payload));
    result = rtDmeConnectionSend(
        device->connections, device->localId, &header, &payload, sizeof(payload));
    if (result != 0) {
        return result;
    }
    connection = rtDmeDeviceFindConnection(device, device->activeConnectionId);
    if (connection != NULL) {
        rtDmeEndpointListRemove(&connection->peers, device->peers.head->info.id);
        rtDmeConnectionResetSlots(connection);
    }
    device->activeConnectionId = 0;
    rtDmePeerSetConnection(device->localPeer, 0);
    device->flags &= ~kDeviceFlagConnectionAlive;
    return result;
}

int rtDmeDeviceSetLocalId(rtDmeDevice *device, int id) {
    device->localId = id;
    device->localPeer->info.id = id;
    return 0;
}

int rtDmeDeviceGetLocalInfo(rtDmeDevice *device, rtDmePeerInfo *info) {
    *info = device->localPeer->info;
    return 0;
}

rtDmePeer *rtDmeDeviceAddPeer(rtDmeDevice *device, const rtDmePeerInfo *info) {
    rtDmePeer *peer = rtDmePeerListAdd(&device->peers, info);
    rtDmeConnection *connection;
    rtDmeWireHeader header;
    unsigned short payload;

    rtDmeConnectionAddPeer(device->connections, peer);
    if (info->connectionId <= 0) {
        return peer;
    }
    connection = rtDmeDeviceFindConnection(device, info->connectionId);
    if (connection == NULL) {
        connection = rtDmeDeviceAddConnection(device, info->connectionId, kDefaultMaxPeers);
        payload = (unsigned short)info->connectionId;
        rtDmeConnectionInitHeader(
            connection, &header, RT_DME_RECORD_QUERY_CONNECTION, sizeof(payload));
        rtDmeDeviceSendRecord(device, info->id, &header, &payload, sizeof(payload));
    }
    rtDmeConnectionAddPeer(connection, peer);
    return peer;
}

// NTSC-U/C: 0x002e702c, PAL: 0x002f0f9c
static void rtDmeDeviceForgetSender(rtDmeDevice *device, int senderId) {
    int i;

    for (i = 0; i < RT_DME_RECORD_HISTORY_SIZE; ++i) {
        if (device->history[i].senderId == senderId) {
            memset(&device->history[i], 0, sizeof(device->history[i]));
        }
    }
}

void rtDmeDeviceRemovePeer(rtDmeDevice *device, int peerId) {
    rtDmePeer *peer = rtDmePeerListFind(&device->peers, peerId);
    rtDmeConnection *connection;

    if (peer == NULL) {
        return;
    }
    if (peer->info.connectionId > 0) {
        connection = rtDmeDeviceFindConnection(device, peer->info.connectionId);
        if (connection != NULL) {
            rtDmeConnectionRemovePeer(connection, peer->info.id);
        }
    }
    if (device->connections != NULL) {
        rtDmeConnectionRemovePeer(device->connections, peer->info.id);
    }
    rtDmeDeviceForgetSender(device, peer->info.id);
    rtDmePeerListRemove(&device->peers, peer->info.id);
}

unsigned int rtDmeDeviceGetStatus(rtDmeDevice *device, rtDmePeerInfo *talker) {
    rtDmeConnection *connection = NULL;

    if (device->activeConnectionId > 0) {
        connection = rtDmeDeviceFindConnection(device, device->activeConnectionId);
    }
    if (connection != NULL && talker != NULL) {
        rtDmeConnectionGetTalker(connection, talker);
    } else {
        memset(talker, 0, sizeof(*talker)); // Yes, the binary clears a null talker too.
    }
    if (rtDmeDeviceGetActiveConnection(device) != NULL) {
        device->flags |= kDeviceFlagTalkAllowed;
    } else {
        device->flags &= ~kDeviceFlagTalkAllowed;
    }
    return device->flags;
}

// NTSC-U/C: 0x002e7134, PAL: 0x002f10a4
static void rtDmeDeviceResetSubbandDecoder(rtDmeDevice *device) {
    if (device->subbandDecoder != NULL) {
        rtDmeSubbandCodecReset(device->subbandDecoder);
    }
}

// NTSC-U/C: 0x002e715c, PAL: 0x002f10cc
static void rtDmeDeviceResetLpc(rtDmeDevice *device) {
    (void)device; // Yes, the binary ignores the device.
    rtLpcReset();
}

// NTSC-U/C: 0x002e7178, PAL: 0x002f10e8
static void rtDmeDeviceResetSpeech(rtDmeDevice *device) {
    (void)device; // Yes, the binary ignores the device.
    rtSpeechInit();
}

// NTSC-U/C: 0x002e7194, PAL: 0x002f1104
static void rtDmeDeviceOnPortMapped(RtUpnpContext *context,
                                    int status,
                                    int service,
                                    const char *address,
                                    unsigned short port,
                                    void *owner) {
    rtDmeDevice *device = owner;

    if (device == NULL) {
        return;
    }
    if (context != device->upnp) {
        return;
    }
    if (status == 0) {
        device->upnpResult = service;
        device->publicAddress = rtNetParseAddress(address);
        device->publicPort = port;
        device->localPeer->info.publicAddress = device->publicAddress;
        device->localPeer->info.publicPort = port;
    } else {
        device->upnpResult = 0;
        device->publicAddress = 0;
        device->publicPort = 0;
        device->localPeer->info.publicAddress = 0;
        device->localPeer->info.publicPort = port; // Yes, the binary records the failed port.
    }
    device->upnpState = RT_DME_UPNP_MAPPED; // Yes, the binary marks a failed mapping as mapped.
    if (device->onPortMapped != NULL) {
        device->onPortMapped(device, device->publicAddress, device->publicPort);
    }
}

// NTSC-U/C: 0x002e7298, PAL: 0x002f1208
static int rtDmeRandom(void) {
    if (g_dwRandSeed == 0) {
        g_dwRandSeed = rtDmeGetTime();
    }
    g_dwRandSeed = g_dwRandSeed * kRandMultiplier + kRandIncrement;
    return (g_dwRandSeed >> kRandShift) & kRandMask;
}

// NTSC-U/C: 0x002e7304, PAL: 0x002f1274
static void rtDmeConnectionSlotReset(rtDmeConnectionSlot *slot) {
    slot->state = RT_DME_SLOT_FREE;
    slot->peerId = -1;
    slot->peer = NULL;
    slot->lastHeard = 0;
    slot->retries = 0;
}

// NTSC-U/C: 0x002e7320, PAL: 0x002f1290
static void
rtDmeConnectionSlotAssign(rtDmeDevice *device, rtDmeConnectionSlot *slot, int state, int peerId) {
    slot->state = state;
    slot->peerId = peerId;
    slot->peer = rtDmePeerListFind(&device->peers, peerId);
    slot->lastHeard = rtDmeGetTime();
    slot->retries = 0;
}

// NTSC-U/C: 0x002e7364, PAL: 0x002f12d4
static rtDmeConnection *rtDmeConnectionCreate(rtDmeDevice *device, int id, int maxPeers) {
    void *block = NULL;
    rtDmeConnection *connection;
    unsigned int now;
    unsigned int i;

    RtAllocZeroedBlock(&block, sizeof(rtDmeConnection));
    connection = block;
    if (connection == NULL) {
        return NULL;
    }
    now = rtDmeGetTime();
    connection->device = device;
    connection->id = id;
    connection->prev = NULL;
    connection->next = NULL;
    connection->sequence = 0;
    connection->selectTime = 0;
    connection->offerTime = 0;
    connection->talkAllowed = 0;
    connection->talkerId = -1;
    connection->talkTime = 0;
    connection->name[0] = '\0';
    connection->talkTime = now;
    connection->maxPeers = maxPeers;
    connection->slotCount = connection->maxPeers != 0 ? connection->maxPeers : kDefaultSlotCount;
    RtAllocZeroedBlock((void **)&connection->slots,
                       connection->slotCount * sizeof(rtDmeConnectionSlot));
    for (i = 0; i < connection->slotCount; ++i) {
        connection->slots[i].lastHeard = now;
    }
    memset(&connection->pendingSlots, 0, sizeof(connection->pendingSlots));
    connection->peers.head = NULL;
    connection->peers.count = 0;
    return connection;
}

// NTSC-U/C: 0x002e74dc, PAL: 0x002f144c
static void rtDmeConnectionDestroy(rtDmeConnection *connection) {
    void *block = connection;

    if (connection == NULL) {
        return;
    }
    rtDmeEndpointListClear(&connection->peers);
    if (connection->slots != NULL) {
        RtFreeBlock((void **)&connection->slots);
        connection->slots = NULL;
        connection->slotCount = 0;
    }
    RtFreeBlock(&block);
}

// NTSC-U/C: 0x002e7530, PAL: 0x002f14a0
static unsigned int rtDmeConnectionNextSequence(rtDmeConnection *connection) {
    return connection->sequence++;
}

// NTSC-U/C: 0x002e7548, PAL: 0x002f14b8
static int rtDmeConnectionSend(rtDmeConnection *connection,
                               int senderId,
                               const rtDmeWireHeader *header,
                               const void *payload,
                               int length) {
    rtDmeRecordBuffer record;
    const unsigned char *data = payload;
    unsigned int frameSequence;
    unsigned int size;
    unsigned int i;
    int first = 1;
    int result = 0;
    int chunk;

    record.header = *header;
    for (;;) {
        size = 0;
        if (header->type == RT_DME_RECORD_AUDIO_RAW ||
            (header->type >= RT_DME_RECORD_AUDIO_PCM &&
             header->type <= RT_DME_RECORD_AUDIO_SPEECH)) {
            frameSequence = rtDmeDeviceNextAudioSequence(connection->device);
            memcpy(&record.payload[size], &frameSequence, sizeof(frameSequence));
            size += sizeof(frameSequence);
        } else if (header->type == RT_DME_RECORD_VIDEO) {
            frameSequence = rtDmeDeviceNextVideoSequence(connection->device);
            if (first) {
                frameSequence |= kFrameFirst;
            }
            if ((unsigned int)length <= kVideoFragmentSize) {
                frameSequence |= kFrameLast;
            }
            memcpy(&record.payload[size], &frameSequence, sizeof(frameSequence));
            size += sizeof(frameSequence);
        }
        chunk = length;
        if (header->type == RT_DME_RECORD_VIDEO) {
            chunk = length <= kVideoFragmentSize ? length : kVideoFragmentSize;
            record.header.length = chunk;
        }
        memcpy(&record.payload[size], data, chunk);
        size += chunk;
        data += chunk;
        length -= chunk;

        if (connection->id > 0) {
            for (i = 0; i < connection->slotCount; ++i) {
                const rtDmeConnectionSlot *slot = &connection->slots[i];

                if (slot->state == RT_DME_SLOT_CONNECTED && slot->peerId != senderId &&
                    rtDmeDeviceSendToPeer(
                        connection->device, &record, kWireHeaderSize + size, slot->peer) !=
                        (int)(kWireHeaderSize + size)) {
                    result = -1;
                }
            }
        } else {
            rtDmePeer *peer;

            for (peer = connection->device->peers.head->next; peer != connection->device->localPeer;
                 peer = peer->next) {
                if (rtDmeDeviceSendToPeer(
                        connection->device, &record, kWireHeaderSize + size, peer) !=
                    (int)(kWireHeaderSize + size)) {
                    result = -1;
                }
            }
        }

        first = 0;
        if (length == 0) {
            return result;
        }
        record.header.sequence = rtDmeConnectionNextSequence(connection);
    }
}

// NTSC-U/C: 0x002e77b4, PAL: 0x002f1724
static void rtDmeConnectionInitHeader(rtDmeConnection *connection,
                                      rtDmeWireHeader *header,
                                      int type,
                                      int length) {
    header->type = (unsigned short)(type | RT_DME_RECORD_IDENTIFIED);
    header->length = (unsigned short)length;
    header->sequence = rtDmeConnectionNextSequence(connection);
    header->connectionId = (unsigned short)connection->id;
    header->senderId = connection->device->localId;
}

// NTSC-U/C: 0x002e780c, PAL: 0x002f177c
static int rtDmeConnectionGrowSlots(rtDmeConnection *connection) {
    void *previous;
    const rtDmeConnectionSlot *previousSlots;
    unsigned int now;
    unsigned int i;

    if (connection == NULL) {
        return -1;
    }
    if (connection->maxPeers != 0) {
        return -1;
    }
    now = rtDmeGetTime();
    previous = connection->slots;
    RtAllocZeroedBlock((void **)&connection->slots,
                       connection->slotCount * kSlotGrowth * sizeof(rtDmeConnectionSlot));
    if (connection->slots == NULL) {
        connection->slots = previous;
        return -1;
    }
    previousSlots = previous;
    for (i = 0; i < connection->slotCount; ++i) {
        connection->slots[i] = previousSlots[i];
    }
    RtFreeBlock(&previous);
    connection->slotCount *= kSlotGrowth;
    for (; i < connection->slotCount; ++i) {
        memset(&connection->slots[i], 0, sizeof(connection->slots[i]));
        connection->slots[i].lastHeard = now;
    }
    return 0;
}

// NTSC-U/C: 0x002e7968, PAL: 0x002f18d8
static int rtDmeConnectionUpdate(rtDmeConnection *connection) {
    unsigned int now = rtDmeGetTime();
    rtDmeWireHeader offer;
    rtDmeWireHeader ping;
    int alive = 0;
    unsigned int i;

    if (connection->offerTime != 0 && now - connection->offerTime > kOfferDelay) {
        for (i = 0; i < connection->slotCount; ++i) {
            if (connection->slots[i].state == RT_DME_SLOT_FREE) {
                break;
            }
        }
        if (i < connection->slotCount) {
            rtDmeConnectionSlotAssign(connection->device,
                                      &connection->slots[i],
                                      RT_DME_SLOT_OFFERED,
                                      connection->pendingSlots.senderId);
            rtDmeConnectionInitHeader(connection, &offer, RT_DME_RECORD_SLOT_OFFER, 0);
            rtDmeDeviceSendRecord(
                connection->device, connection->pendingSlots.senderId, &offer, NULL, 0);
        }
        connection->offerTime = 0;
        memset(&connection->pendingSlots, 0, sizeof(connection->pendingSlots));
    }

    for (i = 0; i < connection->slotCount; ++i) {
        rtDmeConnectionSlot *slot = &connection->slots[i];

        if (slot->state != RT_DME_SLOT_CONNECTED) {
            continue;
        }
        if ((int)(now - slot->lastHeard) < kSlotTimeout) {
            alive = 1;
            continue;
        }
        if (slot->retries++ < kSlotRetries) {
            slot->lastHeard = now;
            rtDmeConnectionInitHeader(connection, &ping, RT_DME_RECORD_PING, 0);
            rtDmeDeviceSendRecord(connection->device, slot->peerId, &ping, NULL, 0);
            alive = 1;
            continue;
        }
        if (connection->id > 0) {
            rtDmeConnectionRemovePeer(connection, slot->peerId);
        }
        // Yes, the binary reads the peer of the slot again after the slot may have been reset.
        rtDmeDeviceRemovePeer(connection->device, slot->peerId);
    }

    if (connection->id <= 0) {
        return 0;
    }
    if (alive) {
        connection->device->flags |= kDeviceFlagConnectionAlive;
        return 0;
    }
    connection->device->flags &= ~kDeviceFlagConnectionAlive;
    if (now - connection->selectTime > kSelectRetryInterval) {
        rtDmeDeviceSelectConnection(connection->device, connection->id);
    }
    return 0;
}

// Reports whether a connection has more peers than its peer slots take.
static inline int rtDmeConnectionIsOverfull(rtDmeConnection *connection) {
    if (connection->maxPeers == 0) {
        return 0;
    }
    return (unsigned int)(connection->maxPeers + 1) <
           (unsigned int)rtDmeEndpointListCount(&connection->peers);
}

// Refreshes the slot of the peer a record arrived from, and sends the record on to the other
// connected peers when relay is set.
static inline void rtDmeConnectionForward(rtDmeConnection *connection,
                                          const rtDmeWireHeader *header,
                                          const rtDmePeer *peer,
                                          const unsigned char *record,
                                          int length,
                                          int relay,
                                          unsigned int now) {
    unsigned int i;

    for (i = 0; i < connection->slotCount; ++i) {
        rtDmeConnectionSlot *slot = &connection->slots[i];

        if (slot->state != RT_DME_SLOT_CONNECTED) {
            continue;
        }
        if (slot->peerId == peer->info.id) {
            slot->lastHeard = now;
            slot->retries = 0;
        } else if (slot->peerId != header->senderId && relay) {
            rtDmeDeviceSendToPeer(connection->device, record, length, slot->peer);
        }
    }
}

// NTSC-U/C: 0x002e7bd8, PAL: 0x002f1b48
static int rtDmeConnectionHandleAudioRecord(rtDmeConnection *connection,
                                            const rtDmeWireHeader *header,
                                            rtDmePeer *peer,
                                            const unsigned char *record,
                                            int length) {
    unsigned int now = rtDmeGetTime();
    unsigned int frameSequence;
    int codec;

    switch (header->type) {
    case RT_DME_RECORD_AUDIO_PCM:
        codec = RT_DME_AUDIO_PCM;
        break;
    case RT_DME_RECORD_AUDIO_SUBBAND:
        codec = RT_DME_AUDIO_SUBBAND;
        break;
    case RT_DME_RECORD_AUDIO_LPC:
        codec = RT_DME_AUDIO_LPC;
        break;
    case RT_DME_RECORD_AUDIO_SPEECH:
        codec = RT_DME_AUDIO_SPEECH;
        break;
    case RT_DME_RECORD_AUDIO_RAW:
        codec = RT_DME_AUDIO_RAW;
        break;
    default:
        return -1;
    }
    rtDmeConnectionForward(
        connection, header, peer, record, length, rtDmeConnectionIsOverfull(connection), now);
    memcpy(&frameSequence, &record[kWireHeaderSize], sizeof(frameSequence));
    length -= kStreamHeaderSize;
    rtDmeSlotTableStore(connection->device->slotTable,
                        codec,
                        header->senderId,
                        frameSequence,
                        &record[kStreamHeaderSize],
                        length);
    connection->talkTime = now;
    connection->talkerId = length != 0 ? header->senderId : -1;
    return 0;
}

// NTSC-U/C: 0x002e7da4, PAL: 0x002f1d14
static int rtDmeConnectionHandleVideoRecord(rtDmeConnection *connection,
                                            const rtDmeWireHeader *header,
                                            rtDmePeer *peer,
                                            const unsigned char *record,
                                            int length) {
    unsigned int now = rtDmeGetTime();
    rtDmePeer *sender = rtDmePeerListFind(&connection->device->peers, header->senderId);
    const unsigned char *data;
    unsigned int frameSequence;
    int delta;

    if (sender == NULL) {
        return -1;
    }
    if (header->type != RT_DME_RECORD_VIDEO) {
        return -1;
    }
    rtDmeConnectionForward(
        connection, header, peer, record, length, rtDmeConnectionIsOverfull(connection), now);
    memcpy(&frameSequence, &record[kWireHeaderSize], sizeof(frameSequence));
    data = &record[kStreamHeaderSize];
    length -= kStreamHeaderSize;
    if ((frameSequence & (kFrameFirst | kFrameLast)) != (kFrameFirst | kFrameLast)) {
        if (sender->videoFragments == NULL) {
            sender->videoFragments = rtReassemblerCreate(sender);
            if (sender->videoFragments == NULL) {
                return -1;
            }
        }
    } else if (sender->videoFragments == NULL) {
        delta = frameSequence - sender->videoSequence;
        if (delta < 0 && delta >= -kVideoSequenceWindow) {
            return 0;
        }
        if ((unsigned int)length > RT_DME_PEER_FRAME_SIZE) {
            return -1;
        }
        memcpy(sender->frame, data, length);
        sender->frameLength = length;
        sender->flags |= RT_DME_PEER_VIDEO_READY;
        sender->videoSequence = frameSequence + 1;
        return 0;
    }
    rtReassemblerStore(sender->videoFragments, frameSequence, data, length);
    if (rtReassemblerHasMessage(sender->videoFragments) != 0) {
        sender->flags |= RT_DME_PEER_VIDEO_READY;
    }
    return 0;
}

// Moves the sender of an RT_DME_RECORD_SLOTS_FREE record to the connection it selected, and
// offers it a peer slot.
static inline void rtDmeConnectionOnSlotsFree(rtDmeConnection *connection,
                                              const rtDmeWireHeader *header,
                                              const rtDmeSlotsFreeRecord *slotsFree,
                                              unsigned int now) {
    rtDmePeer *sender = rtDmePeerListFind(&connection->device->peers, header->senderId);
    rtDmeConnection *selected;
    rtDmeConnectionSlot *slot;
    rtDmeWireHeader reply;
    unsigned short queryId;
    unsigned int i;

    if (sender->info.connectionId != slotsFree->connectionId) {
        if (sender->info.connectionId > 0) {
            rtDmeConnectionRemovePeer(
                rtDmeDeviceFindConnection(connection->device, sender->info.connectionId),
                sender->info.id);
        }
        rtDmePeerSetConnection(sender, slotsFree->connectionId);
        if (sender->info.connectionId > 0) {
            rtDmeConnectionAddPeer(
                rtDmeDeviceFindConnection(connection->device, sender->info.connectionId), sender);
        }
    }
    if (connection->id == 0) {
        rtDmeConnectionAddPeer(connection, sender);
        if (slotsFree->connectionId != 0) {
            selected = rtDmeDeviceFindConnection(connection->device, slotsFree->connectionId);
            if (selected == NULL) {
                selected = rtDmeDeviceAddConnection(
                    connection->device, slotsFree->connectionId, kDefaultMaxPeers);
                queryId = slotsFree->connectionId;
                rtDmeConnectionInitHeader(
                    selected, &reply, RT_DME_RECORD_QUERY_CONNECTION, sizeof(queryId));
                rtDmeDeviceSendRecord(
                    selected->device, header->senderId, &reply, &queryId, sizeof(queryId));
            }
            rtDmeConnectionAddPeer(selected, sender);
        }
    }

    for (i = 0; i < connection->slotCount; ++i) {
        slot = &connection->slots[i];
        if (slot->state == RT_DME_SLOT_CONNECTED && slot->peerId == header->senderId) {
            return;
        }
    }
    for (i = 0; i < connection->slotCount; ++i) {
        slot = &connection->slots[i];
        if (slot->state == RT_DME_SLOT_FREE || slot->peerId == header->senderId) {
            break;
        }
    }
    if (i >= connection->slotCount) {
        if (connection->maxPeers != 0) {
            return;
        }
        rtDmeConnectionGrowSlots(connection);
        slot = &connection->slots[i];
    }
    if (slotsFree->freeSlots != 0) {
        rtDmeConnectionSlotAssign(connection->device, slot, RT_DME_SLOT_OFFERED, header->senderId);
        rtDmeConnectionInitHeader(connection, &reply, RT_DME_RECORD_SLOT_OFFER, 0);
        rtDmeDeviceSendRecord(connection->device, header->senderId, &reply, NULL, 0);
    } else {
        connection->offerTime = now;
        connection->pendingSlots = *slotsFree;
    }
}

// Gives the sender of an RT_DME_RECORD_SLOT_OFFER a peer slot, taking one from a random peer
// when every slot is in use, and accepts the offer.
static inline void rtDmeConnectionOnSlotOffer(rtDmeConnection *connection,
                                              const rtDmeWireHeader *header) {
    rtDmeConnectionSlot *slot;
    rtDmeWireHeader reply;
    unsigned int i;

    for (i = 0; i < connection->slotCount; ++i) {
        if (connection->slots[i].state != RT_DME_SLOT_CONNECTED ||
            connection->slots[i].peerId == header->senderId) {
            break;
        }
    }
    if (i >= connection->slotCount && connection->maxPeers == 0) {
        rtDmeConnectionGrowSlots(connection);
    }
    slot = &connection->slots[i];
    if (i >= connection->slotCount) {
        slot = &connection->slots[rtDmeRandom() % connection->slotCount];
        rtDmeConnectionInitHeader(connection, &reply, RT_DME_RECORD_SLOT_REVOKE, 0);
        rtDmeDeviceSendRecord(connection->device, slot->peerId, &reply, NULL, 0);
        rtDmeConnectionSlotReset(slot);
    }
    rtDmeConnectionSlotAssign(connection->device, slot, RT_DME_SLOT_CONNECTED, header->senderId);
    rtDmeConnectionInitHeader(connection, &reply, RT_DME_RECORD_SLOT_ACCEPT, 0);
    rtDmeDeviceSendRecord(connection->device, header->senderId, &reply, NULL, 0);
}

// Connects the slot offered to the sender of an RT_DME_RECORD_SLOT_ACCEPT, or revokes the
// acceptance of a slot that was not offered.
static inline void rtDmeConnectionOnSlotAccept(rtDmeConnection *connection,
                                               const rtDmeWireHeader *header,
                                               unsigned int now) {
    rtDmeWireHeader reply;
    unsigned int i;

    for (i = 0; i < connection->slotCount; ++i) {
        rtDmeConnectionSlot *slot = &connection->slots[i];

        if (slot->state == RT_DME_SLOT_OFFERED && slot->peerId == header->senderId) {
            slot->state = RT_DME_SLOT_CONNECTED;
            slot->lastHeard = now;
            slot->retries = 0;
            return;
        }
    }
    rtDmeConnectionInitHeader(connection, &reply, RT_DME_RECORD_SLOT_REVOKE, 0);
    rtDmeDeviceSendRecord(connection->device, header->senderId, &reply, NULL, 0);
}

// Frees the slot of the sender of an RT_DME_RECORD_SLOT_REVOKE and announces the free slot.
static inline void rtDmeConnectionOnSlotRevoke(rtDmeConnection *connection,
                                               const rtDmeWireHeader *header) {
    rtDmeWireHeader released;
    unsigned int i;

    for (i = 0; i < connection->slotCount; ++i) {
        if (connection->slots[i].peerId == header->senderId) {
            rtDmeConnectionSlotReset(&connection->slots[i]);
            rtDmeConnectionInitHeader(connection, &released, RT_DME_RECORD_SLOT_RELEASED, 0);
            rtDmeConnectionSend(connection, header->senderId, &released, NULL, 0);
            return;
        }
    }
}

// Creates the connection of an RT_DME_RECORD_OPEN_CONNECTION record and adds its sender.
static inline void rtDmeConnectionOnOpenConnection(rtDmeConnection *connection,
                                                   const rtDmeWireHeader *header,
                                                   const rtDmeOpenConnectionRecord *open) {
    rtDmePeer *sender;
    rtDmeConnection *opened;

    if (rtDmeDeviceFindConnection(connection->device, open->connectionId) != NULL) {
        return;
    }
    sender = rtDmePeerListFind(&connection->device->peers, header->senderId);
    opened = rtDmeDeviceAddConnection(connection->device, open->connectionId, open->maxPeers);
    // Yes, the binary copies past the end of the shorter name of the record.
    strncpy(opened->name, open->name, RT_DME_CONNECTION_NAME_SIZE);
    opened->name[RT_DME_CONNECTION_NAME_SIZE - 1] = '\0';
    rtDmePeerSetConnection(sender, open->connectionId);
    rtDmeConnectionAddPeer(opened, sender);
}

// Sends the details of the connection an RT_DME_RECORD_QUERY_CONNECTION record identifies.
static inline void rtDmeConnectionOnQueryConnection(rtDmeConnection *connection,
                                                    const rtDmeWireHeader *header,
                                                    const unsigned short *connectionId) {
    rtDmeOpenConnectionRecord info;
    rtDmeWireHeader reply;
    rtDmeConnection *queried = rtDmeDeviceFindConnection(connection->device, *connectionId);

    if (queried == NULL) {
        return;
    }
    info.size = sizeof(info);
    info.connectionId = (unsigned short)queried->id;
    info.maxPeers = (unsigned short)queried->maxPeers;
    info.reserved06 = (unsigned short)queried->maxPeers;
    strncpy(info.name, queried->name, RT_DME_OPEN_CONNECTION_NAME_SIZE);
    info.name[RT_DME_OPEN_CONNECTION_NAME_SIZE - 1] = '\0';
    rtDmeConnectionInitHeader(queried, &reply, RT_DME_RECORD_CONNECTION_INFO, sizeof(info));
    rtDmeDeviceSendRecord(queried->device, header->senderId, &reply, &info, sizeof(info));
}

// Takes the name and the peer limit of an RT_DME_RECORD_CONNECTION_INFO record.
static inline void rtDmeConnectionOnConnectionInfo(rtDmeConnection *connection,
                                                   const rtDmeOpenConnectionRecord *info) {
    rtDmeConnection *described = rtDmeDeviceFindConnection(connection->device, info->connectionId);

    if (described == NULL) {
        return;
    }
    // Yes, the binary copies past the end of the shorter name of the record.
    strncpy(described->name, info->name, RT_DME_CONNECTION_NAME_SIZE);
    if (info->maxPeers == 0 && described->maxPeers != 0) {
        described->maxPeers = 0;
        rtDmeConnectionGrowSlots(described);
    }
}

// Moves the sender of an RT_DME_RECORD_SELECT_CONNECTION record to the connection it selected.
static inline int rtDmeConnectionOnSelectConnection(rtDmeConnection *connection,
                                                    const rtDmeWireHeader *header,
                                                    const unsigned short *connectionId) {
    rtDmeDevice *device = connection->device;
    rtDmeConnection *selected;
    rtDmePeer *sender;

    if (*connectionId == 0) {
        return 0;
    }
    sender = rtDmePeerListFind(&device->peers, header->senderId);
    if (sender == NULL) {
        return 0;
    }
    if (sender->info.connectionId > 0) {
        selected = rtDmeDeviceFindConnection(device, sender->info.connectionId);
        if (selected != NULL) {
            rtDmeConnectionRemovePeer(selected, sender->info.id);
        }
    }
    selected = rtDmeDeviceFindConnection(device, *connectionId);
    if (selected == NULL) {
        return 0;
    }
    rtDmePeerSetConnection(sender, selected->id);
    rtDmeConnectionAddPeer(selected, sender);
    if (device->activeConnectionId != selected->id) {
        return 0;
    }
    return rtDmeConnectionSendSlotsFree(selected, header->senderId, 1);
}

// Removes the sender of an RT_DME_RECORD_DESELECT_CONNECTION record from the connection.
static inline void rtDmeConnectionOnDeselectConnection(rtDmeConnection *connection,
                                                       const rtDmeWireHeader *header,
                                                       const unsigned short *connectionId) {
    rtDmeDevice *device = connection->device;
    rtDmeConnection *deselected;
    rtDmeConnectionSlot *slot;
    rtDmePeer *sender;

    if (*connectionId == 0) {
        return;
    }
    sender = rtDmePeerListFind(&device->peers, header->senderId);
    deselected = rtDmeDeviceFindConnection(device, *connectionId);
    if (deselected == NULL) {
        return;
    }
    if (sender == NULL) {
        return;
    }
    rtDmeEndpointListRemove(&deselected->peers, header->senderId);
    rtDmePeerSetConnection(sender, 0);
    if (device->activeConnectionId != deselected->id) {
        return;
    }
    slot = rtDmeConnectionFindSlot(deselected, header->senderId);
    if (slot == NULL) {
        return;
    }
    rtDmeConnectionSlotReset(slot);
    // Yes, the binary announces the free slot on the connection of the record.
    rtDmeConnectionSendSlotReleased(connection);
}

// NTSC-U/C: 0x002e7fd4, PAL: 0x002f1f44
static int rtDmeConnectionHandleControlRecord(rtDmeConnection *connection,
                                              const rtDmeWireHeader *header,
                                              rtDmePeer *peer,
                                              const unsigned char *record,
                                              int length) {
    const unsigned char *payload = &record[kWireHeaderSize];
    const unsigned short *connectionId = (const unsigned short *)payload;
    const rtDmeOpenConnectionRecord *open = (const rtDmeOpenConnectionRecord *)payload;
    rtDmeWireHeader reply;
    unsigned int now = rtDmeGetTime();
    int result = 0;
    int relay = 0;

    if (connection->id != 0 && rtDmeConnectionIsOverfull(connection)) {
        switch (header->type) {
        case RT_DME_RECORD_FORWARD:
        case RT_DME_RECORD_SLOTS_FREE:
        case RT_DME_RECORD_OPEN_CONNECTION:
        case RT_DME_RECORD_SELECT_CONNECTION:
        case RT_DME_RECORD_DESELECT_CONNECTION:
        case RT_DME_RECORD_SLOT_RELEASED:
        case RT_DME_RECORD_DISCONNECT:
            relay = 1;
            break;
        default:
            relay = 0;
            break;
        }
    }
    rtDmeConnectionForward(connection, header, peer, record, length, relay, now);

    switch (header->type) {
    case RT_DME_RECORD_PING:
        rtDmeConnectionInitHeader(connection, &reply, RT_DME_RECORD_PING_REPLY, 0);
        rtDmeDeviceSendRecord(connection->device, peer->info.id, &reply, NULL, 0);
        break;
    case RT_DME_RECORD_SLOTS_FREE:
        rtDmeConnectionOnSlotsFree(connection, header, (const rtDmeSlotsFreeRecord *)payload, now);
        break;
    case RT_DME_RECORD_OPEN_CONNECTION:
        rtDmeConnectionOnOpenConnection(connection, header, open);
        break;
    case RT_DME_RECORD_QUERY_CONNECTION:
        rtDmeConnectionOnQueryConnection(connection, header, connectionId);
        break;
    case RT_DME_RECORD_SLOT_OFFER:
        rtDmeConnectionOnSlotOffer(connection, header);
        break;
    case RT_DME_RECORD_SELECT_CONNECTION:
        result = rtDmeConnectionOnSelectConnection(connection, header, connectionId);
        break;
    case RT_DME_RECORD_DESELECT_CONNECTION:
        rtDmeConnectionOnDeselectConnection(connection, header, connectionId);
        break;
    case RT_DME_RECORD_CONNECTION_INFO:
        rtDmeConnectionOnConnectionInfo(connection, open);
        break;
    case RT_DME_RECORD_SLOT_ACCEPT:
        rtDmeConnectionOnSlotAccept(connection, header, now);
        break;
    case RT_DME_RECORD_SLOT_REVOKE:
        rtDmeConnectionOnSlotRevoke(connection, header);
        break;
    case RT_DME_RECORD_SLOT_RELEASED:
        result = rtDmeConnectionSendSlotsFree(connection, header->senderId, 0);
        break;
    case RT_DME_RECORD_DISCONNECT:
        rtDmeDeviceRemovePeer(connection->device, header->senderId);
        break;
    default:
        break;
    }
    return result;
}

// NTSC-U/C: 0x002e8958, PAL: 0x002f28c8
static void rtDmeConnectionUpdateTalk(rtDmeConnection *connection) {
    unsigned int i;

    for (i = 0; i < connection->slotCount; ++i) {
        if (connection->slots[i].state == RT_DME_SLOT_CONNECTED) {
            break;
        }
    }
    if (i >= connection->slotCount) {
        connection->talkAllowed = 0;
    } else if (connection->talkerId == -1 || connection->talkerId == connection->device->localId) {
        connection->talkAllowed = 1;
    } else if ((int)(rtDmeGetTime() - connection->talkTime) > kTalkTimeout) {
        connection->talkerId = -1;
        connection->talkAllowed = 1;
    } else if (rtDmeEndpointListCount(&connection->peers) < kTalkPeerLimit) {
        connection->talkAllowed = 1;
    } else {
        connection->talkAllowed = 0;
    }
    if (!connection->talkAllowed) {
        rtDmeDeviceResetAudioSequence(connection->device);
    }
}

// NTSC-U/C: 0x002e8928, PAL: 0x002f2898
static int rtDmeConnectionCheckTalk(rtDmeConnection *connection) {
    rtDmeConnectionUpdateTalk(connection);
    return connection->talkAllowed != 0 ? 0 : -1;
}

// NTSC-U/C: 0x002e8a64, PAL: 0x002f29d4
static int rtDmeConnectionSendSlotsFree(rtDmeConnection *connection, int peerId, int force) {
    rtDmeSlotsFreeRecord record;
    rtDmeWireHeader header;
    unsigned int i;

    record.senderId = connection->device->localId;
    record.connectionId = (unsigned short)connection->device->activeConnectionId;
    record.freeSlots = 0;
    record.size = sizeof(record);
    for (i = 0; i < connection->slotCount; ++i) {
        const rtDmeConnectionSlot *slot = &connection->slots[i];

        if (slot->state == RT_DME_SLOT_FREE || slot->peerId == peerId) {
            ++record.freeSlots;
        }
    }
    if (record.freeSlots == 0 && connection->maxPeers == 0) {
        ++record.freeSlots;
    }
    if (force == 0 && record.freeSlots == 0) {
        return 0;
    }
    rtDmeConnectionInitHeader(connection, &header, RT_DME_RECORD_SLOTS_FREE, sizeof(record));
    return rtDmeDeviceSendRecord(connection->device, peerId, &header, &record, sizeof(record));
}

// NTSC-U/C: 0x002e8b74, PAL: 0x002f2ae4
static rtDmeConnectionSlot *rtDmeConnectionFindSlot(rtDmeConnection *connection, int peerId) {
    unsigned int i;

    for (i = 0; i < connection->slotCount; ++i) {
        if (connection->slots[i].peerId == peerId) {
            return &connection->slots[i];
        }
    }
    return NULL;
}

// NTSC-U/C: 0x002e8bc0, PAL: 0x002f2b30
static int rtDmeConnectionSendSlotReleased(rtDmeConnection *connection) {
    rtDmeWireHeader header;

    if (connection == NULL || connection->id == 0) {
        return -1;
    }
    rtDmeConnectionInitHeader(connection, &header, RT_DME_RECORD_SLOT_RELEASED, 0);
    return rtDmeConnectionSend(connection, connection->device->localId, &header, NULL, 0);
}

// NTSC-U/C: 0x002e8c24, PAL: 0x002f2b94
static void rtDmeConnectionDisconnect(rtDmeConnection *connection) {
    rtDmeWireHeader header;
    unsigned int i;

    rtDmeConnectionInitHeader(connection, &header, RT_DME_RECORD_DISCONNECT, 0);
    rtDmeConnectionSend(connection, connection->device->localId, &header, NULL, 0);
    rtDmeEndpointListClear(&connection->peers);
    for (i = 0; i < connection->slotCount; ++i) {
        rtDmeConnectionSlotReset(&connection->slots[i]);
    }
}

// NTSC-U/C: 0x002e8cb8, PAL: 0x002f2c28
static void rtDmeConnectionResetSlots(rtDmeConnection *connection) {
    unsigned int i;

    for (i = 0; i < connection->slotCount; ++i) {
        rtDmeConnectionSlotReset(&connection->slots[i]);
    }
}

// NTSC-U/C: 0x002e8d18, PAL: 0x002f2c88
static void rtDmeConnectionAddPeer(rtDmeConnection *connection, rtDmePeer *peer) {
    rtDmeEndpointListAdd(&connection->peers, peer);
    if (connection->maxPeers == 0 &&
        connection->slotCount < (unsigned int)rtDmeEndpointListCount(&connection->peers)) {
        rtDmeConnectionGrowSlots(connection);
    }
}

// NTSC-U/C: 0x002e8d78, PAL: 0x002f2ce8
static void rtDmeConnectionRemovePeer(rtDmeConnection *connection, int peerId) {
    rtDmeConnectionSlot *slot = rtDmeConnectionFindSlot(connection, peerId);

    if (slot != NULL) {
        rtDmeConnectionSlotReset(slot);
        rtDmeConnectionSendSlotReleased(connection);
    }
    rtDmeEndpointListRemove(&connection->peers, peerId);
}

// NTSC-U/C: 0x002e8dcc, PAL: 0x002f2d3c
static void rtDmeConnectionGetTalker(rtDmeConnection *connection, rtDmePeerInfo *info) {
    rtDmePeer *talker;

    memset(info, 0, sizeof(*info));
    if (connection->talkerId == -1) {
        return;
    }
    if ((int)(rtDmeGetTime() - connection->talkTime) > kTalkTimeout) {
        connection->talkerId = -1;
        connection->talkAllowed = 1;
    }
    if (connection->talkerId == -1) {
        return;
    }
    talker = rtDmePeerListFind(&connection->device->peers, connection->talkerId);
    if (talker != NULL) {
        *info = talker->info;
    }
}

// NTSC-U/C: 0x002e8e98, PAL: 0x002f2e08
static rtDmePeer *rtDmePeerCreate(const rtDmePeerInfo *info) {
    void *block = NULL;
    rtDmePeer *peer;

    RtAllocZeroedBlock(&block, sizeof(rtDmePeer));
    peer = block;
    if (peer == NULL) {
        return NULL;
    }
    peer->info = *info;
    peer->state = RT_DME_PEER_UNRESOLVED;
    return peer;
}

// NTSC-U/C: 0x002e8f10, PAL: 0x002f2e80
static void rtDmePeerDestroy(rtDmePeer *peer) {
    void *block = peer;

    if (peer == NULL) {
        return;
    }
    if (peer->surface != NULL) {
        rtImageDestroySurface(peer->surface);
        peer->surface = NULL;
    }
    if (peer->videoFragments != NULL) {
        rtReassemblerDestroy(peer->videoFragments);
        peer->videoFragments = NULL;
    }
    RtFreeBlock(&block);
}

// NTSC-U/C: 0x002e8f70, PAL: 0x002f2ee0
static void rtDmePeerSetConnection(rtDmePeer *peer, int connectionId) {
    peer->info.connectionId = connectionId;
}

// NTSC-U/C: 0x002e8f78, PAL: 0x002f2ee8
static rtDmePeer *rtDmePeerListAdd(rtDmePeerList *list, const rtDmePeerInfo *info) {
    rtDmePeer *peer = rtDmePeerListFind(list, info->id);
    rtDmePeer *head;

    if (peer != NULL) {
        rtDmePeerSetConnection(peer, info->connectionId);
        return peer;
    }
    peer = rtDmePeerCreate(info);
    if (list->head == NULL) {
        list->head = peer;
    }
    head = list->head;
    peer->next = head;
    peer->prev = list->head->prev;
    head->prev = peer;
    peer->prev->next = peer;
    return peer;
}

// NTSC-U/C: 0x002e9010, PAL: 0x002f2f80
static void rtDmePeerListRemove(rtDmePeerList *list, int id) {
    rtDmePeer *peer = rtDmePeerListFind(list, id);

    if (peer == NULL) {
        return;
    }
    if (peer == list->head) {
        list->head = peer->next != peer ? peer->next : NULL;
    }
    peer->prev->next = peer->next;
    peer->next->prev = peer->prev;
    rtDmePeerDestroy(peer);
}

// NTSC-U/C: 0x002e9078, PAL: 0x002f2fe8
static rtDmePeer *rtDmePeerListFind(rtDmePeerList *list, int id) {
    rtDmePeer *peer = list->head;

    while (peer != NULL && peer->info.id != id) {
        peer = peer->next;
        if (peer == list->head) {
            peer = NULL;
        }
    }
    return peer;
}

// NTSC-U/C: 0x002e90bc, PAL: 0x002f302c
static void rtDmePeerListClear(rtDmePeerList *list) {
    while (list->head != NULL) {
        rtDmePeerListRemove(list, list->head->info.id);
    }
}

// NTSC-U/C: 0x002e9184, PAL: 0x002f30f4
static rtDmeEndpointNode *rtDmeEndpointListFind(const rtDmeEndpointList *list, int id) {
    rtDmeEndpointNode *node = list->head;

    while (node != NULL) {
        if (node->peer->info.id == id) {
            return node;
        }
        node = node->next;
        if (node == list->head) {
            node = NULL;
        }
    }
    return NULL;
}

// NTSC-U/C: 0x002e9104, PAL: 0x002f3074
static void rtDmeEndpointListAdd(rtDmeEndpointList *list, rtDmePeer *peer) {
    rtDmeEndpointNode *node;
    rtDmeEndpointNode *head;

    if (rtDmeEndpointListFind(list, peer->info.id) != NULL) {
        return;
    }
    node = rtDmeEndpointNodeCreate(peer);
    if (list->head == NULL) {
        list->head = node;
    }
    head = list->head;
    node->next = head;
    node->prev = list->head->prev;
    head->prev = node;
    node->prev->next = node;
    ++list->count;
}

// NTSC-U/C: 0x002e91c4, PAL: 0x002f3134
static int rtDmeEndpointListCount(const rtDmeEndpointList *list) {
    return list->count;
}

// NTSC-U/C: 0x002e91cc, PAL: 0x002f313c
static void rtDmeEndpointListRemove(rtDmeEndpointList *list, int id) {
    rtDmeEndpointNode *node = rtDmeEndpointListFind(list, id);

    if (node == NULL) {
        return;
    }
    if (node == list->head) {
        list->head = node->next != node ? node->next : NULL;
    }
    node->prev->next = node->next;
    node->next->prev = node->prev;
    rtDmeEndpointNodeDestroy(node);
    --list->count;
}

// NTSC-U/C: 0x002e9240, PAL: 0x002f31b0
static void rtDmeEndpointListClear(rtDmeEndpointList *list) {
    while (list->head != NULL) {
        rtDmeEndpointListRemove(list, list->head->peer->info.id);
    }
}

// NTSC-U/C: 0x002e928c, PAL: 0x002f31fc
static unsigned int rtDmeReadCycleCounter(void) {
    unsigned int count;

    __asm__ volatile("mfc0 %0, $9\n\tsync.p" : "=r"(count));
    return count;
}

// NTSC-U/C: 0x002e929c, PAL: 0x002f320c
static unsigned int rtDmeGetTime(void) {
    unsigned int cycles = rtDmeReadCycleCounter();
    unsigned int milliseconds;

    if (g_nMonotonicTickCount == 1 && g_nTickClockBase == 0) {
        g_nTickClockBase = cycles;
    }
    if ((int)(cycles - g_nTickClockBase) >= kCyclesPerMillisecond) {
        milliseconds = (cycles - g_nTickClockBase) / kCyclesPerMillisecond;
        g_nMonotonicTickCount += milliseconds;
        g_nTickClockBase += milliseconds * kCyclesPerMillisecond;
    } else if ((int)(cycles - g_nTickClockBase) < 0) {
        // Yes, the binary divides the negative difference as an unsigned value.
        g_nMonotonicTickCount += (cycles - g_nTickClockBase) / kCyclesPerMillisecond;
        g_nTickClockBase = cycles;
    }
    if (g_nMonotonicTickCount == 0) {
        return 1;
    }
    return g_nMonotonicTickCount;
}

// NTSC-U/C: 0x002e93a4, PAL: 0x002f3314
static void rtDmeAlarmWakeThread(int id, unsigned short time, void *arg) {
    (void)id;
    (void)time;
    iWakeupThread((int)arg);
}

// NTSC-U/C: 0x002e93c0, PAL: 0x002f3330
static void rtDmeSleep(int ticks) {
    int thread = GetThreadId();

    SetAlarm((unsigned short)ticks, rtDmeAlarmWakeThread, (void *)thread);
    SleepThread();
}

// NTSC-U/C: 0x002e9400, PAL: 0x002f3370
static rtDmeSlotTable *rtDmeSlotTableCreate(rtDmeDevice *device) {
    void *block = NULL;
    rtDmeSlotTable *table;
    unsigned int i;

    RtAllocZeroedBlock(&block, sizeof(rtDmeSlotTable));
    table = block;
    if (table == NULL) {
        return NULL;
    }
    table->device = device;
    table->codec = 0;
    table->state = kSlotTableFilling;
    table->senderId = -1;
    table->nextSequence = 0;
    table->count = 0;
    for (i = 0; i < RT_DME_AUDIO_FRAGMENT_COUNT; ++i) {
        table->fragments[i].sequence = 0;
        table->fragments[i].length = 0;
    }
    return table;
}

// NTSC-U/C: 0x002e94a4, PAL: 0x002f3414
static void rtDmeSlotTableDestroy(rtDmeSlotTable *table) {
    void *block = table;

    if (table != NULL) {
        RtFreeBlock(&block);
    }
}

// NTSC-U/C: 0x002e94cc, PAL: 0x002f343c
static void rtDmeSlotTableReset(rtDmeSlotTable *table, int senderId) {
    unsigned int i;

    if (table->senderId == senderId) {
        return;
    }
    table->senderId = senderId;
    table->state = kSlotTableFilling;
    table->nextSequence = 0;
    for (i = 0; i < RT_DME_AUDIO_FRAGMENT_COUNT; ++i) {
        table->fragments[i].sequence = 0;
        table->fragments[i].length = 0;
    }
}

// NTSC-U/C: 0x002e9520, PAL: 0x002f3490
static void rtDmeSlotTableStore(rtDmeSlotTable *table,
                                int codec,
                                int senderId,
                                int sequence,
                                const unsigned char *data,
                                int length) {
    rtDmeAudioFragment *fragment;
    unsigned int index = sequence & kFragmentMask;

    if ((unsigned int)length >= RT_DME_AUDIO_FRAGMENT_SIZE) {
        return;
    }
    if (length == 0) {
        rtDmeSlotTableReset(table, -1);
        return;
    }
    table->codec = codec;
    if (table->senderId != senderId) {
        rtDmeSlotTableReset(table, senderId);
        switch (table->codec) {
        case RT_DME_AUDIO_SUBBAND:
            rtDmeDeviceResetSubbandDecoder(table->device);
            break;
        case RT_DME_AUDIO_LPC:
            rtDmeDeviceResetLpc(table->device);
            break;
        case RT_DME_AUDIO_SPEECH:
            rtDmeDeviceResetSpeech(table->device);
            break;
        default:
            break;
        }
    }

    if (sequence - table->nextSequence < 0) {
        if (table->nextSequence - sequence < kFragmentWindow) {
            return;
        }
        table->nextSequence = sequence;
    } else if (sequence - table->nextSequence >= kFragmentWindow) {
        if (sequence - table->nextSequence >= kFragmentDropWindow) {
            return;
        }
        table->nextSequence = sequence;
    }
    // Yes, the binary drops fragments outside a window the assignments above already closed.
    while (sequence - table->nextSequence >= kFragmentWindow) {
        fragment = &table->fragments[table->nextSequence & kFragmentMask];
        if (fragment->length != 0) {
            fragment->length = 0;
            --table->count;
            table->state = kSlotTableSkipping;
        }
        ++table->nextSequence;
    }

    fragment = &table->fragments[index];
    if (fragment->length != 0) {
        fragment->length = 0;
        --table->count;
    }
    fragment->sequence = sequence;
    if (length != 0) {
        memcpy(fragment->data, data, length);
        fragment->length = length;
        ++table->count;
    }
    if (table->state == kSlotTableFilling && table->count >= kPrebufferFragments) {
        table->state = kSlotTablePlaying;
    }
}

// NTSC-U/C: 0x002e9748, PAL: 0x002f36b8
static int rtDmeSlotTableRead(rtDmeSlotTable *table, int *codec, void *buffer, int size) {
    unsigned char *output = buffer;
    unsigned int start = table->nextSequence & kFragmentMask;
    unsigned int index = start;
    unsigned int remaining = size;
    unsigned int total = 0;
    rtDmeAudioFragment *fragment;
    unsigned int length;

    *codec = table->codec;
    if (table->state == kSlotTableFilling) {
        return 0;
    }
    if (remaining != 0) {
        fragment = &table->fragments[index];
        length = fragment->length;
        while (remaining >= length) {
            if (length != 0) {
                memcpy(&output[total], fragment->data, length);
                fragment->length = 0;
                total += length;
                remaining -= length;
                --table->count;
            } else if (table->state != kSlotTableSkipping) {
                return total;
            }
            ++table->nextSequence;
            index = table->nextSequence & kFragmentMask;
            if (index == start || remaining == 0) {
                break;
            }
            fragment = &table->fragments[index];
            length = fragment->length;
        }
    }
    if (table->state == kSlotTableSkipping && table->count < kRecoverFragments) {
        table->state = kSlotTablePlaying;
    }
    return total;
}

// NTSC-U/C: 0x002e989c, PAL: 0x002f380c
static rtSubbandCodec *rtDmeSubbandCodecCreate(void) {
    void *block = NULL;

    RtAllocZeroedBlock(&block, sizeof(rtSubbandCodec));
    if (block != NULL) {
        rtDmeSubbandCodecReset(block);
    }
    return block;
}

// NTSC-U/C: 0x002e98d8, PAL: 0x002f3848
static void rtDmeSubbandCodecDestroy(rtSubbandCodec *codec) {
    void *block = codec;

    if (codec != NULL) {
        RtFreeBlock(&block);
    }
}

// NTSC-U/C: 0x002e9900, PAL: 0x002f3870
static void rtDmeSubbandCodecReset(rtSubbandCodec *codec) {
    memset(codec, 0, sizeof(*codec));
    codec->previousLag = RT_SUBBAND_INITIAL_LAG;
}
