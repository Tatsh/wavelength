#include "game/forcefeedbackmgrcontroller.h"

#include "game/forcefeedbackmgrmotoreffect.h"
#include "game/gameconfig.h"

namespace {

constexpr int kSmallMotorOff = 0;
constexpr int kSmallMotorOn = 1;
constexpr int kBigMotorOff = 0;
constexpr int kBigMotorFull = 255;
constexpr int kBigMotorAutocatch = 100;

// One beat and half a beat at 480 ticks per beat.
constexpr int kBumpTicks = 480;
constexpr int kAutocatchTicks = 240;

} // namespace

ForceFeedbackMgr::Controller::Controller(int nController, int nTicksPerBar)
    : mSmallMotor(0), mBigMotor(0), mEnabled(1), mEffects(kEffectCount) {
    // The configured duration is in milliseconds by its key, and the scheduler reads it as ticks.
    mBeatEffect =
        new MotorEffect(nController, kSmallMotorOn, kBigMotorOff, TheGameConfig->mBeatDurationMs);
    mEffects[kEffectCripple] = new MotorEffect(nController,
                                               kSmallMotorOn,
                                               kBigMotorFull,
                                               TheGameConfig->mCripplerDurationBars * nTicksPerBar);
    mEffects[kEffectBump] = new MotorEffect(nController, kSmallMotorOff, kBigMotorFull, kBumpTicks);
    mEffects[kEffectAutocatch] =
        new MotorEffect(nController, kSmallMotorOn, kBigMotorAutocatch, kAutocatchTicks);
}

ForceFeedbackMgr::Controller::~Controller() {
    for (auto *pEffect : mEffects) {
        delete pEffect;
    }
    delete mBeatEffect;
}

void ForceFeedbackMgr::Controller::CancelAll() {
    mBeatEffect->Cancel();
    for (auto *pEffect : mEffects) {
        pEffect->Cancel();
    }
}
