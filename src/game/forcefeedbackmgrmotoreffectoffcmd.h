#pragma once

#include "game/forcefeedbackmgrmotoreffect.h"
#include "os/command.h"

/**
 * Command that ends a MotorEffect once its duration has passed.
 *
 * The RTTI records the name and Command as the base. Every member is inline.
 */
class ForceFeedbackMgr::MotorEffect::OffCmd : public Command {
public:
    /**
     * Construct the command of an effect.
     *
     * @param pEffect The effect.
     * @ghidraAddress NTSC-U/C: 0x00335d20
     * @ghidraAddress PAL: 0x003a32d0
     */
    explicit OffCmd(MotorEffect *pEffect) : mEffect(pEffect) {
    }

    /**
     * Mark the effect as not running and stop the controller's motors.
     *
     * @ghidraAddress NTSC-U/C: 0x00336040
     * @ghidraAddress PAL: 0x003a35f0
     */
    void Execute() override {
        mEffect->mActive = 0;
        mEffect->SetMotors(mEffect->mController, 0, 0);
    }

    MotorEffect *mEffect; /*!< The effect. */
};
