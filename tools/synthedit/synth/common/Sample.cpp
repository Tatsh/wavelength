#include "synth/common/Sample.h"

#include <string.h>

#include "os/Debug.h"

namespace {

const int kSampleVersion = 1;

} // namespace

Sample::Sample() {
    Init();
}

void Sample::Init() {
    mSampleRate = 0;
    mLoopStart = 0;
    mLoopStop = 0;
    mSampleStart = 0;
}

int Sample::FromStream(const unsigned char *data) {
    int offset = 0;
    int version;
    memcpy(&version, data + offset, sizeof(version));
    offset += sizeof(version);
    if (version == kSampleVersion) {
        Init();
        memcpy(&mSampleRate, data + offset, sizeof(mSampleRate));
        offset += sizeof(mSampleRate);
        memcpy(&mLoopStart, data + offset, sizeof(mLoopStart));
        offset += sizeof(mLoopStart);
        memcpy(&mLoopStop, data + offset, sizeof(mLoopStop));
        offset += sizeof(mLoopStop);
        memcpy(&mSampleStart, data + offset, sizeof(mSampleStart));
        offset += sizeof(mSampleStart);
    } else {
        TheDebug.Printf("version# = %i.\n", version);
        ASSERT(false);
    }
    return offset;
}

int Sample::ToStream(unsigned char *data) const {
    int offset = 0;
    int version = kSampleVersion;
    memcpy(data + offset, &version, sizeof(version));
    offset += sizeof(version);
    memcpy(data + offset, &mSampleRate, sizeof(mSampleRate));
    offset += sizeof(mSampleRate);
    memcpy(data + offset, &mLoopStart, sizeof(mLoopStart));
    offset += sizeof(mLoopStart);
    memcpy(data + offset, &mLoopStop, sizeof(mLoopStop));
    offset += sizeof(mLoopStop);
    memcpy(data + offset, &mSampleStart, sizeof(mSampleStart));
    offset += sizeof(mSampleStart);
    return offset;
}

int Sample::StreamSize() {
    return sizeof(int) + sizeof(unsigned short) + sizeof(int) + sizeof(int) + sizeof(int);
}

void Sample::Dump() const {
    const Sample *iSamp = this;
    ASSERT(iSamp);
    TheDebug.Printf("iSamp->mSampleRate = %i.\n", iSamp->mSampleRate);
    TheDebug.Printf("iSamp->mLoopStart = %i.\n", iSamp->mLoopStart);
    TheDebug.Printf("iSamp->mLoopStop = %i.\n", iSamp->mLoopStop);
    TheDebug.Printf("iSamp->mSampleStart = %i.\n", iSamp->mSampleStart);
}
