#ifndef LIBRTDMEDEV_H
#define LIBRTDMEDEV_H

#include <librtcodec.h>
#include <librtimage.h>
#include <librtnet.h>
#include <librtreasm.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Peer media device of the SCE-RT network library. The device exchanges wire records, audio, and
 * video with the peers of its connections over one datagram socket.
 *
 * The names of the types, the routines, and their parameters are inferred.
 */

/** Bit of the type of a wire record set when the receiver finds the sender by its identifier. */
#define RT_DME_RECORD_IDENTIFIED 0x80

/** Types of wire records. */
#define RT_DME_RECORD_PING 0x01              /*!< A probe of a peer that has gone quiet. */
#define RT_DME_RECORD_PING_REPLY 0x02        /*!< The reply to RT_DME_RECORD_PING. */
#define RT_DME_RECORD_FORWARD 0x03           /*!< A record a full connection relays to its peers. */
#define RT_DME_RECORD_SLOTS_FREE 0x05        /*!< The free peer slots of the sender. */
#define RT_DME_RECORD_OPEN_CONNECTION 0x06   /*!< A peer opened a named connection. */
#define RT_DME_RECORD_QUERY_CONNECTION 0x07  /*!< A request for the details of a connection. */
#define RT_DME_RECORD_SLOT_OFFER 0x08        /*!< The sender reserved a slot for the receiver. */
#define RT_DME_RECORD_SELECT_CONNECTION 0x09 /*!< The sender selected a connection. */
#define RT_DME_RECORD_DESELECT_CONNECTION 0x0a /*!< The sender deselected a connection. */
#define RT_DME_RECORD_CONNECTION_INFO 0x0b     /*!< The details of a connection. */
#define RT_DME_RECORD_SLOT_ACCEPT 0x0c         /*!< The sender took an offered peer slot. */
#define RT_DME_RECORD_SLOT_REVOKE 0x0e   /*!< The sender dropped the receiver from its slots. */
#define RT_DME_RECORD_SLOT_RELEASED 0x0f /*!< The sender freed a peer slot. */
#define RT_DME_RECORD_DISCONNECT 0x10    /*!< The sender departed from every connection. */
#define RT_DME_RECORD_AUDIO_PCM 0x20     /*!< Uncompressed samples. */
#define RT_DME_RECORD_AUDIO_SUBBAND 0x21 /*!< Samples of the sub-band coder. */
#define RT_DME_RECORD_AUDIO_LPC 0x22     /*!< Samples of the linear prediction coder. */
#define RT_DME_RECORD_AUDIO_SPEECH 0x23  /*!< Samples of the speech coder. */
#define RT_DME_RECORD_AUDIO_RAW 0x2f     /*!< Audio bytes of an unknown codec, sent as they are. */
#define RT_DME_RECORD_VIDEO 0x30         /*!< An encoded video frame. */

/** Codecs of the audio the capture callback reports. */
#define RT_DME_AUDIO_PCM 0x21     /*!< Uncompressed samples. */
#define RT_DME_AUDIO_SUBBAND 0x22 /*!< The sub-band coder. */
#define RT_DME_AUDIO_LPC 0x23     /*!< The linear prediction coder. */
#define RT_DME_AUDIO_SPEECH 0x24  /*!< The speech coder. */
#define RT_DME_AUDIO_RAW 0x3f     /*!< Bytes of an unknown codec, as the sender captured them. */

/** States of a peer slot of a connection. */
#define RT_DME_SLOT_FREE 0      /*!< The slot is unused. */
#define RT_DME_SLOT_OFFERED 1   /*!< The slot waits for the peer to accept it. */
#define RT_DME_SLOT_CONNECTED 2 /*!< The peer exchanges records directly with this console. */

/** The fragments of audio a device buffers for playback. */
#define RT_DME_AUDIO_FRAGMENT_COUNT 64

/** The size of the buffer of one fragment of audio. */
#define RT_DME_AUDIO_FRAGMENT_SIZE 248

