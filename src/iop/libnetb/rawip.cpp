#include "libnetb/rawip.h"

#include <stdio.h>

#include <kernel.h>
#include <sysclib.h>

#include "libnetb/libnetb.h"

namespace {

constexpr unsigned char kProtocolIcmp = 1;
constexpr unsigned char kIcmpEchoReply = 0;
constexpr unsigned char kIcmpEchoRequest = 8;
// Echo identifier that marks the module's datagrams.
constexpr unsigned short kEchoIdentifier = 0xa5a5;
constexpr unsigned char kTimeToLive = 64;
constexpr int kWaitForever = -1;
constexpr int kSendTimeout = 500;
constexpr int kIdleDelay = 2000;
constexpr int kResultFailed = 1;
constexpr int kResultNullArgument = 2;

constexpr char kRawReceiveFailedMessage[] = "RECV/%s: sceInetRecv() failed, ";
constexpr char kRawSendFailedMessage[] = "SEND/%s: sceInetSend() failed, ";

inline unsigned short Swap16(unsigned short value) {
    return static_cast<unsigned short>((value << 8) | (value >> 8));
}

inline unsigned int Swap32(unsigned int value) {
    return (value << 24) | ((value & 0xff00) << 8) | ((value >> 8) & 0xff00) | (value >> 24);
}

// The first four bytes of an address, read as a little-endian word.
inline unsigned int LoadAddressWord(const sceInetAddress *address) {
    return address->data[0] | (address->data[1] << 8) | (address->data[2] << 16) |
           (static_cast<unsigned int>(address->data[3]) << 24);
}

inline void StoreAddressWord(sceInetAddress *address, unsigned int value) {
    address->data[0] = static_cast<unsigned char>(value);
    address->data[1] = static_cast<unsigned char>(value >> 8);
    address->data[2] = static_cast<unsigned char>(value >> 16);
    address->data[3] = static_cast<unsigned char>(value >> 24);
}

// Copy a header field by field, where an assignment could become a call the module does not import.
inline void CopyIpHeader(IpHeader *destination, const IpHeader *source) {
    destination->mVersionAndLength = source->mVersionAndLength;
    destination->mTypeOfService = source->mTypeOfService;
    destination->mTotalLength = source->mTotalLength;
    destination->mIdentification = source->mIdentification;
    destination->mFragment = source->mFragment;
    destination->mTimeToLive = source->mTimeToLive;
    destination->mProtocol = source->mProtocol;
    destination->mChecksum = source->mChecksum;
    destination->mSource = source->mSource;
    destination->mDestination = source->mDestination;
}

inline void CopyIcmpHeader(IcmpHeader *destination, const IcmpHeader *source) {
    destination->mType = source->mType;
    destination->mCode = source->mCode;
    destination->mChecksum = source->mChecksum;
    destination->mIdentifier = source->mIdentifier;
    destination->mSequence = source->mSequence;
}

} // namespace

// NTSC-U/C: 0x00005ce8
IcmpHeader RawIp::sReceivedIcmp;

// NTSC-U/C: 0x00005cf0
IcmpHeader RawIp::sSentIcmp;

// NTSC-U/C: 0x00005d90
IpHeader RawIp::sReceivedIp;

// NTSC-U/C: 0x00005da8
IcmpPacket RawIp::sReceivedPacket;

// NTSC-U/C: 0x00005dc8
IpHeader RawIp::sSentIp;

// NTSC-U/C: 0x00005de0
IcmpPacket RawIp::sSentPacket;

