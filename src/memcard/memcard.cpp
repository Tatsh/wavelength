#include "memcard/memcard.h"

#include <cstring>

#include <libmc.h>

#include "os/debug.h"
#include "os/memcardsync.h"

namespace {

// Entries one MemcardGetDir() lists at most.
constexpr int kMaxDirEntries = 20;

// The year a MemcardDirEntry date counts from.
constexpr int kDateBaseYear = 1900;

// The port or slot of a memory card slot that is unused.
constexpr int kNoSlot = -1;

// The result a SyncHandler reports before its command finishes.
constexpr int kNoResult = -1;

// Commands sceMcSync() reports.
enum Command {
    kCommandGetInfo = sceMcFuncNoCardInfo,
    kCommandOpen = sceMcFuncNoOpen,
    kCommandClose = sceMcFuncNoClose,
    kCommandSeek = sceMcFuncNoSeek,
    kCommandRead = sceMcFuncNoRead,
    kCommandWrite = sceMcFuncNoWrite,
    kCommandMkdir = sceMcFuncNoMkdir,
    kCommandGetDir = sceMcFuncNoGetDir,
    kCommandDelete = sceMcFuncNoDelete,
    kCommandFormat = sceMcFuncNoFormat,
    kCommandUnformat = sceMcFuncNoUnformat,
    kCommandGetEntSpace = sceMcFuncNoEntSpace,
    kCommandRename = sceMcFuncNoRename
};

// The controller port and multitap slot of a memory card slot.
struct CardSlot {
    int mPort;
    int mSlot;
};

// Handler that records the result of one command for the routines that wait for it.
// Destructor NTSC-U/C: 0x003a7020, PAL: 0x00415d00
class SyncHandler : public MemcardCBHandler {
public:
    SyncHandler() : mResult(kNoResult), mDone(0), mEntries(nullptr) {
    }

    // Run MemcardPoll() until the command finishes, and report its result.
    // NTSC-U/C: 0x0028bc88, PAL: 0x00295488
    int Wait() {
        while (mDone == 0) {
            MemcardPoll();
        }
        return mResult;
    }

    // Each override from 0x003a70c8 to 0x003a7178 records its result.
    void OnGetInfo(int nResult) override {
        Finish(nResult);
    }
    void OnGetEntSpace(int nResult) override {
        Finish(nResult);
    }
    void OnOpen(int nResult) override {
        Finish(nResult);
    }
    void OnClose(int nResult) override {
        Finish(nResult);
    }
    void OnRead(int nResult) override {
        Finish(nResult);
    }
    void OnWrite(int nResult) override {
        Finish(nResult);
    }
    void OnSeek(int nResult) override {
        Finish(nResult);
    }
    void OnMkdir(int nResult) override {
        Finish(nResult);
    }
    void OnRename(int nResult) override {
        Finish(nResult);
    }
    void OnDelete(int nResult) override {
        Finish(nResult);
    }
    void OnFormat(int nResult) override {
        Finish(nResult);
    }
    void OnUnformat(int nResult) override {
        Finish(nResult);
    }

    // NTSC-U/C: 0x003a7188
    void OnGetDir(int /*nResult*/, MemcardDirEntry *pEntries) override {
        mEntries = pEntries; // Yes, the binary drops the count and retains kNoResult.
        mDone = 1;
    }

