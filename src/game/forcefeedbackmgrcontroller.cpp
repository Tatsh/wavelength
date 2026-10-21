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
    mBeatEffect = std::make_unique<MotorEffect>(
        nController, kSmallMotorOn, kBigMotorOff, TheGameConfig->GetBeatDurationMs());
    mEffects[kEffectCripple] =
        std::make_unique<MotorEffect>(nController,
                                      kSmallMotorOn,
                                      kBigMotorFull,
                                      TheGameConfig->GetCripplerDurationBars() * nTicksPerBar);
    mEffects[kEffectBump] =
        std::make_unique<MotorEffect>(nController, kSmallMotorOff, kBigMotorFull, kBumpTicks);
    mEffects[kEffectAutocatch] = std::make_unique<MotorEffect>(
        nController, kSmallMotorOn, kBigMotorAutocatch, kAutocatchTicks);
}

ForceFeedbackMgr::Controller::~Controller() {
    // Preserve the original order: the effects first, then the beat pulse. Member destruction would
    // destroy the beat pulse first, because it is declared last.
    for (auto &pEffect : mEffects) {
        pEffect.reset();
    }
    mBeatEffect.reset();
}

void ForceFeedbackMgr::Controller::CancelAll() {
    mBeatEffect->Cancel();
    for (auto &pEffect : mEffects) {
        pEffect->Cancel();
    }
}
