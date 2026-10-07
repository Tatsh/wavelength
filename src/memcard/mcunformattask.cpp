#include "memcard/mcunformattask.h"

#include "memcard/memcard.h"

void MCUnformatTask::OnUnformat(int nResult) {
    SetStatus(nResult);
    Finish(sStatus == kStatusOk);
}

void MCUnformatTask::OnStart() {
    MemcardUnformat(this, mPort);
}