/** States of a peer, set by the first record that arrives from one of its addresses. */
#define RT_DME_PEER_UNRESOLVED 0 /*!< No record has arrived yet. */
#define RT_DME_PEER_LOCAL 1      /*!< Records arrive from the local address. */
#define RT_DME_PEER_PUBLIC 2     /*!< Records arrive from the public address. */

/** Bit of rtDmePeer::flags set while a decoded video frame waits for display. */
#define RT_DME_PEER_VIDEO_READY 0x40000000

/** States of the port mapping of a device. */
#define RT_DME_UPNP_IDLE 0    /*!< No mapping was requested. */
#define RT_DME_UPNP_PENDING 1 /*!< The mapping request is running. */
#define RT_DME_UPNP_MAPPED 2  /*!< The router mapped the port. */

/** The wire records a device remembers to drop repeated records. */
#define RT_DME_RECORD_HISTORY_SIZE 4096

/** The size of a video frame buffer of a peer. */
#define RT_DME_PEER_FRAME_SIZE 0x800

/** The size of the audio a device buffers until a full frame for the encoder is available. */
#define RT_DME_AUDIO_ENCODE_BUFFER_SIZE 0x800

/** The size of the encoded audio a device buffers until a full frame for the decoder is available.
 */
#define RT_DME_AUDIO_DECODE_BUFFER_SIZE 0x40

/** The size of a connection name, the terminator included. */
#define RT_DME_CONNECTION_NAME_SIZE 0x40

/** The size of the name in an RT_DME_RECORD_OPEN_CONNECTION record, the terminator included. */
#define RT_DME_OPEN_CONNECTION_NAME_SIZE 0x3c

/** The size of the dotted address of the router of a port mapping, the terminator included. */
#define RT_DME_UPNP_HOST_SIZE 16

typedef struct rtDmeDevice rtDmeDevice;
typedef struct rtDmeConnection rtDmeConnection;
typedef struct rtDmePeer rtDmePeer;
typedef struct rtDmeEndpointNode rtDmeEndpointNode;

/** A fragment of audio waiting for playback. */
typedef struct {
    int sequence;                                   /*!< The sequence number of the fragment. */
    unsigned int length;                            /*!< The bytes in data, or 0 for none. */
    unsigned char data[RT_DME_AUDIO_FRAGMENT_SIZE]; /*!< The encoded audio. */
} rtDmeAudioFragment;

/** The audio fragments of the peer that talks, ordered for playback. */
typedef struct {
    rtDmeDevice *device; /*!< The device. */
    int state;           /*!< Whether the table fills, plays, or skips missing fragments. */
    int codec;           /*!< The codec of the fragments. */
    int senderId;        /*!< The peer the fragments come from, or -1. */
    int nextSequence;    /*!< The sequence number of the next fragment to play. */
    int count;           /*!< The fragments with audio. */
    rtDmeAudioFragment fragments[RT_DME_AUDIO_FRAGMENT_COUNT]; /*!< The fragments by sequence. */
} rtDmeSlotTable;

/** The header of every wire record. */
typedef struct {
    unsigned short type; /*!< One of the RT_DME_RECORD_ types, with RT_DME_RECORD_IDENTIFIED. */
    unsigned short connectionId; /*!< The connection, or 0 for the device itself. */
    unsigned int length;         /*!< The bytes of the payload. */
    int senderId;                /*!< The peer that sent the record. */
    unsigned int sequence;       /*!< The sequence number of the record on its connection. */
} rtDmeWireHeader;

/** The payload of an RT_DME_RECORD_OPEN_CONNECTION record. */
typedef struct {
    unsigned short size;                         /*!< The bytes of the payload. */
    unsigned short connectionId;                 /*!< The connection. */
    unsigned short maxPeers;                     /*!< The peers the connection takes. */
    unsigned short reserved06;                   /* +0x06, written with maxPeers. */
    char name[RT_DME_OPEN_CONNECTION_NAME_SIZE]; /*!< The name of the connection. */
} rtDmeOpenConnectionRecord;

