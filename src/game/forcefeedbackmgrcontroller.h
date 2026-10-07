#pragma once

#include <vector>

#include "game/forcefeedbackmgr.h"
#include "game/forcefeedbackmgrmotoreffect.h"

/**
 * One player's controller, with its beat pulse and its effects.
 *
 * The class is not polymorphic. The RTTI of a pointer to it records the name.
 */
class ForceFeedbackMgr::Controller {
public:
    /**
     * Construct the beat pulse and the effects of a controller.
     *
     * @param nController The controller.
     * @param nTicksPerBar The song ticks in one bar, which sets the crippler effect's duration.
     * @ghidraAddress NTSC-U/C: 0x0010c4b8
     * @ghidraAddress PAL: 0x0010dbf0
     */
    Controller(int nController, int nTicksPerBar);

    /**
     * Delete the effects and the beat pulse.
     *
     * @ghidraAddress NTSC-U/C: 0x0010c708
     * @ghidraAddress PAL: 0x0010de40
     */
    ~Controller();

    /**
     * Cancel the beat pulse and every effect.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0010c7f0
     * @ghidraAddress PAL: 0x0010df28
     */
    void CancelAll();

    /**
     * Report whether an effect runs. The program expands it at its one call.
     *
     * The name is inferred.
     *
     * @return Whether an effect runs.
     */
    bool IsPlayingEffect() const {
        for (const auto *pEffect : mEffects) {
            if (pEffect->mActive != 0) {
                return true;
            }
        }
        return false;
    }

    int mSmallMotor;                     /*!< The small motor's last state. */
    int mBigMotor;                       /*!< The big motor's last level. */
    int mVibration;                      /*!< The vibration option of the game. */
    int mEnabled;                        /*!< Whether the beat pulse drives the controller. */
    std::vector<MotorEffect *> mEffects; /*!< The effects, indexed by Effect. */
    MotorEffect *mBeatEffect;            /*!< The beat pulse. */
};
