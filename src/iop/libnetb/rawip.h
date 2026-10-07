#pragma once

#include <inet.h>

#include "libnetb/icmppacket.h"

/**
 * Datagrams over a raw IP connection. Each one is an ICMP echo whose identifier marks it as the
 * module's and whose sequence number is the port of the datagram. The header is all a datagram
 * transfers.
 *
 * The module was built without RTTI. The name is inferred from the module's routines.
 */
class RawIp {
public:
    /**
     * Receive datagrams until one of the module's echo replies arrives, an error occurs, or
     * nothing is waiting.
     *
     * @param cid Connection identifier.
     * @param port Receives the port of the datagram.
     * @param address Receives the sender address.
     * @param received Set when a datagram arrived.
     * @param name Protocol name for messages.
     * @return Zero, 1 when the receive failed, or 2 for a null argument.
     * @ghidraAddress NTSC-U/C: 0x00002dd8
     * @ghidraAddress PAL: 0x00002c90
     */
    static int Receive(
        int cid, unsigned short *port, sceInetAddress *address, bool *received, const char *name);

    /**
     * Send one datagram as an echo request.
     *
     * @param cid Connection identifier.
     * @param port Port of the datagram.
     * @param address Destination address.
     * @param sent Set when the datagram was sent.
     * @param name Protocol name for messages.
     * @return Zero, 1 when the send failed, or 2 for a null argument.
     * @ghidraAddress NTSC-U/C: 0x00003100
     * @ghidraAddress PAL: 0x00002fb4
     */
    static int
    Send(int cid, unsigned short port, const sceInetAddress *address, bool *sent, const char *name);

private:
    /**
     * Compute the Internet checksum of a buffer, adding halfwords in host byte order.
     *
     * @param data Buffer, halfword aligned.
     * @param size Byte count.
     * @return The checksum.
     * @ghidraAddress NTSC-U/C: 0x000033e0
     * @ghidraAddress PAL: 0x00003290
     */
    static unsigned short Checksum(const void *data, int size);

    /** ICMP header of the last packet received. */
    static IcmpHeader sReceivedIcmp;

    /** ICMP header of the last packet sent. */
    static IcmpHeader sSentIcmp;

    /** IP header of the last packet received. */
    static IpHeader sReceivedIp;

    /** The last packet received. */
    static IcmpPacket sReceivedPacket;

    /** IP header of the last packet sent. */
    static IpHeader sSentIp;

    /** The last packet sent. */
    static IcmpPacket sSentPacket;
};
