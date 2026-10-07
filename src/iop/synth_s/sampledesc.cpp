#include "synth_s/sampledesc.h"

#include <stdio.h>

namespace {

enum SampleDescVersion {
    kSampleDescVersionKeys = 1,     // Through mSampleIndex.
    kSampleDescVersionBus = 2,      // Adds mBus and mBusMode.
    kSampleDescVersionSurround = 3, // Adds mSurround.
};

constexpr unsigned char kDefaultAttackRate = 15;
constexpr unsigned short kDefaultAttackMode = 1;
constexpr unsigned char kDefaultDecayRate = 1;
constexpr unsigned char kDefaultSusLevel = 15;
constexpr unsigned char kDefaultSusRate = 15;
constexpr unsigned char kDefaultReleaseRate = 7;
constexpr unsigned char kDefaultSampleDescVolume = 100;
constexpr unsigned char kCenterPan = 64;

// The bank file stores records back to back. Records are read and written unaligned. Each
// version is a prefix of the next.
struct __attribute__((packed)) SampleDescRecord {
    int mVersion;
    unsigned char mLowKeymap;
    unsigned char mHighKeymap;
    unsigned char mBaseKey;
    signed char mTranspose;
    signed char mFineTranspose;
    unsigned short mAttackMode;
    unsigned short mSusMode;
    unsigned short mReleaseMode;
    unsigned char mAttackRate;
    unsigned char mDecayRate;
    unsigned char mSusLevel;
    unsigned char mSusRate;
    unsigned char mReleaseRate;
    unsigned char mVolume;
    unsigned char mPan;
    unsigned short mSampleIndex;
    unsigned short mBus;
    unsigned short mBusMode;
    signed char mSurround;
};

constexpr int kKeysRecordSize = __builtin_offsetof(SampleDescRecord, mBus);
constexpr int kBusRecordSize = __builtin_offsetof(SampleDescRecord, mSurround);

} // namespace

SampleDesc::SampleDesc() {
    Init();
}

void SampleDesc::Init() {
    mLowKeymap = 0;
    mHighKeymap = 0;
    mBaseKey = 0;
    mTranspose = 0;
    mFineTranspose = 0;
    mAttackRate = kDefaultAttackRate;
    mAttackMode = kDefaultAttackMode;
    mDecayRate = kDefaultDecayRate;
    mSusLevel = kDefaultSusLevel;
    mSusRate = kDefaultSusRate;
    mSusMode = 0;
    mReleaseRate = kDefaultReleaseRate;
    mReleaseMode = 0;
    mVolume = kDefaultSampleDescVolume;
    mPan = kCenterPan;
    mSampleIndex = 0;
    mBus = 0;
    mBusMode = 0;
    mSurround = 0;
}

int SampleDesc::Pack(unsigned char *data) const {
    auto *record = static_cast<SampleDescRecord *>(static_cast<void *>(data));
    record->mVersion = kSampleDescVersionSurround;
    record->mLowKeymap = mLowKeymap;
    record->mHighKeymap = mHighKeymap;
    record->mBaseKey = mBaseKey;
    record->mTranspose = mTranspose;
    record->mFineTranspose = mFineTranspose;
    record->mAttackMode = mAttackMode;
    record->mSusMode = mSusMode;
    record->mReleaseMode = mReleaseMode;
    record->mAttackRate = mAttackRate;
    record->mDecayRate = mDecayRate;
    record->mSusLevel = mSusLevel;
    record->mSusRate = mSusRate;
    record->mReleaseRate = mReleaseRate;
    record->mVolume = mVolume;
    record->mPan = mPan;
    record->mSampleIndex = mSampleIndex;
    record->mBus = mBus;
    record->mBusMode = mBusMode;
    record->mSurround = mSurround;
    (void)PackedSize(); // Yes, retail calls PackedSize() and discards the result.
    return sizeof(SampleDescRecord);
}

int SampleDesc::PackedSize() const {
    return sizeof(SampleDescRecord);
}

int SampleDesc::Unpack(const unsigned char *data) {
    const auto *record = static_cast<const SampleDescRecord *>(static_cast<const void *>(data));
    int consumed = sizeof record->mVersion;
    const int version = record->mVersion;
    if ((version == kSampleDescVersionKeys) || (version == kSampleDescVersionBus) ||
        (version == kSampleDescVersionSurround)) {
        Init();
        mLowKeymap = record->mLowKeymap;
        mHighKeymap = record->mHighKeymap;
        mBaseKey = record->mBaseKey;
        mTranspose = record->mTranspose;
        mFineTranspose = record->mFineTranspose;
        mAttackMode = record->mAttackMode;
        mSusMode = record->mSusMode;
        mReleaseMode = record->mReleaseMode;
        mAttackRate = record->mAttackRate;
        mDecayRate = record->mDecayRate;
        mSusLevel = record->mSusLevel;
        mSusRate = record->mSusRate;
        mReleaseRate = record->mReleaseRate;
        mVolume = record->mVolume;
        mPan = record->mPan;
        mSampleIndex = record->mSampleIndex;
        consumed = kKeysRecordSize;
        if (version != kSampleDescVersionKeys) {
            mBus = record->mBus;
            mBusMode = record->mBusMode;
            consumed = kBusRecordSize;
            if (version == kSampleDescVersionSurround) {
                mSurround = record->mSurround;
                consumed = sizeof(SampleDescRecord);
            }
        }
    }
    Validate();
    return consumed;
}

void SampleDesc::Validate() {
}

void SampleDesc::Dump() const {
    printf("iSd->mLowKeymap = %i.\n", mLowKeymap);
    printf("iSd->mHighKeymap = %i.\n", mHighKeymap);
    printf("iSd->mBaseKey = %i.\n", mBaseKey);
    printf("iSd->mTranspose = %i.\n", mTranspose);
    printf("iSd->mFineTranspose = %i.\n", mFineTranspose);
    printf("iSd->mAttackRate = %i.\n", mAttackRate);
    printf("iSd->mAttackMode = %i.\n", mAttackMode);
    printf("iSd->mDecayRate = %i.\n", mDecayRate);
    printf("iSd->mSusLevel = %i.\n", mSusLevel);
    printf("iSd->mSusRate = %i.\n", mSusRate);
    printf("iSd->mSusMode = %i.\n", mSusMode);
    printf("iSd->mReleaseRate = %i.\n", mReleaseRate);
    printf("iSd->mReleaseMode = %i.\n", mReleaseMode);
    printf("iSd->mVolume = %i.\n", mVolume);
    printf("iSd->mPan = %i.\n", mPan);
    printf("iSd->mSampleIndex = %i.\n", mSampleIndex);
    printf("iSd->mBus = %i.\n", mBus);
    printf("iSd->mBusMode = %i.\n", mBusMode);
    printf("iSd->mSurround = %i.\n", mSurround);
}