/** The payload of an RT_DME_RECORD_SLOTS_FREE record. */
typedef struct {
    unsigned short size;         /*!< The bytes of the payload. */
    unsigned short connectionId; /*!< The connection the sender selected. */
    unsigned short freeSlots;    /*!< The slots the sender can give the receiver. */
    unsigned short reserved06;   /* +0x06, never written. */
    int senderId;                /*!< The sender. */
} rtDmeSlotsFreeRecord;

/** The addresses and identifiers that describe a peer. */
typedef struct {
    unsigned int publicAddress; /*!< The address of the peer as the Internet sees it. */
    unsigned int localAddress;  /*!< The address of the peer on its network. */
    int publicPort;             /*!< The port that goes with publicAddress. */
    int localPort;              /*!< The port that goes with localAddress. */
    int connectionId;           /*!< The connection the peer selected, or 0. */
    int id;                     /*!< The identifier of the peer. */
} rtDmePeerInfo;

/** A peer of a device, in a circular list. */
struct rtDmePeer {
    rtDmePeer *prev;                             /*!< The previous peer. */
    rtDmePeer *next;                             /*!< The next peer. */
    unsigned int state;                          /*!< One of the RT_DME_PEER_ states. */
    rtDmePeerInfo info;                          /*!< The addresses and identifiers. */
    unsigned int lastHeard;                      /*!< The tick of the last record of the peer. */
    unsigned int retries;                        /*!< Pings sent since the last record. */
    rtImage *surface;                            /*!< The pixels of the last video frame. */
    rtReassembler *videoFragments;               /*!< The fragments of video frames. */
    unsigned int flags;                          /*!< A combination of RT_DME_PEER_ bits. */
    unsigned int videoSequence;                  /*!< The sequence number after the last frame. */
    int frameLength;                             /*!< The bytes in frame. */
    unsigned char frame[RT_DME_PEER_FRAME_SIZE]; /*!< A complete video frame. */
};

/** The circular list of the peers of a device. */
typedef struct {
    rtDmePeer *head; /*!< The first peer, or null. */
} rtDmePeerList;

/** An entry of the circular list of the peers of a connection. */
struct rtDmeEndpointNode {
    rtDmeEndpointNode *prev; /*!< The previous entry. */
    rtDmeEndpointNode *next; /*!< The next entry. */
    rtDmePeer *peer;         /*!< The peer. */
};

/** The circular list of the peers of a connection. */
typedef struct {
    int count;               /*!< The number of entries. */
    rtDmeEndpointNode *head; /*!< The first entry, or null. */
} rtDmeEndpointList;

/** A slot of the peer table of a connection. */
typedef struct {
    int peerId;             /*!< The peer, or -1 for a free slot. */
    rtDmePeer *peer;        /*!< The peer, or null. */
    int state;              /*!< One of the RT_DME_SLOT_ states. */
    unsigned int lastHeard; /*!< The tick of the last record of the peer. */
    unsigned int retries;   /*!< Pings sent since the last record. */
} rtDmeConnectionSlot;

/** A connection of a device, in a circular list. */
struct rtDmeConnection {
    rtDmeConnection *prev;                  /*!< The previous connection. */
    rtDmeConnection *next;                  /*!< The next connection. */
    rtDmeDevice *device;                    /*!< The device. */
    rtDmeEndpointList peers;                /*!< The peers of the connection. */
    unsigned int sequence;                  /*!< The sequence number of the next record. */
    int talkAllowed;                        /*!< Nonzero while this console may send audio. */
    int talkerId;                           /*!< The peer whose audio arrived last, or -1. */
    unsigned int talkTime;                  /*!< The tick of the last audio, or of creation. */
    int id;                                 /*!< The identifier of the connection. */
    char name[RT_DME_CONNECTION_NAME_SIZE]; /*!< The name, empty for the device connection. */
    unsigned int selectTime;                /*!< The tick this console last selected it. */
    unsigned int offerTime;                 /*!< The tick pendingSlots arrived, or 0. */
    rtDmeSlotsFreeRecord pendingSlots;      /*!< The record of a peer that had no free slots. */
    int maxPeers;                           /*!< The peers the connection takes, or 0. */
    unsigned int slotCount;                 /*!< The entries of slots. */
    rtDmeConnectionSlot *slots;             /*!< The peer table. */
};

