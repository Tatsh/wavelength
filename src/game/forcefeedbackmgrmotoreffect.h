#pragma once

#include "game/forcefeedbackmgr.h"
#include "os/command.h"
#include "os/ptr.h"
#include "os/scheduler.h"

/**
 * One vibration pattern of a controller, which runs both motors for a duration.
 *
 * The class is not polymorphic. The RTTI of its nested OffCmd and of a pointer to it records the
 * name. Every member except the constructor is inline.
 */
class ForceFeedbackMgr::MotorEffect {
public:
    class OffCmd;

    /**
     * Construct an effect that is not running, with the command that ends it.
     *
     * Inline in the program, which expands it into Controller's constructor.
     *
     * @param nController The controller.
     * @param nSmallMotor The small motor's state while the effect runs.
     * @param nBigMotor The big motor's level while the effect runs.
     * @param nDuration The ticks the effect runs for.
     */
    MotorEffect(int nController, int nSmallMotor, int nBigMotor, int nDuration);

    /** Cancel the effect. */
    ~MotorEffect() {
        Cancel();
    }

    /**
     * Stop the motors and withdraw the command that ends the effect, when the effect runs.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00335cb8
     * @ghidraAddress PAL: 0x003a3268
     */
    void Cancel() {
        if (mActive) {
            mActive = 0;
            TheForceFeedbackMgr->SetVibration(mController, 0, 0);
            TheSongScheduler.Cancel(mOffCmd.Get());
        }
    }

    /**
     * Start the effect again from its beginning.
     *
     * The program expands it at every call. The name is inferred.
     */
    void Start() {
        if (mActive) {
            Cancel();
        }
        mActive = 1;
        TheForceFeedbackMgr->SetVibration(mController, mSmallMotor, mBigMotor);
        TheSongScheduler.PostIn(mOffCmd.Get(), mDuration, false);
    }

    /**
     * Pass motor levels to the manager.
     *
     * The name is inferred.
     *
     * @param nController The controller.
     * @param nSmallMotor The small motor's state.
     * @param nBigMotor The big motor's level.
     * @ghidraAddress NTSC-U/C: 0x00336110
     * @ghidraAddress PAL: 0x003a36c0
     */
    void SetMotors(int nController, int nSmallMotor, int nBigMotor) {
        TheForceFeedbackMgr->SetVibration(nController, nSmallMotor, nBigMotor);
    }

    int mController;      /*!< The controller. */
    int mSmallMotor;      /*!< The small motor's state while the effect runs. */
    int mBigMotor;        /*!< The big motor's level while the effect runs. */
    int mDuration;        /*!< The ticks the effect runs for. */
    Ptr<Command> mOffCmd; /*!< The OffCmd that ends the effect. */
    int mActive;          /*!< Whether the effect runs. */
};
