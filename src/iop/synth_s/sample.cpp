#include "synth_s/sample.h"

namespace {

constexpr int kSampleVersion = 1;

// The bank file stores records back to back. Records are read and written unaligned.
struct __attribute__((packed)) SampleRecord {
    int mVersion;
    unsigned short mSampleRate;
    unsigned int mLoopStart;
    unsigned int mLoopEnd;
    unsigned int mOffset;
};

} // namespace

Sample::Sample() {
    Init();
}

void Sample::Init() {
    mSampleRate = 0;
    mLoopStart = 0;
    mLoopEnd = 0;
    mOffset = 0;
}

int Sample::Unpack(const unsigned char *data) {
    const auto *record = static_cast<const SampleRecord *>(static_cast<const void *>(data));
    int consumed = sizeof record->mVersion;
    if (record->mVersion == kSampleVersion) {
        Init();
        mSampleRate = record->mSampleRate;
        mLoopStart = record->mLoopStart;
        mLoopEnd = record->mLoopEnd;
        mOffset = record->mOffset;
        consumed = sizeof(SampleRecord);
    }
    return consumed;
}

int Sample::Pack(unsigned char *data) const {
    auto *record = static_cast<SampleRecord *>(static_cast<void *>(data));
    record->mVersion = kSampleVersion;
    record->mSampleRate = mSampleRate;
    record->mLoopStart = mLoopStart;
    record->mLoopEnd = mLoopEnd;
    record->mOffset = mOffset;
    return sizeof(SampleRecord);
}

int Sample::PackedSize() const {
    return sizeof(SampleRecord);
}

void Sample::Dump() const {
}
