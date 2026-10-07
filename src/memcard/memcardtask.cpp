#include "memcard/memcardtask.h"

#include <libmc.h>

#include "memcard/memcard.h"
#include "os/debug.h"

namespace {

// Every library result below this one means that the slot has no usable card.
constexpr int kLowestCardError = -9;

} // namespace

int MemcardTask::sFd;
int MemcardTask::sResult;
int MemcardTask::sStatus;
MemcardDirEntry *MemcardTask::sDirEntries;
int MemcardTask::sDirCount;

void MemcardTask::SetStatus(int nResult) {
    sResult = nResult;
    switch (nResult) {
    case sceMcResDeniedPermit:
        sStatus = kStatusNoCard;
        return;
    case sceMcResNoEntry:
        sStatus = kStatusNotFound;
        return;
    case sceMcResFullDevice:
        sStatus = kStatusFull;
        return;
    case sceMcResNoFormat:
        sStatus = kStatusUnformatted;
        return;
    case sceMcResChangedCard:
        sStatus = kStatusChangedCard;
        return;
    case sceMcResSucceed:
        sStatus = kStatusOk;
        return;
    default:
        break;
    }
    if (nResult < kLowestCardError) {
        sStatus = kStatusNoCard;
    } else if (nResult > 0) {
        sStatus = kStatusOk;
    } else {
        DebugWarn(" Unhandled memcard error code: %d\n", nResult);
    }
}

void MemcardTask::SetPort(int nPort) {
    (void)GetState(); // Yes, the binary discards this call's result.
    mPort = nPort;
}

void MemcardTask::OnPoll() {
    MemcardPoll();
}
