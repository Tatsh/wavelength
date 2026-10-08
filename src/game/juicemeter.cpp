#include "game/juicemeter.h"

#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "game/helptext.h"
#include "game/stats.h"
#include "gfx/gfxmanager.h"
#include "os/scheduler.h"
#include "synth/fxmidi.h"

namespace {

constexpr int kFirstPlayer = 0;
constexpr float kUnsetMax = -1.0f;
constexpr float kRearmFraction = 1.0f / 3.0f;
constexpr float kLowFraction = 1.0f / 6.0f;

} // namespace

JuiceMeter::JuiceMeter(float fValue) : mValue(fValue), mMax(kUnsetMax), mWarned(0) {
    mMax = TheGameConfig->mJuiceMeterMax * TheGameConfig->mInitialJuice[TheGameDb->mSkillLevel];
    UpdateDisplay();
}

void JuiceMeter::Set(float fValue) {
    if (mMax < fValue) {
        fValue = mMax;
    }
    mValue = fValue;
    UpdateDisplay();
    TheStats->Juice(fValue, TheSongScheduler.mTick);
}

void JuiceMeter::Add(float fAmount) {
    CheckLow(mValue, mValue + fAmount);
    Set(mValue + fAmount);
}

float JuiceMeter::Get() const {
    return mValue;
}

float JuiceMeter::GetMax() const {
    return mMax;
}

void JuiceMeter::UpdateDisplay() {
    TheGfxManager.SetEnergy(kFirstPlayer, mValue / mMax);
    TheGfxManager.SetDying(kFirstPlayer, mValue <= 0.0f);
}

void JuiceMeter::CheckLow(float, float fNew) {
    if (mMax * kRearmFraction < fNew) {
        mWarned = 0;
    }
    if (!mWarned && fNew < mMax * kLowFraction) {
        FxMidi::PlayJuiceLowSound();
        (void)TheHelpText->ShowEnergyLow(); // The binary discards the result.
        mWarned = 1;
    }
}
