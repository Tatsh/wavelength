#pragma once

#include "gs/muse.h"
#include "os/command.h"
#include "os/ptr.h"
#include "os/scheduler.h"

/**
 * Player that repeats a piece of music over a fixed number of ticks.
 *
 * The RTTI includes the class name. The class is not polymorphic. Its nested command
 * MuseLooper::RepeatCmd restarts the piece each time a loop ends.
 */
class MuseLooper {
public:
    /**
     * Command that restarts the piece of a looper at the end of each loop.
     *
     * The RTTI includes the nested name and records Command as the base.
     */
    class RepeatCmd : public Command {
    public:
        /**
         * Construct the command of a looper.
         *
         * @param pLooper The looper.
         */
        explicit RepeatCmd(MuseLooper *pLooper) : mLooper(pLooper) {
        }

        /**
         * Restart the piece and schedule the end of the new loop.
         *
         * @ghidraAddress NTSC-U/C: 0x0034eec8
         * @ghidraAddress PAL: 0x003bc2f0
         */
        void Execute() override;

    private:
        MuseLooper *mLooper; /*!< The looper. */
    };

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

    /**
     * Report the ticks of one loop.
     *
     * @return The ticks.
     * @ghidraAddress NTSC-U/C: 0x0015be08
     * @ghidraAddress PAL: 0x0015d5f8
     */
    int GetLength() const;

    /**
     * Report the position of the scheduler in the current loop.
     *
     * @return The ticks since the current loop started.
     * @ghidraAddress NTSC-U/C: 0x0015bf30
     * @ghidraAddress PAL: 0x0015d720
     */
    int GetPosition() const;

    Ptr<Command> mRepeatCmd; /*!< The MuseLooper::RepeatCmd that restarts the piece. */
    Ptr<Muse> mMuse;         /*!< The piece. */
    int mLength;             /*!< The ticks of one loop. */
    int mStartTick;          /*!< The tick of the scheduler the current loop started at. */
    Scheduler *mScheduler;   /*!< The scheduler the piece plays on, or null when stopped. */
};
