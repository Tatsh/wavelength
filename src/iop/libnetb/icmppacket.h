#pragma once

/** An IPv4 header. Multibyte fields are in network byte order on the wire. */
struct IpHeader {
    unsigned char mVersionAndLength; /*!< Version and header length. */
    unsigned char mTypeOfService;    /*!< Type of service. */
    unsigned short mTotalLength;     /*!< Total length. */
    unsigned short mIdentification;  /*!< Identification. */
    unsigned short mFragment;        /*!< Flags and fragment offset. */
    unsigned char mTimeToLive;       /*!< Time to live. */
    unsigned char mProtocol;         /*!< Protocol number. */
    unsigned short mChecksum;        /*!< Header checksum. */
    unsigned int mSource;            /*!< Source address. */
    unsigned int mDestination;       /*!< Destination address. */
};

/** An ICMP echo header. */
struct IcmpHeader {
    unsigned char mType;        /*!< Message type. */
    unsigned char mCode;        /*!< Message code. */
    unsigned short mChecksum;   /*!< Checksum of the header. */
    unsigned short mIdentifier; /*!< Echo identifier. */
    unsigned short mSequence;   /*!< Echo sequence number. */
};

/** An IPv4 packet with only an ICMP echo header. */
struct IcmpPacket {
    IpHeader mIp;     /*!< IP header. */
    IcmpHeader mIcmp; /*!< ICMP header. */
};
