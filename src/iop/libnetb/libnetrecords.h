#pragma once

#include <inet.h>
#include <netcnf.h>

/**
 * Argument and reply records of the libnetb RPC functions. A reply overwrites the arguments in the
 * same buffer, so each function reads its arguments before it writes its reply.
 */

/** Result codes of the module. */
enum LibnetError {
    kLibnetErrorSemaphore = -540, /*!< A semaphore operation failed. */
    kLibnetErrorUnhandled = -541, /*!< The function number is not handled. */
    kLibnetErrorNoEvent = -544,   /*!< No interface event arrived, or the interface went away. */
};

/** Reply that is only a result. */
struct LibnetResult {
    int mResult; /*!< Result of the function. */
};

/** Arguments that are one value. */
struct LibnetValueArgs {
    int mValue; /*!< The value. */
};

/** Arguments that pass an environment to inetctl_4(). */
struct LibnetEnvArgs {
    sceNetCnfEnv *mEnv; /*!< Environment in IOP memory. */
};

/** Arguments of a connection and one value. */
struct LibnetConnectionArgs {
    int mCid;   /*!< Connection identifier. */
    int mValue; /*!< The value. */
};

/** Arguments that pass a parameter block. */
struct LibnetParamArgs {
    int mReserved;          /*!< Receives the result. */
    unsigned char mParam[]; /*!< Parameter block of inet_6(). */
};

/** Arguments of a receive. */
struct LibnetTransferArgs {
    int mCid;     /*!< Connection identifier. */
    int mFlags;   /*!< Flags. */
    int mCount;   /*!< Byte count. */
    int mTimeout; /*!< Timeout. */
};

/** Arguments of a send, followed by the data. */
struct LibnetSendArgs {
    int mCid;              /*!< Connection identifier. */
    int mFlags;            /*!< Flags. */
    int mCount;            /*!< Byte count. */
    int mTimeout;          /*!< Timeout. */
    unsigned char mData[]; /*!< The data. */
};

/** Reply of a receive, followed by the data. */
struct LibnetRecvReply {
    int mResult;           /*!< Byte count, or a negative #sceInetError. */
    int mFlags;            /*!< Flags of the receive. */
    unsigned char mData[]; /*!< The data. */
};

/** Reply with flags. */
struct LibnetFlagsReply {
    int mResult; /*!< Byte count, or a negative #sceInetError. */
    int mFlags;  /*!< Flags of the transfer. */
};

/** Arguments of a name resolution, followed by the name. */
struct LibnetName2AddressArgs {
    int mFlags;      /*!< Flags. */
    int mTimeout;    /*!< Timeout. */
    int mRetries;    /*!< Retry count. */
    int mOption;     /*!< Sixth argument of sceInetName2Address(). */
    int mNameLength; /*!< Length of mName, zero for no name. */
    char mName[];    /*!< The name. */
};

/** Reply with an address. */
struct LibnetAddressReply {
    int mResult;             /*!< Result. */
    sceInetAddress mAddress; /*!< The address. */
};

/** Arguments of an address conversion. */
struct LibnetAddress2StringArgs {
    int mSize;               /*!< Size of the text. */
    sceInetAddress mAddress; /*!< The address. */
};

/** Reply of an address conversion, followed by the text. */
struct LibnetAddress2StringReply {
    int mResult;             /*!< Result. */
    sceInetAddress mAddress; /*!< The address, unchanged. */
    char mText[];            /*!< The text. */
};

/** Arguments of a value and a buffer the function reads and writes. */
struct LibnetBufferArgs {
    int mValue;            /*!< The value. */
    unsigned char mData[]; /*!< The buffer. */
};

/** Reply that is a result and a buffer. */
struct LibnetBufferReply {
    int mResult;           /*!< Result. */
    unsigned char mData[]; /*!< The buffer. */
};

/** Arguments of a control operation, followed by its buffer. */
struct LibnetControlArgs {
    int mId;               /*!< Connection or interface identifier. */
    unsigned int mCode;    /*!< Control code. */
    int mSize;             /*!< Size of mData, zero for no buffer. */
    unsigned char mData[]; /*!< The buffer. */
};

/** Reply of a control operation, followed by its buffer. */
struct LibnetControlReply {
    int mResult;           /*!< Result. */
    unsigned int mCode;    /*!< The control code, unchanged. */
    int mSize;             /*!< The size, unchanged. */
    unsigned char mData[]; /*!< The buffer. */
};

/** Reply of a datagram receive, followed by the data. */
struct LibnetRecvFromReply {
    int mResult;             /*!< Byte count, or a negative #sceInetError. */
    int mFlags;              /*!< Flags of the receive. */
    sceInetAddress mAddress; /*!< Sender address. */
    int mPort;               /*!< Sender port. */
    unsigned char mData[];   /*!< The data. */
};

/** Arguments of a datagram send, followed by the data. */
struct LibnetSendToArgs {
    int mCid;                /*!< Connection identifier. */
    int mFlags;              /*!< Flags. */
    int mCount;              /*!< Byte count. */
    int mTimeout;            /*!< Timeout. */
    int mPort;               /*!< Destination port. */
    sceInetAddress mAddress; /*!< Destination address. */
    unsigned char mData[];   /*!< The data. */
};

/** Arguments of two values. */
struct LibnetPairArgs {
    int mFirst;  /*!< First value. */
    int mSecond; /*!< Second value. */
};

/** Reply of a result and a buffer after one reserved word. */
struct LibnetOffsetBufferReply {
    int mResult;           /*!< Result. */
    int mReserved;         /*!< Never written by the module. */
    unsigned char mData[]; /*!< The buffer. */
};

/** Arguments of inet_14(). */
struct LibnetInet14Args {
    int mArgument0;             /*!< First argument. */
    int mArgument2;             /*!< Third argument. */
    int mArgument4;             /*!< Fifth argument. */
    int mArgument5;             /*!< Sixth argument. */
    unsigned char mBuffer3[16]; /*!< Fourth argument, a buffer. */
    int mArgument6;             /*!< Seventh argument. The second argument points here. */
};

/** Reply with an interface state. */
struct LibnetStateReply {
    int mResult; /*!< Result. */
    int mState;  /*!< An #InetCtlState. */
};

/** Reply with an interface event. */
struct LibnetEventReply {
    int mResult;      /*!< Zero, or a negative #LibnetError. */
    int mInterfaceId; /*!< Interface identifier. */
    int mEvent;       /*!< Event. */
};

/** Reply of an interface wait that ended with the awaited state. */
struct LibnetInterfacesReply {
    int mResult;          /*!< The awaited state. */
    int mInterfaceIds[2]; /*!< The recorded interfaces. */
};

/** Reply of an interface wait that ended because the interface went away. */
struct LibnetLostReply {
    int mResult;      /*!< #kLibnetErrorNoEvent. */
    int mEvent;       /*!< Event. */
    int mInterfaceId; /*!< Interface identifier. */
};

/** Reply of an interface wait that ended with an error. */
struct LibnetWaitErrorReply {
    int mResult;      /*!< Negative #LibnetError. */
    int mInterfaceId; /*!< Interface identifier. */
};

/** Arguments of closing a connection with asynchronous transfers. */
struct LibnetCloseAsyncArgs {
    int mCid;     /*!< Connection identifier. */
    int mTimeout; /*!< Milliseconds to let pending sends finish, or negative to wait forever. */
};