int RawIp::Receive(
    int cid, unsigned short *port, sceInetAddress *address, bool *received, const char *name) {
    if (received == nullptr) {
        return kResultNullArgument;
    }
    *received = false;
    if (port == nullptr || address == nullptr) {
        return kResultNullArgument;
    }
    int count;
    do {
        int flags;
        count = sceInetRecv(cid, &sReceivedPacket, sizeof(IcmpPacket), &flags, kWaitForever);
        if (count == static_cast<int>(sizeof(IcmpPacket))) {
            CopyIpHeader(&sReceivedIp, &sReceivedPacket.mIp);
            CopyIcmpHeader(&sReceivedIcmp, &sReceivedPacket.mIcmp);
            sReceivedIp.mTotalLength = Swap16(sReceivedIp.mTotalLength);
            sReceivedIp.mIdentification = Swap16(sReceivedIp.mIdentification);
            sReceivedIp.mFragment = Swap16(sReceivedIp.mFragment);
            sReceivedIp.mSource = Swap32(sReceivedIp.mSource);
            sReceivedIp.mDestination = Swap32(sReceivedIp.mDestination);
            sReceivedIcmp.mIdentifier = Swap16(sReceivedIcmp.mIdentifier);
            sReceivedIcmp.mSequence = Swap16(sReceivedIcmp.mSequence);
            if (sReceivedIcmp.mIdentifier == kEchoIdentifier &&
                sReceivedIp.mProtocol == kProtocolIcmp && sReceivedIcmp.mType == kIcmpEchoReply) {
                memset(address, 0, sizeof(sceInetAddress));
                StoreAddressWord(address, sReceivedIp.mSource);
                *port = sReceivedIcmp.mSequence;
                *received = true;
                return 0;
            }
        } else if (count < 0) {
            printf(kRawReceiveFailedMessage, name);
            Libnet::PrintInetError(count);
            return kResultFailed;
        } else if (count == 0) {
            DelayThread(kIdleDelay);
        }
    } while (count > 0);
    return 0;
}

int RawIp::Send(
    int cid, unsigned short port, const sceInetAddress *address, bool *sent, const char *name) {
    if (sent == nullptr) {
        return kResultNullArgument;
    }
    *sent = false;
    if (address == nullptr) {
        return kResultNullArgument;
    }
    memset(&sSentIp, 0, sizeof(IpHeader));
    sSentIp.mTimeToLive = kTimeToLive;
    sSentIp.mProtocol = kProtocolIcmp;
    sSentIp.mTotalLength = Swap16(sSentIp.mTotalLength);
    sSentIp.mIdentification = Swap16(sSentIp.mIdentification);
    sSentIp.mFragment = Swap16(sSentIp.mFragment);
    sSentIp.mSource = Swap32(sSentIp.mSource);
    sSentIp.mDestination = Swap32(LoadAddressWord(address));
    memset(&sSentIcmp, 0, sizeof(IcmpHeader));
    sSentIcmp.mType = kIcmpEchoRequest;
    sSentIcmp.mIdentifier = kEchoIdentifier;
    sSentIcmp.mCode = 0;
    sSentIcmp.mSequence = Swap16(port);
    sSentIcmp.mChecksum = Checksum(&sSentIcmp, sizeof(IcmpHeader));
    CopyIpHeader(&sSentPacket.mIp, &sSentIp);
    CopyIcmpHeader(&sSentPacket.mIcmp, &sSentIcmp);
    int flags = 0; // The binary leaves the flags uninitialised.
    const int count = sceInetSend(cid, &sSentPacket, sizeof(IcmpPacket), &flags, kSendTimeout);
    if (count < 0) {
        printf(kRawSendFailedMessage, name);
        Libnet::PrintInetError(count);
        return kResultFailed;
    }
    if (count > 0) {
        *sent = true;
    }
    return 0;
}

unsigned short RawIp::Checksum(const void *data, int size) {
    const auto *bytes = static_cast<const unsigned char *>(data);
    int sum = 0;
    for (; size >= 2; size -= 2, bytes += 2) {
        sum += bytes[0] | (bytes[1] << 8);
    }
    if (size == 1) {
        sum += bytes[0];
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += sum >> 16;
    return static_cast<unsigned short>(~sum);
}
