#include "synth_s/program.h"

namespace {

constexpr int kProgramVersion = 1;
constexpr unsigned char kCenterPan = 64;
constexpr unsigned char kDefaultProgramVolume = 100;

// The bank file stores records back to back. Records are read and written unaligned.
struct __attribute__((packed)) ProgramRecord {
    int mVersion;
    unsigned short mProgram;
    unsigned char mPan;
    unsigned char mVolume;
    signed char mTranspose;
    signed char mFineTranspose;
    unsigned short mNumSampleDescs;
};

} // namespace

Program::Program() {
    Init();
}

void Program::Init() {
    mProgram = 0;
    mPan = kCenterPan;
    mVolume = kDefaultProgramVolume;
    mTranspose = 0;
    mFineTranspose = 0;
    mNumSampleDescs = 0;
}

int Program::Unpack(const unsigned char *data) {
    const auto *record = static_cast<const ProgramRecord *>(static_cast<const void *>(data));
    int consumed = sizeof record->mVersion;
    if (record->mVersion == kProgramVersion) {
        Init();
        mProgram = record->mProgram;
        mPan = record->mPan;
        mVolume = record->mVolume;
        mTranspose = record->mTranspose;
        mFineTranspose = record->mFineTranspose;
        mNumSampleDescs = record->mNumSampleDescs;
        consumed = sizeof(ProgramRecord);
    }
    return consumed;
}

int Program::Pack(unsigned char *data) const {
    auto *record = static_cast<ProgramRecord *>(static_cast<void *>(data));
    record->mVersion = kProgramVersion;
    record->mProgram = mProgram;
    record->mPan = mPan;
    record->mVolume = mVolume;
    record->mTranspose = mTranspose;
    record->mFineTranspose = mFineTranspose;
    record->mNumSampleDescs = mNumSampleDescs;
    return sizeof(ProgramRecord);
}

int Program::PackedSize() const {
    return sizeof(ProgramRecord);
}
