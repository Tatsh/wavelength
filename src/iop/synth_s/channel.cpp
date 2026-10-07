#include "synth_s/channel.h"

namespace {

constexpr unsigned char kDefaultVolume = 100;
constexpr unsigned char kDefaultExpression = 100;
constexpr unsigned char kCenterPan = 64;
constexpr unsigned char kDefaultPriority = 10;
constexpr unsigned char kControllerEnabled = 127;

} // namespace

void Channel::ResetControllers() {
    mVolume = kDefaultVolume;
    mExpression = kDefaultExpression;
    mPan = kCenterPan;
    mPriority = kDefaultPriority;
    mBusMode = kBusModeFromSampleDesc;
    mBus = 0;
    mDetune = 0;
    mStereo = 0;
    mMonophonic = 0;
    for (int i = 0; i < kNumControllerEnables; ++i) {
        mControllerEnable[i] = kControllerEnabled;
    }
}

void Channel::Init() {
    mBank = kNoBank;
    mProgram = kNoProgram;
    mVolume = kDefaultVolume;
    mExpression = kDefaultExpression;
    mPan = kCenterPan;
    mPitchBendSemitones = 0;
    mPitchBendCents = 0;
    mTranspose = 0;
    mPriority = kDefaultPriority;
    mBusMode = kBusModeFromSampleDesc;
    mBus = 0;
    mDetune = 0;
    mStereo = 0;
    mMonophonic = 0;
    for (int i = 0; i < kNumControllerEnables; ++i) {
        mControllerEnable[i] = kControllerEnabled;
    }
}

void Channel::SetTranspose(signed char transpose) {
    mTranspose = transpose;
}
