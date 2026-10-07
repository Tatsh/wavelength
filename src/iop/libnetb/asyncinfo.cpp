#include "libnetb/asyncinfo.h"

#include <stdio.h>

#include <inet.h>
#include <kernel.h>
#include <sif.h>

#include "libnetb/libnetb.h"
#include "libnetb/rawip.h"

namespace {

// Values of the kind argument of SendBlock() and ReceiveBlock().
enum BlockKind {
    kKindUdp = 0,
    kKindRaw = 1,
    kKindCount = 2,
};

constexpr int kCloseRequested = 1;
constexpr int kWaitForever = -1;
constexpr int kSendTimeout = 500;
constexpr int kIdleDelay = 2000;
constexpr int kPollDelay = 500;
constexpr int kDmaPollDelay = 200;
constexpr int kStopPollDelay = 100;
constexpr int kBlockSendDelay = 1000;
constexpr int kThreadStackSize = 0x800;
constexpr int kReadThreadPriority = 0x42;
constexpr int kSendThreadPriority = 0x41;
// The TCP threads mirror mDataWaitingSize and mLastReturnedFlag to the EE.
constexpr int kStatusSize = 8;
// The datagram send thread mirrors the record fields before mCid to the EE.
constexpr int kBlockStatusSize = 0x10;
// A raw IP datagram is only its two-byte port.
constexpr int kRawLength = 2;
// Receive flag the library sets when the remote side closed the connection.
constexpr int kInetFlagClosed = 0x04;
constexpr int kResultFailed = 1;
constexpr int kResultBadArgument = 2;

constexpr char kUnknownProtocolName[] = "???";
constexpr char kUdpName[] = "UDP";
constexpr char kRawName[] = "RAW";

constexpr char kReadThreadName[] = "libnet_asyncread";
constexpr char kSendThreadName[] = "libnet_asyncsend";
constexpr char kTcpReceiveTypeError[] = "RECV/TCP: Error - not a TCP socket!!!!\n";
constexpr char kTcpReceiveFailedMessage[] = "RECV/TCP: sceInetRecv() failed, ";
constexpr char kTcpReceiveDmaError[] = "RECV/TCP: Error DMAing data to EE memory!!!\n";
constexpr char kTcpReceiveExitMessage[] = "RECV/TCP: ASync listen thread exiting due to error, ";
constexpr char kTcpReceiveClosedMessage[] =
    "RECV/TCP: Socket closed remotely, ASync listen thread exiting\n";
constexpr char kBlockReceiveNullError[] = "RECV/BLOCK: Error - NULL passed to main thread\n";
constexpr char kBlockReceiveTypeError[] = "RECV/BLOCK: Error not a UDP or RAW socket!!!!\n";
constexpr char kBlockReceiveError[] = "RECV/%s: Error while receiving data\n";
constexpr char kTcpSendTypeError[] = "RECV/TCP: Error not a TCP socket!!!!\n";
constexpr char kTcpSendFailedMessage[] = "SEND/TCP: sceInetSend() failed, ";
constexpr char kTcpSendDmaError[] = "SEND/TCP: Error DMAing data to EE memory!!!\n";
constexpr char kBlockSendNullError[] = "SEND/BLOCK: Error - NULL passed to main thread\n";
constexpr char kBlockSendTypeError[] = "SEND/BLOCK: Error not a UDP or RAW socket!!!!\n";
constexpr char kBlockSendCheckError[] = "SEND/%s: Error while checking for data\n";
constexpr char kBlockSendError[] = "SEND/%s: Error while sending data\n";
constexpr char kSendToFailedMessage[] = "SEND/%s: sceInetSendTo() failed, ";
constexpr char kReceiveFromFailedMessage[] = "RECV/%s: sceInetRecvFrom() failed, ";
constexpr char kThreadStatusError[] = "Can't get status on thread\n";
constexpr char kDeleteThreadError[] = "Couldn't delete async receive handler's thread!\n";
constexpr char kTerminateThreadError[] = "Couldn't terminate async receive handler's thread!\n";

constexpr char kNewline[] = "\n";
constexpr char kDumpRule[] = "==============================================\n";
constexpr char kDumpReadTitle[] = "(READ) Doing a DUMP on the gASyncReadList List\n";
constexpr char kDumpSendTitle[] = "(SEND) Doing a DUMP on the gASyncSendList List\n";
constexpr char kStackLine[] = "Check Thread Stack size:\t'%d'\n";
constexpr char kThreadIdLine[] = "THIS ThreadID:\t'%d'\n";
constexpr char kEntryRule[] = "------------------------------------------------------\n";
constexpr char kReadEntryTitle[] = "(READ) gASyncReadList '#%d' -- sceaInetSimpleAsyncInfo\n";
constexpr char kSendEntryTitle[] = "(SEND) gASyncSendList '#%d' -- sceaInetSimpleAsyncInfo\n";
constexpr char kThidLine[] = " (thid)Async Thread ID:\t\t\t\t'%d'\n";
constexpr char kCidLine[] = " (cid)Thread Connection Index:\t\t\t'%d'\n";
constexpr char kCloseLine[] = " (close)Time to Close?:\t\t\t\t'%d'\n";
constexpr const char *kTypeLines[] = {
    " (type)Connection Type:\t\t\t\t'sceINETT_DGRAM -- UDP'\n",
    " (type)Connection Type:\t\t\t\t'sceINETT_CONNECT -- TCP, Active-Open'\n",
    " (type)Connection Type:\t\t\t\t'sceINETT_LISTEN -- TCP, Passive-Open'\n",
    " (type)Connection Type:\t\t\t\t'sceINETT_RAW -- IP'\n",
};
constexpr char kRemotePortLine[] = " (remote_port)Remote Port:\t\t\t'%d'\n";
constexpr char kDataWaitingLine[] = " (data_waiting_size)Data waiting Size:\t\t'%d'\n";
constexpr char kLastFlagLine[] = " (last_returned_flag)Last returned Flag:\t'%d'\n";
constexpr char kInetFlagLine[] = " (inet_flag)Inet Flag:\t\t\t\t'%d'\n";
constexpr char kBufferSizeLine[] = " (buffer_size)Buffer Size:\t\t\t'%u'\n";
constexpr char kBufferStartLine[] = " (buffer_start)Buffer Start:\t\t\t(*)'%p'\n";
constexpr char kOtherInfoLine[] = " (other_processors_info)Other Processors Info:\t(*)'%p'\n";
constexpr char kOtherWriteLine[] = " (other_write_loc)Other sides write location:\t(*)'%p'\n";
constexpr char kThreadStateWarning[] =
    "Warning!!:  Could not obtain the state of the specified thread ID:'%d'\n";
constexpr char kThreadStateTitle[] = "THREAD STATE:\n";
constexpr char kAttrLine[] = " (attr)Thread attribute set by CreateThread():\t\t'%u'\n";
constexpr char kOptionLine[] = " (option)Additional info set by CreateThread():\t\t'%u'\n";
constexpr char kStatusRunLine[] = " (status)The Thread State:\t\t\t\t'THS_RUN -- RUN State'\n";
constexpr char kStatusReadyLine[] =
    " (status)The Thread State:\t\t\t\t'THS_READY -- READY State'\n";
constexpr char kStatusWaitLine[] = " (status)The Thread State:\t\t\t\t'THS_WAIT -- WAIT State'\n";
constexpr char kStatusSuspendLine[] =
    " (status)The Thread State:\t\t\t\t'THS_SUSPEND -- SUSPEND State'\n";
constexpr char kStatusWaitSuspendLine[] =
    " (status)The Thread State:\t\t\t\t'THS_WAITSUSPEND -- WAIT-SUSPEND State'\n";
constexpr char kStatusDormantLine[] =
    " (status)The Thread State:\t\t\t\t'THS_DORMANT -- DORMANT State'\n";
constexpr char kEntryLine[] = " (entry)Entry address set by CreateThread:\t\t(*)'%p'\n";
constexpr char kStackStartLine[] = " (stack)Starting address of stack area:\t\t\t(*)'%p'\n";
constexpr char kStackSizeLine[] = " (stackSize)Stack Size:\t\t\t\t\t'%d'\n";
constexpr char kInitPriorityLine[] = " (initPriority)Thread startup priority:\t\t\t'%d'\n";
constexpr char kCurrentPriorityLine[] = " (currentPriority)Current Priority:\t\t\t'%d'\n";
constexpr const char *kWaitTypeLines[] = {
    " (waittype)The type of WAIT state:\t\t\t'TSW_SLEEP -- WAIT state due to SleepThread()'\n",
    " (waittype)The type of WAIT state:\t\t\t'TSW_DELAY -- WAIT state due to DelayThread()'\n",
    " (waittype)The type of WAIT state:\t\t\t'TSW_SEMA -- Semaphore WAIT state'\n",
    " (waittype)The type of WAIT state:\t\t\t'TSW_EVENTFLAG -- Event flag WAIT state'\n",
    " (waittype)The type of WAIT state:\t\t\t'TSW_MBX -- Message box WAIT state'\n",
    " (waittype)The type of WAIT state:\t\t\t"
    "'TSW_VPL -- Variable-length memory pool acquisition WAIT state'\n",
    " (waittype)The type of WAIT state:\t\t\t"
    "'TSW_FPL -- Fixed-length memory block acquisition WAIT state'\n",
};
constexpr char kWaitIdLine[] = " (waitId)ID of wait target of above waitType:\t\t'%d'\n";
constexpr char kWakeupCountLine[] = " (wakeupCount) Unprocessed WakeupThread() count:\t'%d'\n";

inline bool IsBlockType(int type) {
    return type == sceINETT_DGRAM || type == sceINETT_RAW;
}

// Copy an address field by field, where an assignment could become a call the module does not
// import.
inline void CopyAddress(sceInetAddress *destination, const sceInetAddress *source) {
    destination->reserved = source->reserved;
    for (unsigned int i = 0; i < sizeof(source->data); ++i) {
        destination->data[i] = source->data[i];
    }
}

// Release a transfer allocated with its buffer.
inline void FreeTransfer(AsyncInfo *info) {
    int state;
    const int suspended = CpuSuspendIntr(&state);
    FreeSysMemory(info);
    if (suspended == KE_OK) {
        CpuResumeIntr(state);
    }
}

} // namespace

