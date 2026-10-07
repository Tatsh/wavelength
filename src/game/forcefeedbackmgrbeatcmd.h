#pragma once

#include "game/forcefeedbackmgr.h"
#include "game/forcefeedbackmgrcontroller.h"
#include "game/forcefeedbackmgrmotoreffect.h"
#include "os/command.h"
#include "os/scheduler.h"

/**
 * Command that runs the beat pulse once a period.
 *
 * The RTTI records the name and Command as the base. Every member is inline.
 */
class ForceFeedbackMgr::BeatCmd : public Command {
public:
    /** Construct the command with no period. */
    BeatCmd() : mPeriod(0) {
    }

    /**
     * Start the beat pulse of every enabled controller that plays no effect, and run again a period
     * later.
     *
     * @ghidraAddress NTSC-U/C: 0x00335e08
     * @ghidraAddress PAL: 0x003a33b8
     */
    void Execute() override {
        for (auto *pController : TheForceFeedbackMgr->mControllers) {
            if (pController->mEnabled && !pController->IsPlayingEffect()) {
                pController->mBeatEffect->Start();
            }
        }
        TheSongScheduler.PostIn(this, mPeriod, false);
    }

    int mPeriod; /*!< The ticks between two pulses. */
};