/**
 * Callback that supplies captured audio.
 *
 * @param device The device.
 * @param codec Receives one of the RT_DME_AUDIO_ codecs, or another value for bytes the device
 * sends as they are.
 * @param buffer Receives the audio.
 * @param size The size of the buffer in bytes.
 * @return The bytes written, or 0 when no audio is available.
 */
typedef int (*rtDmeAudioCaptureFunc)(rtDmeDevice *device, int *codec, void *buffer, int size);

/**
 * Callback that plays the audio of a peer.
 *
 * @param device The device.
 * @param codec The codec the audio arrived in.
 * @param samples The samples, or the bytes of an unknown codec.
 * @param length The number of bytes.
 */
typedef void (*rtDmeAudioPlayFunc)(rtDmeDevice *device, int codec, const void *samples, int length);

/**
 * Callback that supplies a captured video frame.
 *
 * @param device The device.
 * @param y Receives the luma plane.
 * @param u Receives the first chroma plane.
 * @param v Receives the second chroma plane.
 * @return A positive value when a frame was captured.
 */
typedef int (*rtDmeVideoCaptureFunc)(rtDmeDevice *device, rtImage *y, rtImage *u, rtImage *v);

/**
 * Callback that displays the video frame of a peer.
 *
 * @param device The device.
 * @param peerId The peer.
 * @param surface The pixels.
 */
typedef void (*rtDmeVideoDisplayFunc)(rtDmeDevice *device, int peerId, rtImage *surface);

/**
 * Callback that receives the public address of a finished port mapping.
 *
 * @param device The device.
 * @param address The public address.
 * @param port The public port.
 */
typedef void (*rtDmePortMappedFunc)(rtDmeDevice *device, unsigned int address, int port);

/** The parameters of rtDmeDeviceInit(). */
typedef struct {
    int port;                           /*!< The local port of the socket. */
    int reserved04;                     /* +0x04, copied to rtDmeDevice::reserved18. */
    int reserved08;                     /* +0x08, copied to rtDmeDevice::reserved08. */
    int reserved0C;                     /* +0x0c, copied to rtDmeDevice::reserved10. */
    int reserved10;                     /* +0x10, copied to rtDmeDevice::reserved1C. */
    int reserved14;                     /* +0x14, copied to rtDmeDevice::reserved0C. */
    int reserved18;                     /* +0x18, copied to rtDmeDevice::reserved14. */
    int width;                          /*!< The width of the video in pixels. */
    int height;                         /*!< The height of the video in pixels. */
    rtDmeAudioCaptureFunc captureAudio; /*!< Supplies audio, or null. */
    rtDmeAudioPlayFunc playAudio;       /*!< Plays audio, or null. */
    rtDmeVideoCaptureFunc captureVideo; /*!< Supplies video, or null. */
    rtDmeVideoDisplayFunc displayVideo; /*!< Displays video, or null. */
} rtDmeDeviceParams;

