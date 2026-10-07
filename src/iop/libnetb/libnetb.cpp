#include "libnetb/libnetb.h"

#include <stdio.h>

#include <inet.h>
#include <inetctl.h>
#include <msifrpc.h>
#include <sif.h>
#include <sifrpc.h>
#include <sysclib.h>

#include "libnetb/asyncinfo.h"
#include "libnetb/ineteventqueue.h"
#include "libnetb/libnetrecords.h"

namespace {

constexpr unsigned short kModuleVersion = 0x0114;
constexpr int kRpcThreadPriority = 0x40;
constexpr int kRpcThreadStackSize = 0x2000;
constexpr int kDefaultSendDelay = 10000;
constexpr int kDecimal = 10;
constexpr int kBuildDateSize = 64;
constexpr int kAddressTextSize = 16;
constexpr int kDmaPollDelay = 200;
constexpr int kClosePollDelay = 100;
constexpr int kMicrosecondsPerMillisecond = 1000;
// AllocSysMemory() placement of the lowest free area that fits.
constexpr int kAllocLowest = 0;
constexpr int kSemaphoreAttribute = 1;
constexpr int kResultBadArgument = 2;
constexpr int kResultDmaFull = 4;

#ifdef VIDEO_STANDARD_PAL
constexpr char kVersionText[] = "libnetb version: 1.09.0003";
#else
constexpr char kVersionText[] = "libnetb version: 1.09.0002";
#endif
constexpr char kVerboseOption[] = "-verbose";
constexpr char kSendDelayOption[] = "-send_delay=";
constexpr int kSendDelayOptionLength = sizeof(kSendDelayOption) - 1;
constexpr char kMonthNames[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
constexpr int kMonthNameLength = 3;
constexpr char kMonthFormat[] = "%.3s";
// Offsets into the __DATE__ text, which reads "Mmm dd yyyy", and the __TIME__ text, which reads
// "hh:mm:ss".
constexpr int kDateDayOffset = 4;
constexpr int kDateYearOffset = 7;
constexpr int kTimeMinuteOffset = 3;
constexpr int kTimeSecondOffset = 6;
constexpr char kBuildDateFormat[] = "%.2d.%.2d.%.4s.%.2s.%.2s.%.2s";

constexpr char kCreateThreadErrorMessage[] = "CreateThread() in Libnet failed.(%d)\n";
constexpr char kStartThreadErrorMessage[] = "StartThread() in Libnet failed.(%d)\n";
constexpr char kSendDelayMessage[] = "libnetb: delay between sends = %d msecs\n";
constexpr char kCreateNamedThreadError[] = "Create %s thread failed.\n";
constexpr char kDestroyThreadErrorMessage[] = "Couldn't not destroy recv thread\n";
constexpr char kAllocateHandlerErrorMessage[] =
    "ERROR: libnetb - Couldn't allocate Async Inet handler!\n";
constexpr char kCreateListenerErrorMessage[] =
    "ERROR: libnetb - Couldn't create Async Inet listener thread!\n";
constexpr char kNewline[] = "\n";
constexpr char kLineFormat[] = "%s\n";
constexpr char kStateRule[] = "--------------------------\n";
constexpr char kStateTitle[] = "Displaying LIBNETB's State\n";
constexpr char kBuiltMessage[] = "built: %s\n";
constexpr char kUnhandledCaseMessage[] = "ERROR: libnetb - UNHANDLED CASE!!!!!!\n";

constexpr char kInetErrorPrefix[] = "error %ld, ";
constexpr char kUnknownErrorMessage[] = "unknown error\n";
constexpr char kNoErrorMessage[] = "no error\n";
// Descriptions of the inet error codes, from sceINETE_NO_ROUTE up to sceINETE_TIMEOUT.
constexpr const char *kInetErrorMessages[] = {
    "sceINETE_NO_ROUTE, no routing to destination exists\n",
    "sceINETE_INVALID_CALL, invalid function call\n",
    "sceINETE_INVALID_ARGUMENT, an argument is invalid\n",
    "sceINETE_CONNECTION_REFUSED, connection was reset when status is syn-received\n",
    "sceINETE_CONNECTION_RESET, connection was reset\n",
    "sceINETE_CONNECTION_CLOSING, connection status is closing\n",
    "sceINETE_CONNECTION_DOES_NOT_EXIST, no connection was established or connection has been "
    "closed\n",
    "sceINETE_CONNECTION_ALREADY_EXISTS, an attempt was made to open a connection that was "
    "already established\n",
    "sceINETE_FOREIGN_SOCKET_UNSPECIFIED, invalid value as specified as remote_addr or "
    "remote_port\n",
    "sceINETE_LOCAL_SOCKET_UNSPECIFIED, invalid value was specified as local_port\n",
    "sceINETE_INSUFFICIENT_RESOURCES, insufficient memory area\n",
    "sceINETE_LINK_DOWN, device initialization or connection processing is not completed\n",
    "sceINETE_BUSY, INET module initialization is not completed\n",
    "sceINETE_ABORT, interruption due to sceInetAbort() call\n",
    "sceINETE_TIMEOUT, timeout specified by argument in function occurred or TCP resend "
    "timeout occurred\n",
};

constexpr char kConnectionRule[] = "=====================================\n";
constexpr char kConnectionTitle[] = "(SEND & RECV) CONNECTION INFORMATION:\n";
constexpr char kConnectionWarning[] =
    " Warning!! Could not get sceInetControl() information for (CID):'%d' with ERROR:'%d'\n";
constexpr char kConnectionIdLine[] = " (cid)ConnectionID:\t\t\t\t\t\t'%d'\n";
constexpr char kProtocolTcpLine[] = " (proto)Protocol:\t\t\t\t\t\t'sceINETI_PROTO_TCP -- TCP'\n";
constexpr char kProtocolUdpLine[] = " (proto)Protocol:\t\t\t\t\t\t'sceINETI_PROTO_UDP -- UDP'\n";
constexpr char kProtocolIpLine[] = " (proto)Protocol:\t\t\t\t\t\t'sceINETI_PROTO_IP -- RAW IP'\n";
constexpr char kReceiveQueueLine[] =
    " (recv_queue_length)Number of data bytes in receive buffer:\t'%d'\n";
constexpr char kSendQueueLine[] =
    " (send_queue_length)Number of data bytes in send buffer:\t'%d'\n";
constexpr char kLocalReservedLine[] =
    " (sceInetAddress)(local_adr.reserved)Reserved area (always 0):\t'%d'\n";
constexpr char kLocalAddressLine[] = " (sceInetAddress)(local_adr.data)IP Address:\t\t\t'%s'\n";
constexpr char kLocalPortLine[] = " (local_port)Local port number:\t\t\t\t\t'%d'\n";
constexpr char kRemoteReservedLine[] =
    " (sceInetAddress)(remote_adr.reserved)Reserved area (always 0):\t'%d'\n";
constexpr char kRemoteAddressLine[] = " (sceInetAddress)(remote_adr.data)IP Address:\t\t\t'%s'\n";
constexpr char kRemotePortLine[] = " (remote_port)Remote port number:\t\t\t\t'%d'\n";
constexpr const char *kConnectionStateLines[] = {
    " (state)Connection State:\t\t\t\t\t"
    "'sceINETI_STATE_UNKNOWN -- State unknown (TCP, UDP, Raw IP)'\n",
    " (state)Connection State:\t\t\t\t\t'sceINETI_STATE_CLOSED -- Closed (TCP, UDP, Raw IP)'\n",
    " (state)Connection State:\t\t\t\t\t'sceINETI_STATE_CREATED -- Created (UDP)'\n",
    " (state)Connection State:\t\t\t\t\t'sceINETI_STATE_OPENED -- Opened (UDP, Raw IP)'\n",
    " (state)Connection State:\t\t\t\t\t'sceINETI_STATE_LISTEN -- TCP internal state'\n",
    " (state)Connection State:\t\t\t\t\t'sceINETI_STATE_SYN_SENT -- TCP internal state'\n",
    " (state)Connection State:\t\t\t\t\t'sceINETI_STATE_SYN_RECEIVED -- TCP internal state'\n",
    " (state)Connection State:\t\t\t\t\t'sceINETI_STATE_ESTABLISHED -- TCP internal state'\n",
    " (state)Connection State:\t\t\t\t\t'sceINETI_STATE_FIN_WAIT_1 -- TCP internal state'\n",
    " (state)Connection State:\t\t\t\t\t'sceINETI_STATE_FIN_WAIT_2 -- TCP internal state'\n",
    " (state)Connection State:\t\t\t\t\t'sceINETI_STATE_CLOSE_WAIT -- TCP internal state'\n",
    " (state)Connection State:\t\t\t\t\t'sceINETI_STATE_CLOSING -- TCP internal state'\n",
    " (state)Connection State:\t\t\t\t\t'sceINETI_STATE_LAST_ACK -- TCP internal state'\n",
    " (state)Connection State:\t\t\t\t\t'sceINETI_STATE_TIME_WAIT -- TCP internal state'\n",
};
constexpr unsigned int kConnectionStateCount =
    sizeof(kConnectionStateLines) / sizeof(kConnectionStateLines[0]);

// An AsyncInfo allocated together with the IOP copy of its buffer.
struct AsyncAllocation {
    AsyncInfo mInfo;
    unsigned char mBuffer[];
};

inline void SetResult(void *buffer, int result) {
    static_cast<LibnetResult *>(buffer)->mResult = result;
}

// Copy the record the EE sent into the record of a new transfer.
inline void CopyAsyncInfo(AsyncInfo *destination, const AsyncInfo *source) {
    destination->mDataWaitingSize = source->mDataWaitingSize;
    destination->mLastReturnedFlag = source->mLastReturnedFlag;
    destination->mMaxPacketSize = source->mMaxPacketSize;
    destination->mReserved = source->mReserved;
    destination->mCid = source->mCid;
    destination->mInetFlag = source->mInetFlag;
    destination->mBufferSize = source->mBufferSize;
    destination->mBufferStart = source->mBufferStart;
    destination->mOtherProcessorsInfo = source->mOtherProcessorsInfo;
    destination->mNext = source->mNext;
    destination->mThid = source->mThid;
    destination->mOtherWriteLoc = source->mOtherWriteLoc;
    destination->mClose = source->mClose;
    destination->mType = source->mType;
    destination->mRemotePort = source->mRemotePort;
    destination->mWriteOffset = source->mWriteOffset;
}

// Allocate a transfer and its buffer from the EE's record.
inline AsyncAllocation *AllocateTransfer(const AsyncInfo *request) {
    int state;
    const int suspended = CpuSuspendIntr(&state);
    auto *allocation = static_cast<AsyncAllocation *>(
        AllocSysMemory(kAllocLowest, request->mBufferSize + sizeof(AsyncInfo), nullptr));
    if (suspended == KE_OK) {
        CpuResumeIntr(state);
    }
    return allocation;
}

// Free a transfer whose thread did not start.
inline void FreeTransfer(AsyncAllocation *allocation) {
    int state;
    const int suspended = CpuSuspendIntr(&state);
    FreeSysMemory(allocation);
    if (suspended == KE_OK) {
        CpuResumeIntr(state);
    }
}

} // namespace

// NTSC-U/C: 0x00005c70
ModuleInfo Module = {"Libnet", kModuleVersion};

// NTSC-U/C: 0x00005c80
int Libnet::sVerbose = 0;

// NTSC-U/C: 0x00005c90
const char *Libnet::sVersion = kVersionText;

// NTSC-U/C: 0x00005c9c
int Libnet::sSendDelay = kDefaultSendDelay;

// NTSC-U/C: 0x00005cc0
LibnetConfig Libnet::sConfig;

inline void Libnet::WaitInterfaceState(int state, void *buffer) {
    for (;;) {
        int interfaceId;
        int event;
        const int result = InetEventQueue::GetEvent(&interfaceId, &event);
        if (result < 0) {
            auto *reply = static_cast<LibnetWaitErrorReply *>(buffer);
            reply->mResult = result;
            reply->mInterfaceId = interfaceId;
            return;
        }
        const int mapped = InetEventQueue::MapEvent(interfaceId, event);
        if (mapped == InetEventQueue::kInterfaceLost) {
            auto *reply = static_cast<LibnetLostReply *>(buffer);
            reply->mResult = kLibnetErrorNoEvent;
            reply->mEvent = event;
            reply->mInterfaceId = interfaceId;
            return;
        }
        if (mapped == state) {
            auto *reply = static_cast<LibnetInterfacesReply *>(buffer);
            reply->mResult = mapped;
            for (int i = 0; i < InetEventQueue::kInterfaceCount; ++i) {
                reply->mInterfaceIds[i] = InetEventQueue::sInterfaceIds[i];
            }
            return;
        }
    }
}

void *Libnet::RpcHandler(unsigned int function, void *buffer, [[maybe_unused]] int size) {
    switch (function) {
    case kFunctionInet6:
        SetResult(buffer, inet_6(static_cast<LibnetParamArgs *>(buffer)->mParam));
        break;
    case kFunctionInet7: {
        const auto *args = static_cast<const LibnetConnectionArgs *>(buffer);
        SetResult(buffer, inet_7(args->mCid, args->mValue));
        break;
    }
    case kFunctionInet8: {
        const auto *args = static_cast<const LibnetConnectionArgs *>(buffer);
        SetResult(buffer, inet_8(args->mCid, args->mValue));
        break;
    }
    case kFunctionRecv: {
        const auto *args = static_cast<const LibnetTransferArgs *>(buffer);
        const int cid = args->mCid;
        const int count = args->mCount;
        const int timeout = args->mTimeout;
        auto *reply = static_cast<LibnetRecvReply *>(buffer);
        reply->mResult = sceInetRecv(cid, reply->mData, count, &reply->mFlags, timeout);
        break;
    }
    case kFunctionSend: {
        auto *args = static_cast<LibnetSendArgs *>(buffer);
        auto *reply = static_cast<LibnetFlagsReply *>(buffer);
        reply->mResult =
            sceInetSend(args->mCid, args->mData, args->mCount, &reply->mFlags, args->mTimeout);
        break;
    }
    case kFunctionName2Address: {
        const auto *args = static_cast<const LibnetName2AddressArgs *>(buffer);
        const int flags = args->mFlags;
        const int timeout = args->mTimeout;
        const int retries = args->mRetries;
        const int option = args->mOption;
        const char *name = args->mNameLength > 0 ? args->mName : nullptr;
        auto *reply = static_cast<LibnetAddressReply *>(buffer);
        reply->mResult =
            sceInetName2Address(flags, &reply->mAddress, name, timeout, retries, option);
        break;
    }
    case kFunctionAddress2String: {
        const auto *args = static_cast<const LibnetAddress2StringArgs *>(buffer);
        auto *reply = static_cast<LibnetAddress2StringReply *>(buffer);
        reply->mResult = sceInetAddress2String(reply->mText, args->mSize, &args->mAddress);
        break;
    }
    case kFunctionInet24: {
        auto *args = static_cast<LibnetBufferArgs *>(buffer);
        SetResult(buffer, inet_24(args->mData, args->mValue));
        break;
    }
    case kFunctionInterfaceControl: {
        auto *args = static_cast<LibnetControlArgs *>(buffer);
        void *data = args->mSize != 0 ? args->mData : nullptr;
        SetResult(buffer, sceInetInterfaceControl(args->mId, args->mCode, data, args->mSize));
        break;
    }
    case kFunctionInet27: {
        auto *args = static_cast<LibnetBufferArgs *>(buffer);
        SetResult(buffer, inet_27(args->mData, args->mValue));
        break;
    }
    case kFunctionInet30: {
        auto *args = static_cast<LibnetBufferArgs *>(buffer);
        SetResult(buffer, inet_30(args->mData, args->mValue));
        break;
    }
    case kFunctionInet36:
        SetResult(buffer, inet_36(static_cast<const LibnetValueArgs *>(buffer)->mValue));
        break;
    case kFunctionRecvFrom: {
        const auto *args = static_cast<const LibnetTransferArgs *>(buffer);
        const int cid = args->mCid;
        const int count = args->mCount;
        const int timeout = args->mTimeout;
        auto *reply = static_cast<LibnetRecvFromReply *>(buffer);
        reply->mResult = sceInetRecvFrom(
            cid, reply->mData, count, &reply->mFlags, &reply->mAddress, &reply->mPort, timeout);
        break;
    }
    case kFunctionSendTo: {
        auto *args = static_cast<LibnetSendToArgs *>(buffer);
        auto *reply = static_cast<LibnetFlagsReply *>(buffer);
        reply->mResult = sceInetSendTo(args->mCid,
                                       args->mData,
                                       args->mCount,
                                       &reply->mFlags,
                                       &args->mAddress,
                                       args->mPort,
                                       args->mTimeout);
        break;
    }
    case kFunctionInet11: {
        const auto *args = static_cast<const LibnetConnectionArgs *>(buffer);
        SetResult(buffer, inet_11(args->mCid, args->mValue));
        break;
    }
    case kFunctionInet41:
        SetResult(buffer, inet_41());
        break;
    case kFunctionInet38: {
        const auto *args = static_cast<const LibnetPairArgs *>(buffer);
        const int first = args->mFirst;
        const int second = args->mSecond;
        auto *reply = static_cast<LibnetBufferReply *>(buffer);
        reply->mResult = inet_38(reply->mData, first, second);
        break;
    }
    case kFunctionInet14: {
        auto *args = static_cast<LibnetInet14Args *>(buffer);
        SetResult(buffer,
                  inet_14(args->mArgument0,
                          &args->mArgument6,
                          args->mArgument2,
                          args->mBuffer3,
                          args->mArgument4,
                          args->mArgument5,
                          args->mArgument6));
        break;
    }
    case kFunctionControl: {
        auto *args = static_cast<LibnetControlArgs *>(buffer);
        void *data = args->mSize != 0 ? args->mData : nullptr;
        SetResult(buffer, sceInetControl(args->mId, args->mCode, data, args->mSize));
        break;
    }
    case kFunctionInet16: {
        const auto *args = static_cast<const LibnetPairArgs *>(buffer);
        const int first = args->mFirst;
        const int second = args->mSecond;
        auto *reply = static_cast<LibnetOffsetBufferReply *>(buffer);
        reply->mResult = inet_16(reply->mData, first, second);
        break;
    }
    case kFunctionOpenEvents: {
        for (int i = InetEventQueue::kInterfaceCount - 1; i >= 0; --i) {
            InetEventQueue::sInterfaceIds[i] = 0;
        }
        InetEventQueue::sReadIndex = 0;
        InetEventQueue::sWriteIndex = 0;
        InetEventQueue::sFlags = 0;
        SemaParam param;
        param.attr = kSemaphoreAttribute;
        param.initCount = 1;
        param.maxCount = 1;
        param.option = 0;
        InetEventQueue::sLockSema = CreateSema(&param);
        if (InetEventQueue::sLockSema < 0) {
            SetResult(buffer, kLibnetErrorSemaphore);
            break;
        }
        param.attr = kSemaphoreAttribute;
        param.initCount = 0;
        param.maxCount = InetEventQueue::kQueueSize;
        param.option = 0;
        InetEventQueue::sCountSema = CreateSema(&param);
        if (InetEventQueue::sCountSema < 0) {
            SetResult(buffer, kLibnetErrorSemaphore);
            DeleteSema(InetEventQueue::sLockSema);
            break;
        }
        InetEventQueue::sHandler.handler = InetEventQueue::EventHandler;
        SetResult(buffer, sceInetCtlRegisterEventHandler(&InetEventQueue::sHandler));
        break;
    }
    case kFunctionCloseEvents:
        SetResult(buffer, sceInetCtlUnregisterEventHandler(&InetEventQueue::sHandler));
        if (DeleteSema(InetEventQueue::sCountSema) != KE_OK) {
            SetResult(buffer, kLibnetErrorSemaphore);
            DeleteSema(InetEventQueue::sLockSema);
            break;
        }
        if (DeleteSema(InetEventQueue::sLockSema) != KE_OK) {
            SetResult(buffer, kLibnetErrorSemaphore);
        }
        break;
    case kFunctionInetCtl4:
        SetResult(buffer, inetctl_4(static_cast<const LibnetEnvArgs *>(buffer)->mEnv));
        break;
    case kFunctionWaitInterfaceUp:
        WaitInterfaceState(InetEventQueue::kInterfaceUp, buffer);
        break;
    case kFunctionWaitInterfaceDown:
        WaitInterfaceState(InetEventQueue::kInterfaceDown, buffer);
        break;
    case kFunctionGetEvent: {
        auto *reply = static_cast<LibnetEventReply *>(buffer);
        reply->mResult = InetEventQueue::GetEvent(&reply->mInterfaceId, &reply->mEvent);
        break;
    }
    case kFunctionInetCtl5:
        SetResult(buffer, inetctl_5(static_cast<const LibnetValueArgs *>(buffer)->mValue));
        break;
    case kFunctionInetCtl6:
        SetResult(buffer, inetctl_6(static_cast<const LibnetValueArgs *>(buffer)->mValue));
        break;
    case kFunctionInetCtl7:
        SetResult(buffer, inetctl_7(static_cast<const LibnetValueArgs *>(buffer)->mValue));
        break;
    case kFunctionGetState: {
        const int interfaceId = static_cast<const LibnetValueArgs *>(buffer)->mValue;
        auto *reply = static_cast<LibnetStateReply *>(buffer);
        reply->mResult = sceInetCtlGetState(interfaceId, &reply->mState);
        break;
    }
    default:
        return HandleAsync(function, buffer);
    }
    return buffer;
}

void Libnet::RpcThread() {
    msifrpc_4(0);
    sceSifMServeData serve;
    msifrpc_17(&serve, kRpcServerId, RpcHandler, 0);
}

int Libnet::StartRpcThread() {
    ThreadParam param;
    param.stackSize = kRpcThreadStackSize;
    param.initPriority = kRpcThreadPriority;
    param.entry = RpcThread;
    param.attr = TH_C;
    param.option = 0;
    const int thread = CreateThread(&param);
    if (thread <= 0) {
        printf(kCreateThreadErrorMessage, thread);
        return NO_RESIDENT_END;
    }
    const int result = StartThread(thread, nullptr);
    if (result != KE_OK) {
        printf(kStartThreadErrorMessage, result);
        return NO_RESIDENT_END;
    }
    return RESIDENT_END;
}

void Libnet::ParseArguments(int argc, char **argv) {
    sConfig.Init();
    for (int i = 1; i < argc; ++i) {
        const char *argument = argv[i];
        if (strcmp(kVerboseOption, argument) == 0) {
            sVerbose = 1;
            sConfig.mVerbose = 1;
        } else if (strncmp(argument, kSendDelayOption, kSendDelayOptionLength) == 0 &&
                   strlen(argument) > static_cast<size_t>(kSendDelayOptionLength)) {
            sConfig.mSendDelay =
                static_cast<int>(strtol(&argument[kSendDelayOptionLength], nullptr, kDecimal));
        }
    }
}

const char *Libnet::GetVersion() {
    return sVersion;
}

bool Libnet::FormatBuildDate(char *text, int size) {
    if (text == nullptr) {
        return false;
    }
    constexpr char kDate[] = __DATE__;
    constexpr char kTime[] = __TIME__;
    char month[8];
    sprintf(month, kMonthFormat, kDate);
    const int monthNumber =
        static_cast<int>(strstr(kMonthNames, month) - kMonthNames) / kMonthNameLength + 1;
    const int day = static_cast<int>(strtol(&kDate[kDateDayOffset], nullptr, kDecimal));
    char formatted[32];
    sprintf(formatted,
            kBuildDateFormat,
            monthNumber,
            day,
            &kDate[kDateYearOffset],
            kTime,
            &kTime[kTimeMinuteOffset],
            &kTime[kTimeSecondOffset]);
    strncpy(text, formatted, size);
    return true;
}

void *Libnet::HandleAsync(unsigned int function, void *buffer) {
    switch (function) {
    case kFunctionStartAsyncRead: {
        auto *request = static_cast<AsyncInfo *>(buffer);
        AsyncAllocation *allocation = AllocateTransfer(request);
        if (allocation == nullptr) {
            printf(kAllocateHandlerErrorMessage);
            request->mOtherProcessorsInfo = nullptr;
            break;
        }
        AsyncInfo *info = &allocation->mInfo;
        CopyAsyncInfo(info, request);
        info->mThid = 0;
        info->mNext = nullptr;
        info->mBufferStart = allocation->mBuffer;
        info->mClose = 0;
        info->mOtherWriteLoc = request->mBufferStart;
        if (info->StartReadThread()) {
            FreeTransfer(allocation);
            printf(kCreateListenerErrorMessage);
            request->mOtherProcessorsInfo = nullptr;
            break;
        }
        info->AppendRead();
        request->mOtherProcessorsInfo = info;
        break;
    }
    case kFunctionStartAsyncSend: {
        auto *request = static_cast<AsyncInfo *>(buffer);
        AsyncAllocation *allocation = AllocateTransfer(request);
        if (allocation == nullptr) {
            printf(kAllocateHandlerErrorMessage);
            request->mOtherProcessorsInfo = nullptr;
            break;
        }
        AsyncInfo *info = &allocation->mInfo;
        CopyAsyncInfo(info, request);
        info->mThid = 0;
        info->mNext = nullptr;
        info->mBufferStart = allocation->mBuffer;
        if (info->StartSendThread()) {
            FreeTransfer(allocation);
            printf(kCreateListenerErrorMessage);
            request->mOtherProcessorsInfo = nullptr;
            break;
        }
        info->AppendSend();
        request->mOtherProcessorsInfo = info;
        request->mOtherWriteLoc = info->mBufferStart;
        request->mThid = info->mThid;
        break;
    }
    case kFunctionCloseAsync: {
        const auto *args = static_cast<const LibnetCloseAsyncArgs *>(buffer);
        const int cid = args->mCid;
        const int timeout = args->mTimeout;
        bool available = false;
        int remaining = timeout * kMicrosecondsPerMillisecond;
        AsyncInfo *readInfo = AsyncInfo::FindRead(cid);
        if (readInfo == nullptr) {
            SetResult(buffer, inet_8(cid, timeout));
            break;
        }
        AsyncInfo *sendInfo = AsyncInfo::FindSend(cid);
        if (sendInfo == nullptr) {
            break;
        }
        const bool blocks = sendInfo->mType == sceINETT_DGRAM || sendInfo->mType == sceINETT_RAW;
        sendInfo->PollBlocks(blocks, false, &available);
        // Let the pending sends finish. A negative timeout waits for as long as they take.
        while (available && (remaining > 0 || timeout < 0)) {
            DelayThread(kClosePollDelay);
            sendInfo->PollBlocks(blocks, false, &available);
            remaining -= kClosePollDelay;
        }
        readInfo->mClose = 1;
        sendInfo->mClose = 1;
        SetResult(buffer, inet_8(cid, timeout));
        if (!AsyncInfo::StopReadThread(cid, 0)) {
            printf(kDestroyThreadErrorMessage);
        }
        if (!AsyncInfo::StopSendThread(cid, 0)) {
            printf(kDestroyThreadErrorMessage);
        }
        break;
    }
    case kFunctionDumpState: {
        char built[kBuildDateSize];
        FormatBuildDate(built, kBuildDateSize - 1);
        printf(kNewline);
        printf(kStateRule);
        printf(kStateTitle);
        printf(kLineFormat, GetVersion());
        printf(kBuiltMessage, built);
        printf(kStateRule);
        AsyncInfo::DumpList(AsyncInfo::kListRead);
        AsyncInfo::DumpList(AsyncInfo::kListSend);
        DumpConnections();
        break;
    }
    default:
        printf(kUnhandledCaseMessage);
        SetResult(buffer, kLibnetErrorUnhandled);
        break;
    }
    return buffer;
}

void Libnet::PrintSendDelay() {
    printf(kSendDelayMessage, sSendDelay / kMicrosecondsPerMillisecond);
}

int Libnet::SendToEe(const void *data, void *destination, int size) {
    if (data == nullptr || destination == nullptr || size == 0) {
        return kResultBadArgument;
    }
    sceSifDmaData transfer;
    transfer.data = data;
    transfer.addr = destination;
    transfer.size = size;
    int state;
    const int suspended = CpuSuspendIntr(&state);
    transfer.mode = 0;
    const unsigned int id = sceSifSetDma(&transfer, 1);
    if (suspended == KE_OK) {
        CpuResumeIntr(state);
    }
    if (id == 0) {
        return kResultDmaFull;
    }
    while (sceSifDmaStat(id) >= 0) {
        DelayThread(kDmaPollDelay);
    }
    return 0;
}

void Libnet::DumpConnections() {
    for (AsyncInfo *info = AsyncInfo::sReadList; info != nullptr; info = info->mNext) {
        sceInetConnectionInfo connection;
        const int result = sceInetControl(
            info->mCid, INET_CONTROL_GET_CONNECTION_INFO, &connection, sizeof(connection));
        printf(kNewline);
        printf(kConnectionRule);
        printf(kConnectionTitle);
        printf(kConnectionRule);
        if (result < 0) {
            printf(kConnectionWarning, info->mCid, result);
            continue;
        }
        printf(kConnectionIdLine, connection.cid);
        switch (connection.proto) {
        case sceINETI_PROTO_TCP:
            printf(kProtocolTcpLine);
            break;
        case sceINETI_PROTO_UDP:
            printf(kProtocolUdpLine);
            break;
        case sceINETI_PROTO_IP:
            printf(kProtocolIpLine);
            break;
        default:
            break;
        }
        printf(kReceiveQueueLine, connection.recvQueueLength);
        printf(kSendQueueLine, connection.sendQueueLength);
        char text[kAddressTextSize];
        sceInetAddress2String(text, kAddressTextSize, &connection.localAddress);
        printf(kLocalReservedLine, connection.localAddress.reserved);
        printf(kLocalAddressLine, text);
        printf(kLocalPortLine, connection.localPort);
        sceInetAddress2String(text, kAddressTextSize, &connection.remoteAddress);
        printf(kRemoteReservedLine, connection.remoteAddress.reserved);
        printf(kRemoteAddressLine, text);
        printf(kRemotePortLine, connection.remotePort);
        if (static_cast<unsigned int>(connection.state) < kConnectionStateCount) {
            printf(kConnectionStateLines[connection.state]);
        }
    }
}

int Libnet::CreateNamedThread(ThreadParam *param,
                              const char *name,
                              void (*entry)(void *),
                              int stackSize,
                              int priority,
                              void *argument) {
    param->stackSize = stackSize;
    param->entryWithArgument = entry;
    param->initPriority = priority;
    const int thread = CreateThread(param);
    if (thread <= 0) {
        printf(kCreateNamedThreadError, name);
        return -1;
    }
    StartThread(thread, argument);
    return thread;
}

void Libnet::PrintInetError(int error) {
    if (error >= 0) {
        printf(kNoErrorMessage);
        return;
    }
    printf(kInetErrorPrefix, static_cast<long>(error));
    const char *message = kUnknownErrorMessage;
    if (error >= sceINETE_NO_ROUTE && error <= sceINETE_TIMEOUT) {
        message = kInetErrorMessages[error - sceINETE_NO_ROUTE];
    }
    printf(message);
}

int start(int argc, char **argv) {
    Libnet::ParseArguments(argc, argv);
    Libnet::sConfig.Apply();
    Libnet::PrintSendDelay();
    sceSifInitRpc(0);
    return Libnet::StartRpcThread();
}
