#include "synth/common/SampleDescription.h"

#include <stdio.h>
#include <string.h>

#include "os/Debug.h"

namespace {

const int kVersionWithoutBus = 1;
const int kVersionWithoutSurround = 2;
const int kSampleDescriptionVersion = 3;

const unsigned char kDefaultAttackRate = 15;
const unsigned char kDefaultDecayRate = 1;
const unsigned char kDefaultSusLevel = 15;
const unsigned char kDefaultSusRate = 15;
const unsigned char kDefaultReleaseRate = 7;
const unsigned char kDefaultVolume = 100;
const unsigned char kCentrePan = 64;

template <class T>
inline void ReadField(const unsigned char *data, int &offset, T &field) {
    memcpy(&field, data + offset, sizeof(field));
    offset += sizeof(field);
}

template <class T>
inline void WriteField(unsigned char *data, int &offset, const T &field) {
    memcpy(data + offset, &field, sizeof(field));
    offset += sizeof(field);
}

} // namespace

void SampleDescription::Validate() {
}

SampleDescription::SampleDescription() {
    Init();
}

void SampleDescription::Init() {
    mLowKeymap = 0;
    mHighKeymap = 0;
    mBaseKey = 0;
    mTranspose = 0;
    mFineTranspose = 0;
    mAttackRate = kDefaultAttackRate;
    mAttackMode = 1;
    mDecayRate = kDefaultDecayRate;
    mSusLevel = kDefaultSusLevel;
    mSusRate = kDefaultSusRate;
    mSusMode = 0;
    mReleaseRate = kDefaultReleaseRate;
    mReleaseMode = 0;
    mVolume = kDefaultVolume;
    mPan = kCentrePan;
    mSampleIndex = 0;
    mBus = 0;
    mBusMode = 0;
    mSurround = 0;
}

int SampleDescription::ToStream(unsigned char *data) const {
    int numBytesWritten = 0;
    int version = kSampleDescriptionVersion;
    WriteField(data, numBytesWritten, version);
    WriteField(data, numBytesWritten, mLowKeymap);
    WriteField(data, numBytesWritten, mHighKeymap);
    WriteField(data, numBytesWritten, mBaseKey);
    WriteField(data, numBytesWritten, mTranspose);
    WriteField(data, numBytesWritten, mFineTranspose);
    WriteField(data, numBytesWritten, mAttackMode);
    WriteField(data, numBytesWritten, mSusMode);
    WriteField(data, numBytesWritten, mReleaseMode);
    WriteField(data, numBytesWritten, mAttackRate);
    WriteField(data, numBytesWritten, mDecayRate);
    WriteField(data, numBytesWritten, mSusLevel);
    WriteField(data, numBytesWritten, mSusRate);
    WriteField(data, numBytesWritten, mReleaseRate);
    WriteField(data, numBytesWritten, mVolume);
    WriteField(data, numBytesWritten, mPan);
    WriteField(data, numBytesWritten, mSampleIndex);
    WriteField(data, numBytesWritten, mBus);
    WriteField(data, numBytesWritten, mBusMode);
    WriteField(data, numBytesWritten, mSurround);
    ASSERT(numBytesWritten == StreamSize());
    return numBytesWritten;
}

int SampleDescription::StreamSize() {
    return sizeof(int) + sizeof(mLowKeymap) + sizeof(mHighKeymap) + sizeof(mBaseKey) +
           sizeof(mTranspose) + sizeof(mFineTranspose) + sizeof(mAttackMode) + sizeof(mSusMode) +
           sizeof(mReleaseMode) + sizeof(mAttackRate) + sizeof(mDecayRate) + sizeof(mSusLevel) +
           sizeof(mSusRate) + sizeof(mReleaseRate) + sizeof(mVolume) + sizeof(mPan) +
           sizeof(mSampleIndex) + sizeof(mBus) + sizeof(mBusMode) + sizeof(mSurround);
}

int SampleDescription::FromStream(const unsigned char *data) {
    int offset = 0;
    int version;
    ReadField(data, offset, version);
    if (version == kVersionWithoutBus || version == kVersionWithoutSurround ||
        version == kSampleDescriptionVersion) {
        Init();
        ReadField(data, offset, mLowKeymap);
        ReadField(data, offset, mHighKeymap);
        ReadField(data, offset, mBaseKey);
        ReadField(data, offset, mTranspose);
        ReadField(data, offset, mFineTranspose);
        ReadField(data, offset, mAttackMode);
        ReadField(data, offset, mSusMode);
        ReadField(data, offset, mReleaseMode);
        ReadField(data, offset, mAttackRate);
        ReadField(data, offset, mDecayRate);
        ReadField(data, offset, mSusLevel);
        ReadField(data, offset, mSusRate);
        ReadField(data, offset, mReleaseRate);
        ReadField(data, offset, mVolume);
        ReadField(data, offset, mPan);
        ReadField(data, offset, mSampleIndex);
        if (version != kVersionWithoutBus) {
            ReadField(data, offset, mBus);
            ReadField(data, offset, mBusMode);
        }
        if (version == kSampleDescriptionVersion) {
            ReadField(data, offset, mSurround);
        }
    } else {
        TheDebug.Printf("version# = %i.\n", version);
        ASSERT(false);
    }
    Validate();
    return offset;
}

void SampleDescription::Dump() const {
    const SampleDescription *iSd = this;
    ASSERT(iSd);
    printf("iSd->mLowKeymap = %i.\n", iSd->mLowKeymap);
    printf("iSd->mHighKeymap = %i.\n", iSd->mHighKeymap);
    printf("iSd->mBaseKey = %i.\n", iSd->mBaseKey);
    printf("iSd->mTranspose = %i.\n", iSd->mTranspose);
    printf("iSd->mFineTranspose = %i.\n", iSd->mFineTranspose);
    printf("iSd->mAttackRate = %i.\n", iSd->mAttackRate);
    printf("iSd->mAttackMode = %i.\n", iSd->mAttackMode);
    printf("iSd->mDecayRate = %i.\n", iSd->mDecayRate);
    printf("iSd->mSusLevel = %i.\n", iSd->mSusLevel);
    printf("iSd->mSusRate = %i.\n", iSd->mSusRate);
    printf("iSd->mSusMode = %i.\n", iSd->mSusMode);
    printf("iSd->mReleaseRate = %i.\n", iSd->mReleaseRate);
    printf("iSd->mReleaseMode = %i.\n", iSd->mReleaseMode);
    printf("iSd->mVolume = %i.\n", iSd->mVolume);
    printf("iSd->mPan = %i.\n", iSd->mPan);
    printf("iSd->mSampleIndex = %i.\n", iSd->mSampleIndex);
    printf("iSd->mBus = %i.\n", iSd->mBus);
    printf("iSd->mBusMode = %i.\n", iSd->mBusMode);
    printf("iSd->mSurround = %i.\n", iSd->mSurround);
}
