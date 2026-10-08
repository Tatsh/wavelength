#include "os/BlockRequest.h"

BlockRequest::BlockRequest(int blockNum, const AsyncTask &task) : mBlockNum(blockNum) {
    mTasks.insert(mTasks.end(), task);
}

BlockRequest::~BlockRequest() {
}
