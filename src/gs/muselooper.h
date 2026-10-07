#pragma once

#include "gs/muse.h"
#include "os/command.h"
#include "os/ptr.h"
#include "os/scheduler.h"

/**
 * Player that repeats a piece of music over a fixed number of ticks.
 *
 * The RTTI includes the class name. The class is not polymorphic. Its nested command
 * MuseLooper::RepeatCmd restarts the piece each time a loop ends. The routines are not
 * reconstructed.
 */
class MuseLooper {
public:
    /**
     * Construct a stopped looper.
     *
     * @param pMuse The piece.
     * @param nLength The ticks of one loop.
     * @ghidraAddress NTSC-U/C: 0x0015bd28
     * @ghidraAddress PAL: 0x0015d518
     */
    MuseLooper(Muse *pMuse, int nLength);

    /**
     * Stop the piece and release it.
     *
     * @ghidraAddress NTSC-U/C: 0x0015bda8
     * @ghidraAddress PAL: 0x0015d598
     */
    ~MuseLooper();

    /**
     * Start the piece at a position of the loop on a scheduler, stopping it first.
     *
     * @param pScheduler The scheduler.
     * @param nPosition The position, taken modulo the length of the loop.
     * @ghidraAddress NTSC-U/C: 0x0015be10
     * @ghidraAddress PAL: 0x0015d600
     */
    void Play(Scheduler *pScheduler, int nPosition);

    /**
     * Stop the piece and withdraw the repeat.
     *
     * @ghidraAddress NTSC-U/C: 0x0015bec0
     * @ghidraAddress PAL: 0x0015d6b0
     */
    void Stop();

    /**
     * Report whether the piece plays.
     *
     * @return Whether the piece plays.
     * @ghidraAddress NTSC-U/C: 0x0015bf20
     * @ghidraAddress PAL: 0x0015d710
     */
    bool IsPlaying() const;

    Ptr<Command> mRepeatCmd; /*!< The MuseLooper::RepeatCmd that restarts the piece. */
    Ptr<Muse> mMuse;         /*!< The piece. */
    int mLength;             /*!< The ticks of one loop. */
    int mStartTick;          /*!< The tick of the scheduler the current loop started at. */
    Scheduler *mScheduler;   /*!< The scheduler the piece plays on, or null when stopped. */
};