/** A peer media device. */
struct rtDmeDevice {
    unsigned int audioSequence;         /*!< The sequence number of the next audio frame. */
    unsigned int videoSequence;         /*!< The sequence number of the next video frame. */
    int reserved08;                     /* +0x08 */
    int reserved0C;                     /* +0x0c */
    int reserved10;                     /* +0x10 */
    int reserved14;                     /* +0x14 */
    int reserved18;                     /* +0x18 */
    int reserved1C;                     /* +0x1c */
    unsigned int audioBytesEncoded;     /*!< The bytes of captured audio encoded. */
    unsigned int audioBytesDecoded;     /*!< The bytes of audio decoded for playback. */
    unsigned int audioEncodeTime;       /*!< The ticks spent encoding audio. */
    unsigned int audioDecodeTime;       /*!< The ticks spent decoding audio. */
    unsigned int videoFramesEncoded;    /*!< The video frames encoded. */
    unsigned int videoFramesDecoded;    /*!< The video frames decoded. */
    unsigned int videoEncodeTime;       /*!< The ticks spent encoding video. */
    unsigned int videoDecodeTime;       /*!< The ticks spent decoding video. */
    rtDmeAudioCaptureFunc captureAudio; /*!< Supplies audio, or null. */
    rtDmeAudioPlayFunc playAudio;       /*!< Plays audio, or null. */
    rtDmeVideoCaptureFunc captureVideo; /*!< Supplies video, or null. */
    rtDmeVideoDisplayFunc displayVideo; /*!< Displays video, or null. */
    rtSpeechCodec *speechCodecs[2];     /*!< The speech coder objects. */
    rtSubbandCodec *subbandEncoder;     /*!< The sub-band coder of outgoing audio. */
    rtSubbandCodec *subbandDecoder;     /*!< The sub-band coder of incoming audio. */
    rtDmeConnection *connections;       /*!< The connections, the device connection first. */
    rtDmeSlotTable *slotTable;          /*!< The audio fragments of the peers. */
    rtDmePeer *localPeer;               /*!< The peer of this console. */
    rtDmePeerList peers;                /*!< The peers, this console first. */
    unsigned int flags;                 /*!< A combination of the device flags. */
    int localId;                        /*!< The identifier of this console. */
    unsigned int localAddress;          /*!< The address of this console on its network. */
    int localPort;                      /*!< The local port of the socket. */
    unsigned int publicAddress;         /*!< The address the port mapping reported. */
    int publicPort;                     /*!< The port the port mapping reported. */
    unsigned int historyIndex;          /*!< The next entry of history. */
    rtDmeWireHeader history[RT_DME_RECORD_HISTORY_SIZE]; /*!< The last records received. */
    unsigned int audioEncodeLength;                      /*!< The bytes in audioEncodeBuffer. */
    unsigned int audioDecodeLength;                      /*!< The bytes in audioDecodeBuffer. */
    unsigned char audioEncodeBuffer[RT_DME_AUDIO_ENCODE_BUFFER_SIZE]; /*!< Audio to encode. */
    unsigned char audioDecodeBuffer[RT_DME_AUDIO_DECODE_BUFFER_SIZE]; /*!< Audio to decode. */
    rtNetSocket *socket;                                              /*!< The socket, or null. */
    int audioCodec;                       /*!< The codec of the last captured audio. */
    int activeConnectionId;               /*!< The selected connection, or 0. */
    int width;                            /*!< The width of the video in pixels. */
    int height;                           /*!< The height of the video in pixels. */
    rtImage *captureY;                    /*!< The luma plane of captured video. */
    rtImage *captureU;                    /*!< The first chroma plane of captured video. */
    rtImage *captureV;                    /*!< The second chroma plane of captured video. */
    rtImage *displayY;                    /*!< The luma plane of decoded video. */
    rtImage *displayU;                    /*!< The first chroma plane of decoded video. */
    rtImage *displayV;                    /*!< The second chroma plane of decoded video. */
    int upnpState;                        /*!< One of the RT_DME_UPNP_ states. */
    struct rtUpnpContext *upnp;           /*!< The port mapping request, or null. */
    int upnpResult;                       /*!< The result of the port mapping. */
    char upnpHost[RT_DME_UPNP_HOST_SIZE]; /*!< The dotted address of the router. */
    unsigned short upnpPort;              /*!< The port of the router. */
    rtDmePortMappedFunc onPortMapped;     /*!< Receives the public address, or null. */
};

/**
 * Allocate a cleared device.
 *
 * @return The device, or null.
 * @ghidraAddress NTSC-U/C: 0x002e4840
 * @ghidraAddress PAL: 0x002ee7b0
 */
rtDmeDevice *rtDmeDeviceCreate(void);