    int mResult;
    int mDone;
    MemcardDirEntry *mEntries;

private:
    void Finish(int nResult) {
        mResult = nResult;
        mDone = 1;
    }
};

// The handler of the command in flight.
// NTSC-U/C: 0x003b2104
MemcardCBHandler *gHandler;

// Set while a command is in flight.
// NTSC-U/C: 0x003b2108
int gBusy;

// NTSC-U/C: 0x00481940
CardSlot gSlots[kMemcardSlotCount];

// The listing sceMcGetDir() fills.
// NTSC-U/C: 0x00481440
sceMcTblGetDir gDirTable[kMaxDirEntries] __attribute__((aligned(64)));

// The listing MemcardCBHandler::OnGetDir() receives.
// NTSC-U/C: 0x0028c070, PAL: 0x002959c0 (static initialiser)
// NTSC-U/C: 0x0028c0f0, PAL: 0x00295a40 (constructor call)
// NTSC-U/C: 0x00481000
MemcardDirEntry gDirEntries[kMaxDirEntries];

void CopyDate(DateTime &date, const sceMcStDateTime &card) {
    date.mYear = static_cast<unsigned char>(card.Year - kDateBaseYear);
    date.mMonth = card.Month;
    date.mDay = card.Day;
    date.mHour = card.Hour;
    date.mMinute = card.Min;
    date.mSecond = card.Sec;
}

// Convert the listing of a finished sceMcGetDir() and pass it to the handler.
inline void FinishGetDir(int nResult) {
    for (int i = 0; i < nResult; ++i) {
        MemcardDirEntry &entry = gDirEntries[i];
        const sceMcTblGetDir &card = gDirTable[i];
        strncpy(
            entry.mName, reinterpret_cast<const char *>(card.EntryName), kMemcardDirEntryNameSize);
        entry.mAttributes = card.AttrFile;
        entry.mSize = static_cast<int>(card.FileSizeByte);
        CopyDate(entry.mCreated, card._Create);
        CopyDate(entry.mModified, card._Modify);
    }
    gHandler->OnGetDir(nResult, gDirEntries);
}

} // namespace

void MemcardInit() {
    sceMcInitLibrary();
    MemcardAssignSlots(false, false);
}

void MemcardTerminate() {
    sceMcEnd();
}

bool operator<(const MemcardDirEntry &a, const MemcardDirEntry &b) {
    return a.mModified < b.mModified;
}

void MemcardPoll() {
    int nCommand = kNoResult;
    int nResult = kNoResult;
    const int nState = sceMcSync(sceMcNoWait, &nCommand, &nResult);
    if (nState == sceMcExecIdle || nState == sceMcExecRun) {
        return;
    }
    gBusy = 0;
    switch (nCommand) {
    case kCommandGetInfo:
        gHandler->OnGetInfo(nResult);
        break;
    case kCommandGetEntSpace:
        gHandler->OnGetEntSpace(nResult);
        break;
    case kCommandOpen:
        gHandler->OnOpen(nResult);
        break;
    case kCommandClose:
        gHandler->OnClose(nResult);
        break;
    case kCommandRead:
        gHandler->OnRead(nResult);
        break;
    case kCommandWrite:
        gHandler->OnWrite(nResult);
        break;
    case kCommandSeek:
        gHandler->OnSeek(nResult);
        break;
    case kCommandMkdir:
        gHandler->OnMkdir(nResult);
        break;
    case kCommandGetDir:
        FinishGetDir(nResult);
        break;
    case kCommandRename:
        gHandler->OnRename(nResult);
        break;
    case kCommandDelete:
        gHandler->OnDelete(nResult);
        break;
    case kCommandFormat:
        gHandler->OnFormat(nResult);
        break;
    case kCommandUnformat:
        gHandler->OnUnformat(nResult);
        break;
    default:
        DebugWarn(" Bad memcard func: %d", nCommand);
        break;
    }
}

// The waiting routines below leave gHandler at their finished SyncHandler, as retail does.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdangling-pointer"

void MemcardGetInfo(
    MemcardCBHandler *pHandler, int nPort, int *pnType, int *pnFree, int *pnFormat) {
    gBusy = 1;
    sceMcGetInfo(gSlots[nPort].mPort, gSlots[nPort].mSlot, pnType, pnFree, pnFormat);
    gHandler = pHandler;
}

void MemcardOpen(MemcardCBHandler *pHandler, int nPort, const char *pszName, int nMode) {
    gBusy = 1;
    sceMcOpen(gSlots[nPort].mPort, gSlots[nPort].mSlot, pszName, nMode);
    gHandler = pHandler;
}

void MemcardClose(MemcardCBHandler *pHandler, int nFd) {
    gBusy = 1;
    sceMcClose(nFd);
    gHandler = pHandler;
}

void MemcardRead(MemcardCBHandler *pHandler, int nFd, void *pBuffer, int nSize) {
    gBusy = 1;
    sceMcRead(nFd, pBuffer, nSize);
    gHandler = pHandler;
}

void MemcardWrite(MemcardCBHandler *pHandler, int nFd, const void *pData, int nSize) {
    gBusy = 1;
    sceMcWrite(nFd, pData, nSize);
    gHandler = pHandler;
}

