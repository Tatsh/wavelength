#include "os/AsyncTask.h"

#include <cstring>

#include "os/BlockMgr.h"

AsyncTask::AsyncTask(ArkFile *file, char *buffer, int blockNum, int start, int end) {
    mBlockNum = blockNum;
    mStart = start;
    mEnd = end;
    mBuffer = buffer;
    mFile = file;
}

bool AsyncTask::FillData() {
    const char *data = TheBlockMgr.GetBlockData(mBlockNum);
    if (data == NULL) {
        return false;
    }
    memcpy(mBuffer, &data[mStart], mEnd - mStart);
    mFile->TaskDone(mEnd - mStart);
    return true;
}