/**
 * Release the coders, connections, peers, socket, and memory of a device.
 *
 * @param device The device.
 * @ghidraAddress NTSC-U/C: 0x002e48c4
 * @ghidraAddress PAL: 0x002ee834
 */
void rtDmeDeviceDestroy(rtDmeDevice *device);

/**
 * Restart the sequence of audio frames.
 *
 * @param device The device.
 * @ghidraAddress NTSC-U/C: 0x002e48f8
 * @ghidraAddress PAL: 0x002ee868
 */
void rtDmeDeviceResetAudioSequence(rtDmeDevice *device);

/**
 * Restart the sequence of video frames.
 *
 * @param device The device.
 * @ghidraAddress NTSC-U/C: 0x002e4900
 * @ghidraAddress PAL: 0x002ee870
 */
void rtDmeDeviceResetVideoSequence(rtDmeDevice *device);

/**
 * Take the next sequence number of audio frames.
 *
 * @param device The device.
 * @return The sequence number.
 * @ghidraAddress NTSC-U/C: 0x002e4908
 * @ghidraAddress PAL: 0x002ee878
 */
unsigned int rtDmeDeviceNextAudioSequence(rtDmeDevice *device);

/**
 * Take the next sequence number of video frames.
 *
 * @param device The device.
 * @return The sequence number.
 * @ghidraAddress NTSC-U/C: 0x002e491c
 * @ghidraAddress PAL: 0x002ee88c
 */
unsigned int rtDmeDeviceNextVideoSequence(rtDmeDevice *device);

/**
 * Set up the coders, the socket, and the peer of this console.
 *
 * @param device The device.
 * @param params The parameters.
 * @return 0, 1 when the device is already set up, or -1 when the socket fails.
 * @ghidraAddress NTSC-U/C: 0x002e4934
 * @ghidraAddress PAL: 0x002ee8a4
 */
int rtDmeDeviceInit(rtDmeDevice *device, const rtDmeDeviceParams *params);

/**
 * Request a port mapping from a router.
 *
 * Without a callback, the routine updates the device until the request finishes.
 *
 * @param device The device.
 * @param host The dotted address of the router.
 * @param port The port of the router.
 * @param onPortMapped Receives the public address, or null.
 * @return 0, or -1 when the request does not start.
 * @ghidraAddress NTSC-U/C: 0x002e4e34
 * @ghidraAddress PAL: 0x002eeda4
 */
int rtDmeDeviceMapPort(rtDmeDevice *device,
                       const char *host,
                       unsigned short port,
                       rtDmePortMappedFunc onPortMapped);

/**
 * Deselect the active connection and disconnect from the peers.
 *
 * @param device The device.
 * @ghidraAddress NTSC-U/C: 0x002e4fbc
 * @ghidraAddress PAL: 0x002eef2c
 */
void rtDmeDeviceStop(rtDmeDevice *device);

/**
 * Receive and dispatch wire records, service the peers, and move audio and video.
 *
 * @param device The device.
 * @return 0, or -1 when receiving or dispatching fails.
 * @ghidraAddress NTSC-U/C: 0x002e501c
 * @ghidraAddress PAL: 0x002eef8c
 */
int rtDmeDeviceUpdate(rtDmeDevice *device);

/**
 * Open a named connection, select it, and announce it to the peers.
 *
 * @param device The device.
 * @param maxPeers The peers the connection takes.
 * @param connectionId Receives the identifier of the connection.
 * @param name The name of the connection.
 * @return The result of sending the announcement, or 1 when the name is empty or taken.
 * @ghidraAddress NTSC-U/C: 0x002e541c
 * @ghidraAddress PAL: 0x002ef38c
 */
int rtDmeDeviceOpenConnection(rtDmeDevice *device,
                              int maxPeers,
                              int *connectionId,
                              const char *name);