void MemcardSeek(MemcardCBHandler *pHandler, int nFd, int nOffset, int nMode) {
    gBusy = 1;
    sceMcSeek(nFd, nOffset, nMode);
    gHandler = pHandler;
}

void MemcardMkdir(MemcardCBHandler *pHandler, int nPort, const char *pszName) {
    gBusy = 1;
    sceMcMkdir(gSlots[nPort].mPort, gSlots[nPort].mSlot, pszName);
    gHandler = pHandler;
}

void MemcardGetDir(
    MemcardCBHandler *pHandler, int nPort, const char *pszName, int nMaxEntries, int nMode) {
    gBusy = 1;
    const int nEntries = nMaxEntries > kMaxDirEntries ? kMaxDirEntries : nMaxEntries;
    sceMcGetDir(gSlots[nPort].mPort,
                gSlots[nPort].mSlot,
                pszName,
                static_cast<unsigned int>(nMode),
                nEntries,
                gDirTable);
    gHandler = pHandler;
}

void MemcardDelete(MemcardCBHandler *pHandler, int nPort, const char *pszName) {
    gBusy = 1;
    sceMcDelete(gSlots[nPort].mPort, gSlots[nPort].mSlot, pszName);
    gHandler = pHandler;
}

void MemcardFormat(MemcardCBHandler *pHandler, int nPort) {
    gBusy = 1;
    sceMcFormat(gSlots[nPort].mPort, gSlots[nPort].mSlot);
    gHandler = pHandler;
}

void MemcardUnformat(MemcardCBHandler *pHandler, int nPort) {
    gBusy = 1;
    sceMcUnformat(gSlots[nPort].mPort, gSlots[nPort].mSlot);
    gHandler = pHandler;
}

int MemcardGetInfoAndWait(int nDevice, int *pnType, int *pnFree, int *pnFormat) {
    SyncHandler handler;
    MemcardGetInfo(&handler, nDevice, pnType, pnFree, pnFormat);
    return handler.Wait();
}

int MemcardOpenAndWait(int nDevice, const char *pszPath, int nFlags) {
    SyncHandler handler;
    MemcardOpen(&handler, nDevice, pszPath, nFlags);
    return handler.Wait();
}

int MemcardCloseAndWait(int nFile) {
    SyncHandler handler;
    MemcardClose(&handler, nFile);
    return handler.Wait();
}

int MemcardWriteAndWait(int nFile, const void *pData, int nBytes) {
    SyncHandler handler;
    MemcardWrite(&handler, nFile, pData, nBytes);
    return handler.Wait();
}

int MemcardGetDirAndWait(
    int nDevice, const char *pszName, int nMaxEntries, int nMode, MemcardDirEntry **ppEntries) {
    SyncHandler handler;
    MemcardGetDir(&handler, nDevice, pszName, nMaxEntries, nMode);
    const int nResult = handler.Wait();
    *ppEntries = handler.mEntries;
    return nResult;
}

#pragma GCC diagnostic pop

void MemcardAssignSlots(bool bPort0Multitap, bool bPort1Multitap) {
    if (bPort0Multitap) {
        for (int i = 0; i < kMemcardSlotCount; ++i) {
            gSlots[i].mSlot = i;
            gSlots[i].mPort = 0;
        }
        return;
    }
    gSlots[0].mPort = 0;
    gSlots[0].mSlot = 0;
    if (!bPort1Multitap) {
        gSlots[1].mSlot = 0;
        gSlots[1].mPort = 1;
    } else {
        gSlots[1].mSlot = kNoSlot;
        gSlots[1].mPort = kNoSlot;
    }
    gSlots[3].mSlot = kNoSlot;
    gSlots[2].mPort = kNoSlot;
    gSlots[2].mSlot = kNoSlot;
    gSlots[3].mPort = kNoSlot;
}

const char *MemcardGetSlotName(int nPort) {
    if (gSlots[nPort].mPort == kNoSlot) {
        return "";
    }
    if (gSlots[kMemcardSlotCount - 1].mPort != kNoSlot) {
        switch (nPort) {
        case 0:
            return "mc_mtap_00";
        case 1:
            return "mc_mtap_01";
        case 2:
            return "mc_mtap_02";
        case 3:
            return "mc_mtap_03";
        default:
            return "";
        }
    }
    switch (nPort) {
    case 0:
        return "mc_00";
    case 1:
        return "mc_10";
    default:
        return "";
    }
}