// NTSC-U/C: 0x00005c94
AsyncInfo *AsyncInfo::sReadList = nullptr;

// NTSC-U/C: 0x00005c98
AsyncInfo *AsyncInfo::sSendList = nullptr;

inline void
AsyncInfo::CopyToEe(const void *data, void *destination, int size, const char *message) {
    sceSifDmaData transfer;
    transfer.data = data;
    transfer.size = size;
    transfer.mode = 0;
    int state;
    const int suspended = CpuSuspendIntr(&state);
    transfer.addr = destination;
    const unsigned int id = sceSifSetDma(&transfer, 1);
    if (suspended == KE_OK) {
        CpuResumeIntr(state);
    }
    if (id == 0) {
        printf(message);
    }
    while (sceSifDmaStat(id) >= 0) {
        DelayThread(kDmaPollDelay);
    }
}

void AsyncInfo::TcpReceiveThread(void *transfer) {
    auto *info = static_cast<AsyncInfo *>(transfer);
    if (IsBlockType(info->mType)) {
        printf(kTcpReceiveTypeError);
        ExitThread();
    }
    for (;;) {
        if (info->mClose == kCloseRequested) {
            ExitThread();
        }
        int flags = info->mInetFlag;
        const int count =
            sceInetRecv(info->mCid, info->mBufferStart, info->mBufferSize, &flags, kWaitForever);
        if (count < 0) {
            if (info->mClose == kCloseRequested) {
                ExitThread();
            }
            printf(kTcpReceiveFailedMessage);
            Libnet::PrintInetError(count);
#ifdef VIDEO_STANDARD_PAL
            while (info->mDataWaitingSize != 0) {
#else
            while (info->mDataWaitingSize <= 0) {
#endif
                if (info->mClose == kCloseRequested) {
                    ExitThread();
                }
                DelayThread(kPollDelay);
            }
            info->mDataWaitingSize = count;
            info->mLastReturnedFlag = flags;
            CopyToEe(info, info->mOtherProcessorsInfo, kStatusSize, kTcpReceiveDmaError);
            printf(kTcpReceiveExitMessage);
            Libnet::PrintInetError(count);
            ExitThread();
        }
        if (count == 0) {
            DelayThread(kIdleDelay);
            continue;
        }
        while (info->mDataWaitingSize != 0) {
            if (info->mClose == kCloseRequested) {
                ExitThread();
            }
            DelayThread(kPollDelay);
        }
        CopyToEe(info->mBufferStart, info->mOtherWriteLoc, count, kTcpReceiveDmaError);
        info->mDataWaitingSize = count;
        info->mLastReturnedFlag = flags;
        CopyToEe(info, info->mOtherProcessorsInfo, kStatusSize, kTcpReceiveDmaError);
        if ((flags & kInetFlagClosed) != 0) {
            printf(kTcpReceiveClosedMessage);
            ExitThread();
        }
    }
}

void AsyncInfo::BlockReceiveThread(void *transfer) {
    auto *info = static_cast<AsyncInfo *>(transfer);
    if (info == nullptr) {
        printf(kBlockReceiveNullError);
        ExitThread();
    }
    int kind;
    const char *name;
    if (info->mType == sceINETT_DGRAM) {
        name = kUdpName;
        kind = kKindUdp;
    } else if (info->mType == sceINETT_RAW) {
        name = kRawName;
        kind = kKindRaw;
    } else {
        name = kUnknownProtocolName;
        printf(kBlockReceiveTypeError);
        ExitThread();
    }
    for (;;) {
        if (info->mClose == kCloseRequested) {
            ExitThread();
        }
        if (info->ReceiveBlock(kind, name) != 0) {
            printf(kBlockReceiveError, name);
        }
    }
}

void AsyncInfo::TcpSendThread(void *transfer) {
    auto *info = static_cast<AsyncInfo *>(transfer);
    int count = 0;
    if (IsBlockType(info->mType)) {
        printf(kTcpSendTypeError);
        ExitThread();
    }
    for (;;) {
        if (info->mClose == kCloseRequested) {
            SleepThread();
        }
        DelayThread(Libnet::sSendDelay);
        if (info->mDataWaitingSize <= 0) {
            continue;
        }
        if (info->mClose == kCloseRequested) {
            SleepThread();
        }
        int flags = info->mInetFlag;
        int sent = 0;
        if (info->mDataWaitingSize != 0) {
            do {
                count = sceInetSend(info->mCid,
                                    &info->mBufferStart[sent],
                                    info->mDataWaitingSize - sent,
                                    &flags,
                                    kSendTimeout);
                if (info->mClose == kCloseRequested) {
                    SleepThread();
                }
                if (count < 0) {
                    printf(kTcpSendFailedMessage);
                    Libnet::PrintInetError(count);
                    break;
                }
                sent += count;
            } while (sent != info->mDataWaitingSize);
        }
        if (count == 0) {
            continue;
        }
        info->mDataWaitingSize = count < 0 ? count : 0;
        CopyToEe(info, info->mOtherProcessorsInfo, kStatusSize, kTcpSendDmaError);
    }
}

void AsyncInfo::BlockSendThread(void *transfer) {
    auto *info = static_cast<AsyncInfo *>(transfer);
    bool available = false;
    if (info == nullptr) {
        printf(kBlockSendNullError);
        ExitThread();
    }
    int kind;
    const char *name;
    if (info->mType == sceINETT_DGRAM) {
        name = kUdpName;
        kind = kKindUdp;
    } else if (info->mType == sceINETT_RAW) {
        name = kRawName;
        kind = kKindRaw;
    } else {
        name = kUnknownProtocolName;
        printf(kBlockSendTypeError);
        ExitThread();
    }
    for (;;) {
        if (info->mClose == kCloseRequested) {
            SleepThread();
        }
        if (info->PollBlocks(true, true, &available) != 0) {
            printf(kBlockSendCheckError, name);
        }
        DelayThread(Libnet::sSendDelay);
        if (!available) {
            continue;
        }
        do {
            if (info->SendBlock(kind, name) != 0) {
                printf(kBlockSendError, name);
            }
            DelayThread(kBlockSendDelay);
            if (info->PollBlocks(true, true, &available) != 0) {
                printf(kBlockSendCheckError, name);
            }
        } while (available);
        if (info->mClose == kCloseRequested) {
            SleepThread();
        }
    }
}

inline bool AsyncInfo::StartThread(const char *name,
                                   void (*blockEntry)(void *),
                                   void (*streamEntry)(void *),
                                   int priority) {
    ThreadParam param;
    param.attr = TH_C;
    param.option = 0;
    void (*entry)(void *);
    if (IsBlockType(mType)) {
        if (mBufferSize < static_cast<unsigned int>(mMaxPacketSize) + AsyncBlock::kHeaderSize) {
            return true;
        }
        if (AsyncBlock::At(mBufferStart, 0)->InitRing(mBufferSize) != 0) {
            return true;
        }
        entry = blockEntry;
    } else {
        entry = streamEntry;
    }
    mThid = Libnet::CreateNamedThread(&param, name, entry, kThreadStackSize, priority, this);
    return mThid == -1;
}

bool AsyncInfo::StartReadThread() {
    return StartThread(kReadThreadName, BlockReceiveThread, TcpReceiveThread, kReadThreadPriority);
}

bool AsyncInfo::StartSendThread() {
    return StartThread(kSendThreadName, BlockSendThread, TcpSendThread, kSendThreadPriority);
}

bool AsyncInfo::StopReadThread(int cid, int timeout) {
    AsyncInfo *info = FindRead(cid);
    if (info == nullptr) {
        return false;
    }
    bool dormant = false;
    if (timeout > 0) {
        ThreadInfo status;
        for (;;) {
            if (ReferThreadStatus(info->mThid, &status) != KE_OK) {
                printf(kThreadStatusError);
                return false;
            }
            DelayThread(kStopPollDelay);
            timeout -= kStopPollDelay;
            if (timeout <= 0) {
                break;
            }
            if (status.status == THS_DORMANT) {
                dormant = true;
                break;
            }
        }
    }
    if (!dormant) {
        TerminateThread(info->mThid);
    }
    if (DeleteThread(info->mThid) != KE_OK) {
        printf(kDeleteThreadError);
        return false;
    }
    info->RemoveRead();
    FreeTransfer(info);
    return true;
}

bool AsyncInfo::StopSendThread(int cid, int timeout) {
    AsyncInfo *info = FindSend(cid);
    if (info == nullptr) {
        return false;
    }
    if (timeout > 0) {
        ThreadInfo status;
        for (;;) {
            if (ReferThreadStatus(info->mThid, &status) != KE_OK) {
                printf(kThreadStatusError);
                return false;
            }
            DelayThread(kStopPollDelay);
            timeout -= kStopPollDelay;
            if (timeout <= 0 || status.status == THS_WAIT || status.waitType == TSW_SLEEP) {
                break;
            }
        }
    }
    if (TerminateThread(info->mThid) != KE_OK) {
        printf(kTerminateThreadError);
        return false;
    }
    if (DeleteThread(info->mThid) != KE_OK) {
        printf(kDeleteThreadError);
        return false;
    }
    info->RemoveSend();
    FreeTransfer(info);
    return true;
}

void AsyncInfo::AppendRead() {
    mNext = nullptr;
    if (sReadList == nullptr) {
        sReadList = this;
        return;
    }
    AsyncInfo *tail = sReadList;
    while (tail->mNext != nullptr) {
        tail = tail->mNext;
    }
    tail->mNext = this;
}

void AsyncInfo::AppendSend() {
    mNext = nullptr;
    if (sSendList == nullptr) {
        sSendList = this;
        return;
    }
    AsyncInfo *tail = sSendList;
    while (tail->mNext != nullptr) {
        tail = tail->mNext;
    }
    tail->mNext = this;
}

void AsyncInfo::RemoveRead() {
    if (sReadList == nullptr) {
        return;
    }
    if (sReadList == this) {
        sReadList = mNext;
        return;
    }
    AsyncInfo *previous = sReadList;
    while (previous->mNext != nullptr && previous->mNext != this) {
        previous = previous->mNext;
    }
    if (previous->mNext != nullptr) {
        previous->mNext = previous->mNext->mNext;
    }
}

void AsyncInfo::RemoveSend() {
    if (sSendList == nullptr) {
        return;
    }
    if (sSendList == this) {
        sSendList = mNext;
        return;
    }
    AsyncInfo *previous = sSendList;
    while (previous->mNext != nullptr && previous->mNext != this) {
        previous = previous->mNext;
    }
    if (previous->mNext != nullptr) {
        previous->mNext = previous->mNext->mNext;
    }
}

AsyncInfo *AsyncInfo::FindRead(int cid) {
    AsyncInfo *info = sReadList;
    while (info != nullptr && info->mCid != cid) {
        info = info->mNext;
    }
    return info;
}

AsyncInfo *AsyncInfo::FindSend(int cid) {
    AsyncInfo *info = sSendList;
    while (info != nullptr && info->mCid != cid) {
        info = info->mNext;
    }
    return info;
}

void AsyncInfo::DumpList(int list) {
    int count = 0;
    AsyncInfo *info = nullptr;
    switch (list) {
    case kListRead:
        info = sReadList;
        printf(kNewline);
        printf(kDumpRule);
        printf(kDumpReadTitle);
        printf(kDumpRule);
        break;
    case kListSend:
        info = sSendList;
        printf(kNewline);
        printf(kDumpRule);
        printf(kDumpSendTitle);
        printf(kDumpRule);
        break;
    default:
        break;
    }
    printf(kStackLine, CheckThreadStack());
    printf(kThreadIdLine, GetThreadId());
    for (; info != nullptr; info = info->mNext) {
        printf(kNewline);
        switch (list) {
        case kListRead:
            printf(kEntryRule);
            printf(kReadEntryTitle, count);
            printf(kEntryRule);
            break;
        case kListSend:
            printf(kEntryRule);
            printf(kSendEntryTitle, count);
            printf(kEntryRule);
            break;
        default:
            break;
        }
        printf(kThidLine, info->mThid);
        printf(kCidLine, info->mCid);
        printf(kCloseLine, info->mClose);
        if (info->mType >= sceINETT_DGRAM && info->mType <= sceINETT_RAW) {
            printf(kTypeLines[info->mType]);
        }
        printf(kRemotePortLine, info->mRemotePort);
        printf(kDataWaitingLine, info->mDataWaitingSize);
        printf(kLastFlagLine, info->mLastReturnedFlag);
        printf(kInetFlagLine, info->mInetFlag);
        printf(kBufferSizeLine, info->mBufferSize);
        printf(kBufferStartLine, info->mBufferStart);
        printf(kOtherInfoLine, info->mOtherProcessorsInfo);
        printf(kOtherWriteLine, info->mOtherWriteLoc);
        ThreadInfo status;
        if (ReferThreadStatus(info->mThid, &status) != KE_OK) {
            printf(kThreadStateWarning, info->mThid);
            return;
        }
        printf(kNewline);
        printf(kThreadStateTitle);
        printf(kAttrLine, status.attr);
        printf(kOptionLine, status.option);
        switch (status.status) {
        case THS_RUN:
            printf(kStatusRunLine);
            break;
        case THS_READY:
            printf(kStatusReadyLine);
            break;
        case THS_WAIT:
            printf(kStatusWaitLine);
            break;
        case THS_SUSPEND:
            printf(kStatusSuspendLine);
            break;
        case THS_WAITSUSPEND:
            printf(kStatusWaitSuspendLine);
            break;
        case THS_DORMANT:
            printf(kStatusDormantLine);
            break;
        default:
            break;
        }
        printf(kEntryLine, status.entry);
        printf(kStackStartLine, status.stack);
        printf(kStackSizeLine, status.stackSize);
        printf(kInitPriorityLine, status.initPriority);
        printf(kCurrentPriorityLine, status.currentPriority);
        if (status.waitType >= TSW_SLEEP && status.waitType <= TSW_FPL) {
            printf(kWaitTypeLines[status.waitType - TSW_SLEEP]);
        }
        printf(kWaitIdLine, status.waitId);
        ++count;
        printf(kWakeupCountLine, status.wakeupCount);
    }
}

int AsyncInfo::PollBlocks(bool blocks, bool sending, bool *available) {
    if (available == nullptr) {
        return kResultBadArgument;
    }
    *available = false;
    if (!blocks) {
        *available = mDataWaitingSize > 0;
        return 0;
    }
    if (!sending) {
        AsyncBlock *block = nullptr;
        bool found = false;
        const int error = FindFilledBlock(&block, &found);
        if (error != 0) {
            return error;
        }
        *available = found;
        return 0;
    }
    const AsyncBlock *block = AsyncBlock::At(mBufferStart, mWriteOffset);
    *available = block->mUsed == 1 && block->mValid == 1;
    return 0;
}

int AsyncInfo::FindFilledBlock(AsyncBlock **block, bool *found) {
    if (block == nullptr) {
        return kResultBadArgument;
    }
    *block = nullptr;
    if (found == nullptr) {
        return kResultBadArgument;
    }
    *found = false;
    unsigned int offset = mWriteOffset;
    AsyncBlock *candidate = AsyncBlock::At(mBufferStart, offset);
    while (candidate->mPadding == 1) {
        offset += AsyncBlock::kHeaderSize + candidate->mCapacity;
        if (offset >= mBufferSize) {
            offset = 0;
        }
        candidate = AsyncBlock::At(mBufferStart, offset);
        if (offset == mWriteOffset) {
            *block = nullptr;
            *found = false;
            return 0;
        }
    }
    if (candidate->mUsed == 1 && candidate->mValid == 1) {
        *block = candidate;
        *found = true;
        return 0;
    }
    *block = nullptr;
    *found = false;
    return 0;
}

int AsyncInfo::ReserveBlock(unsigned int size, unsigned int *reserved, bool *ready) {
    for (;;) {
        unsigned int freeBytes = 0;
        if (reserved == nullptr || ready == nullptr) {
            return kResultBadArgument;
        }
        // Round the block up to whole headers.
        unsigned int need = size + AsyncBlock::kHeaderSize;
        const unsigned int excess = need & (AsyncBlock::kHeaderSize - 1);
        if (excess != 0) {
            need = size + 2 * AsyncBlock::kHeaderSize - excess;
        }
        *reserved = need;
        if (mWriteOffset >= mBufferSize) {
            return kResultFailed;
        }
        AsyncBlock *block = AsyncBlock::At(mBufferStart, mWriteOffset);
        if (block->mUsed != 0 || block->mValid != 0) {
            *ready = false;
            return 0;
        }
        const unsigned int remaining = mBufferSize - mWriteOffset;
        int error = block->MeasureFree(remaining, &freeBytes);
        if (error != 0) {
            return error;
        }
        if (freeBytes < need) {
            if (remaining - freeBytes < AsyncBlock::kHeaderSize &&
                block != AsyncBlock::At(mBufferStart, 0)) {
                // Fill the end of the ring buffer and continue from its start.
                block->mUsed = 1;
                block->mPadding = 1;
                block->mCapacity = remaining - AsyncBlock::kHeaderSize;
                block->mLength = remaining - AsyncBlock::kHeaderSize;
                block->mSelf = block;
                block->mValid = 1;
                error =
                    Libnet::SendToEe(block, &mOtherWriteLoc[mWriteOffset], AsyncBlock::kHeaderSize);
                if (error != 0) {
                    return error;
                }
                mWriteOffset = 0;
                continue;
            }
            *ready = false;
            return 0;
        }
        if (need < freeBytes) {
            AsyncBlock *rest = AsyncBlock::At(mBufferStart, mWriteOffset + need);
            rest->mUsed = 0;
            rest->mPadding = 0;
            rest->mCapacity = freeBytes - need - AsyncBlock::kHeaderSize;
            rest->mLength = freeBytes - need - AsyncBlock::kHeaderSize;
            rest->mValid = 0;
            error = Libnet::SendToEe(
                rest, &mOtherWriteLoc[mWriteOffset + need], AsyncBlock::kHeaderSize);
            if (error != 0) {
                return error;
            }
        }
        block = AsyncBlock::At(mBufferStart, mWriteOffset);
        block->mUsed = 0;
        block->mPadding = 0;
        block->mCapacity = need - AsyncBlock::kHeaderSize;
        block->mLength = need - AsyncBlock::kHeaderSize;
        block->mValid = 0;
        error = Libnet::SendToEe(block, &mOtherWriteLoc[mWriteOffset], AsyncBlock::kHeaderSize);
        if (error != 0) {
            return error;
        }
        *ready = true;
        return 0;
    }
}

int AsyncInfo::SendBlock(int kind, const char *name) {
    int flags = 0;
    if (static_cast<unsigned int>(kind) >= kKindCount || name == nullptr) {
        return kResultBadArgument;
    }
    AsyncBlock *block = AsyncBlock::At(mBufferStart, mWriteOffset);
    if (block->mPadding != 0) {
        block->mResult = 0;
        block->mFlags = 0;
    } else {
        int result = 0;
        if (kind == kKindUdp) {
            result = sceInetSendTo(mCid,
                                   block->mData,
                                   block->mLength,
                                   &flags,
                                   &block->mAddress,
                                   block->mPort,
                                   kSendTimeout);
            if (result == sceINETE_INSUFFICIENT_RESOURCES) {
                return 0;
            }
            if (result < 0) {
                printf(kSendToFailedMessage, name);
                Libnet::PrintInetError(result);
            }
        } else {
            if (block->mLength < static_cast<unsigned int>(kRawLength)) {
                return kResultBadArgument;
            }
            const auto port = static_cast<unsigned short>(block->mData[0] | (block->mData[1] << 8));
            bool sent = false;
            const int error = RawIp::Send(mCid, port, &block->mAddress, &sent, name);
            if (error != 0) {
                return error;
            }
            result = sent ? kRawLength : 0;
        }
        if (result == 0) {
            return 0;
        }
        block->mResult = result;
        block->mFlags = flags;
    }
    const int result = block->mResult;
    block->mUsed = 0;
    block->mValid = 0;
    block->mPadding = 0;
    if (result < 0 || block->mFlags != 0) {
        if (mDataWaitingSize >= 0 && result < 0) {
            mDataWaitingSize = result;
        }
        if (mLastReturnedFlag == 0) {
            mLastReturnedFlag = block->mFlags;
        }
        const int error = Libnet::SendToEe(this, mOtherProcessorsInfo, kBlockStatusSize);
        if (error != 0) {
            return error;
        }
    }
    mWriteOffset += AsyncBlock::kHeaderSize + block->mCapacity;
    if (mWriteOffset >= mBufferSize) {
        mWriteOffset = 0;
    }
    return Libnet::SendToEe(block, block->mSelf, AsyncBlock::kHeaderSize);
}

int AsyncInfo::ReceiveBlock(int kind, const char *name) {
    bool ready = false;
    if (static_cast<unsigned int>(kind) >= kKindCount || name == nullptr) {
        return kResultBadArgument;
    }
    const int maxSize = mMaxPacketSize;
    unsigned int reserved;
    int error = ReserveBlock(maxSize, &reserved, &ready);
    if (error != 0) {
        return error;
    }
    if (!ready) {
        DelayThread(kPollDelay);
        return 0;
    }
    const unsigned int offset = mWriteOffset;
    AsyncBlock *block = AsyncBlock::At(mBufferStart, offset);
    const unsigned int capacity = block->mCapacity;
    sceInetAddress address;
    int flags = 0; // The binary leaves the flags uninitialised for raw IP.
    int port = 0;  // The binary leaves the port uninitialised when nothing arrives.
    int count;
    if (kind == kKindUdp) {
        count = sceInetRecvFrom(mCid, block->mData, maxSize, &flags, &address, &port, kWaitForever);
        if (count < 0) {
            printf(kReceiveFromFailedMessage, name);
            Libnet::PrintInetError(count);
        }
    } else {
        unsigned short rawPort = 0;
        bool received = false;
        if (static_cast<unsigned int>(maxSize) < static_cast<unsigned int>(kRawLength)) {
            return kResultBadArgument;
        }
        error = RawIp::Receive(mCid, &rawPort, &address, &received, name);
        if (error != 0) {
            return error;
        }
        count = 0;
        if (received) {
            port = rawPort;
            block->mData[0] = static_cast<unsigned char>(rawPort);
            block->mData[1] = static_cast<unsigned char>(rawPort >> 8);
            count = kRawLength;
        }
    }
    if (count == 0) {
        return 0;
    }
    if (count < 0) {
        block->mCapacity = 0;
        block->mLength = 0;
    } else {
        block->mLength = count;
        block->mCapacity = count;
        if ((count & (AsyncBlock::kHeaderSize - 1)) != 0) {
            block->mCapacity = (count & ~(AsyncBlock::kHeaderSize - 1)) + AsyncBlock::kHeaderSize;
        }
    }
    unsigned int size = block->mCapacity;
    block->mUsed = 1;
    block->mPadding = 0;
    CopyAddress(&block->mAddress, &address);
    block->mValid = 1;
    block->mSelf = block;
    block->mPort = port;
    block->mFlags = flags;
    const unsigned int leftover = capacity - size;
    if (leftover != 0) {
        AsyncBlock *rest = AsyncBlock::At(block->mData, size);
        size += AsyncBlock::kHeaderSize;
        rest->mUsed = 0;
        rest->mPadding = 0;
        rest->mCapacity = leftover - AsyncBlock::kHeaderSize;
        rest->mLength = leftover - AsyncBlock::kHeaderSize;
        rest->mValid = 0;
    }
    if (size != 0) {
        error =
            Libnet::SendToEe(block->mData, &mOtherWriteLoc[offset + AsyncBlock::kHeaderSize], size);
        if (error != 0) {
            return error;
        }
    }
    error = Libnet::SendToEe(block, &mOtherWriteLoc[offset], AsyncBlock::kHeaderSize);
    if (error != 0) {
        return error;
    }
    mWriteOffset += AsyncBlock::kHeaderSize + block->mCapacity;
    if (mWriteOffset >= mBufferSize) {
        mWriteOffset = 0;
    }
    return 0;
}