/**
 * Send a wire record to the addresses of a peer.
 *
 * @param device The device.
 * @param peerId The peer.
 * @param header The header of the record.
 * @param payload The payload.
 * @param length The bytes of the payload.
 * @return 0, or -1 when the record is too large, the peer is unknown, or a send fails.
 * @ghidraAddress NTSC-U/C: 0x002e595c
 * @ghidraAddress PAL: 0x002ef8cc
 */
int rtDmeDeviceSendRecord(rtDmeDevice *device,
                          int peerId,
                          const rtDmeWireHeader *header,
                          const void *payload,
                          unsigned int length);

/**
 * Send a complete wire record to the addresses of a peer.
 *
 * @param device The device.
 * @param record The record.
 * @param length The bytes of the record.
 * @param peer The peer.
 * @return length, the bytes of the last short send, or -1 when an address has no port.
 * @ghidraAddress NTSC-U/C: 0x002e5b84
 * @ghidraAddress PAL: 0x002efaf4
 */
int rtDmeDeviceSendToPeer(rtDmeDevice *device,
                          const void *record,
                          int length,
                          const rtDmePeer *peer);

/**
 * Create a connection and append it to the connections of a device.
 *
 * @param device The device.
 * @param id The identifier of the connection.
 * @param maxPeers The peers the connection takes.
 * @return The connection.
 * @ghidraAddress NTSC-U/C: 0x002e5d50
 * @ghidraAddress PAL: 0x002efcc0
 */
rtDmeConnection *rtDmeDeviceAddConnection(rtDmeDevice *device, int id, int maxPeers);

/**
 * Find a connection of a device.
 *
 * @param device The device.
 * @param id The identifier of the connection.
 * @return The connection, or null.
 * @ghidraAddress NTSC-U/C: 0x002e5db0
 * @ghidraAddress PAL: 0x002efd20
 */
rtDmeConnection *rtDmeDeviceFindConnection(rtDmeDevice *device, int id);

/**
 * Deselect the active connection and announce the departure to its peers.
 *
 * @param device The device.
 * @param connectionId The active connection.
 * @return 0, -1 when the connection is not the active one, or the error of the announcement.
 * @ghidraAddress NTSC-U/C: 0x002e6d9c
 * @ghidraAddress PAL: 0x002f0d0c
 */
int rtDmeDeviceDeselectConnection(rtDmeDevice *device, int connectionId);

/**
 * Set the identifier of this console.
 *
 * @param device The device.
 * @param id The identifier.
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002e6e84
 * @ghidraAddress PAL: 0x002f0df4
 */
int rtDmeDeviceSetLocalId(rtDmeDevice *device, int id);

/**
 * Copy the addresses and identifiers of this console.
 *
 * @param device The device.
 * @param info Receives the addresses and identifiers.
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002e6e98
 * @ghidraAddress PAL: 0x002f0e08
 */
int rtDmeDeviceGetLocalInfo(rtDmeDevice *device, rtDmePeerInfo *info);

/**
 * Add a peer or update its connection, and add it to the connections it belongs to.
 *
 * A connection the device does not know yet is created, and the peer receives a query for its
 * details.
 *
 * @param device The device.
 * @param info The addresses and identifiers of the peer.
 * @return The peer.
 * @ghidraAddress NTSC-U/C: 0x002e6ed4
 * @ghidraAddress PAL: 0x002f0e44
 */
rtDmePeer *rtDmeDeviceAddPeer(rtDmeDevice *device, const rtDmePeerInfo *info);

/**
 * Remove a peer from its connections and from the device.
 *
 * @param device The device.
 * @param peerId The peer.
 * @ghidraAddress NTSC-U/C: 0x002e6fa0
 * @ghidraAddress PAL: 0x002f0f10
 */
void rtDmeDeviceRemovePeer(rtDmeDevice *device, int peerId);

/**
 * Report the peer that talks on the active connection and refresh the talk flag of the device.
 *
 * @param device The device.
 * @param talker Receives the addresses and identifiers of the peer that talks, cleared when no
 * peer talks or no connection is active. The routine clears it even when it is null.
 * @return The flags of the device.
 * @ghidraAddress NTSC-U/C: 0x002e7094
 * @ghidraAddress PAL: 0x002f1004
 */
