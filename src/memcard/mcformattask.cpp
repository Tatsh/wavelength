#include "memcard/mcformattask.h"

#include "memcard/memcard.h"

void MCFormatTask::Set(int nPort) {
    SetPort(nPort);
}

void MCFormatTask::OnFormat(int nResult) {
    SetStatus(nResult);
    Finish(sStatus == kStatusOk);
}

void MCFormatTask::OnStart() {
    MemcardFormat(this, mPort);
}
