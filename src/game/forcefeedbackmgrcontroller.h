#pragma once

#include <memory>
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
        for (const auto &pEffect : mEffects) {
            if (pEffect->GetActive() != 0) {
                return true;
            }
        }
        return false;
    }

    /** @return The small motor's last state. */
    int GetSmallMotor() const {
        return mSmallMotor;
    }

    /**
     * Set the small motor's last state.
     *
     * @param nSmallMotor The state.
     */
    void SetSmallMotor(int nSmallMotor) {
        mSmallMotor = nSmallMotor;
    }

    /** @return The big motor's last level. */
    int GetBigMotor() const {
        return mBigMotor;
    }

    /**
     * Set the big motor's last level.
     *
     * @param nBigMotor The level.
     */
    void SetBigMotor(int nBigMotor) {
        mBigMotor = nBigMotor;
    }

    /** @return The vibration option of the game. */
    int GetVibration() const {
        return mVibration;
    }

    /**
     * Set the vibration option of the game.
     *
     * @param nVibration The option.
     */
    void SetVibration(int nVibration) {
        mVibration = nVibration;
    }

    /** @return Whether the beat pulse drives the controller. */
    int GetEnabled() const {
        return mEnabled;
    }

    /**
     * Set whether the beat pulse drives the controller.
     *
     * @param nEnabled Whether it does.
     */
    void SetEnabled(int nEnabled) {
        mEnabled = nEnabled;
    }

    /** @return The effects, indexed by Effect, deleted with the controller. */
    const std::vector<std::unique_ptr<MotorEffect>> &GetEffects() const {
        return mEffects;
    }

    /** @return The beat pulse. */
    MotorEffect *GetBeatEffect() {
        return mBeatEffect.get();
    }

private:
    int mSmallMotor; /*!< The small motor's last state. */
    int mBigMotor;   /*!< The big motor's last level. */
    int mVibration;  /*!< The vibration option of the game. */
    int mEnabled;    /*!< Whether the beat pulse drives the controller. */
    std::vector<std::unique_ptr<MotorEffect>> mEffects; /*!< The effects, indexed by Effect. */
    std::unique_ptr<MotorEffect> mBeatEffect;           /*!< The beat pulse. */
};
