#include "memcard/memcardserialtask.h"

#include "memcard/memcardtask.h"

int MemcardSerialTask::GetResult() {
    return mResult;
}

void MemcardSerialTask::OnStop() {
    mResult = MemcardTask::sStatus;
}
