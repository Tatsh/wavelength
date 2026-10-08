#include "os/Block.h"

#include "os/Debug.h"

namespace {

const int kBlockBufferSize = 0x10000;
const int kNoBlock = -1;

// 0x10041110
char gBlockBufferData[kNumBlockBuffers][kBlockBufferSize];

// 0x10038eec
char *gBlockBuffers[kNumBlockBuffers] = {
    gBlockBufferData[0], gBlockBufferData[1], gBlockBufferData[2], gBlockBufferData[3],
    gBlockBufferData[4], gBlockBufferData[5], gBlockBufferData[6], gBlockBufferData[7],
};

} // namespace

int kBlockSize = kBlockBufferSize;

int gCurrBuffNum;

int gBlockTimestamp;

int Block::NextBufferIndex() {
    ASSERT(gCurrBuffNum < kNumBlockBuffers);
    return gCurrBuffNum++;
}

Block::Block() {
    mBlockNum = kNoBlock;
    mBuffer = gBlockBuffers[NextBufferIndex()];
    UpdateTimestamp();
}

int Block::CurrentTimestamp() {
    return gBlockTimestamp;
}

void Block::UpdateTimestamp() {
    mTimestamp = ++gBlockTimestamp;
}