unsigned int rtDmeDeviceGetStatus(rtDmeDevice *device, rtDmePeerInfo *talker);

/**
 * Create an entry of the list of the peers of a connection.
 *
 * @param peer The peer.
 * @return The entry, or null.
 * @ghidraAddress NTSC-U/C: 0x002f036c
 * @ghidraAddress PAL: 0x002fa2dc
 */
rtDmeEndpointNode *rtDmeEndpointNodeCreate(rtDmePeer *peer);

/**
 * Release an entry of the list of the peers of a connection.
 *
 * @param node The entry, or null.
 * @ghidraAddress NTSC-U/C: 0x002f03c0
 * @ghidraAddress PAL: 0x002fa330
 */
void rtDmeEndpointNodeDestroy(rtDmeEndpointNode *node);

// Routines of the DME core that the DME core headers do not declare yet.

/** A port mapping request to a router. */
typedef struct rtUpnpContext rtUpnpContext;

/**
 * Receiver of the result of a port mapping request.
 *
 * @param context The request.
 * @param status 0 for success.
 * @param result The result the router reported.
 * @param address The dotted public address.
 * @param port The public port.
 * @param owner The device that started the request.
 */
typedef void (*rtUpnpPortMappingCallback)(rtUpnpContext *context,
                                          int status,
                                          int result,
                                          const char *address,
                                          unsigned short port,
                                          rtDmeDevice *owner);

/**
 * Allocate a cleared block from the allocator of the library.
 *
 * @param block Receives the block, or null.
 * @param size The size of the block in bytes.
 * @return 0, or 1 when the allocation fails.
 * @ghidraAddress NTSC-U/C: 0x002aa464
 * @ghidraAddress PAL: 0x002b43d4
 */
int rtDmeAllocZeroed(void **block, int size);

/**
 * Release a block of the allocator of the library and clear the pointer.
 *
 * @param block The pointer to the block.
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002aa4c8
 * @ghidraAddress PAL: 0x002b4438
 */
int rtDmeFree(void **block);

/**
 * Parse a dotted address.
 *
 * @param text The address.
 * @return The address, the first part in the high byte.
 * @ghidraAddress NTSC-U/C: 0x002af95c
 * @ghidraAddress PAL: 0x002b98cc
 */
unsigned int rtNetParseAddress(const char *text);

/**
 * Format an address as dotted text.
 *
 * @param address The address, the first part in the high byte.
 * @param text Receives the text.
 * @ghidraAddress NTSC-U/C: 0x002af9cc
 * @ghidraAddress PAL: 0x002b993c
 */
void rtNetFormatAddress(unsigned int address, char *text);

/**
 * Create a port mapping request.
 *
 * @param context Receives the request.
 * @param socket The socket of the request.
 * @param port The local port to map.
 * @param owner The device that receives the result.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002ccb08
 * @ghidraAddress PAL: 0x002d6a78
 */
int rtUpnpCreate(rtUpnpContext **context,
                 rtNetSocket *socket,
                 unsigned short port,
                 rtDmeDevice *owner);

/**
 * Release a port mapping request.
 *
 * @param context The request.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002ccbf8
 * @ghidraAddress PAL: 0x002d6b68
 */
int rtUpnpDestroy(rtUpnpContext *context);

/**
 * Send a port mapping request to a router.
 *
 * @param context The request.
 * @param host The dotted address of the router.
 * @param port The port of the router.
 * @param callback Receives the result.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002cd6a0
 * @ghidraAddress PAL: 0x002d7610
 */
int rtUpnpBeginPortMapping(rtUpnpContext *context,
                           const char *host,
                           unsigned short port,
                           rtUpnpPortMappingCallback callback);

/**
 * Advance a port mapping request.
 *
 * @param context The request.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002cd7c8
 * @ghidraAddress PAL: 0x002d7738
 */
int rtUpnpUpdate(rtUpnpContext *context);

#ifdef __cplusplus
}
#endif

#endif
