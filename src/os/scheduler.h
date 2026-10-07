#pragma once

#include "os/command.h"
#include "os/commandid.h"

/**
 * Queue of commands that run when the song clock reaches their tick.
 *
 * The RTTI includes the name in the nested classes CommandInfo, ByCommand, ByID, and CancelPred.
 * The class is not polymorphic. This header declares only the members the tracks and the game
 * logic use.
 */
class Scheduler {
public:
    /**
     * Run a command a number of ticks from now.
     *
     * @param pCommand The command.
     * @param nDelayTicks The delay.
     * @param bRecordable The command is recorded for replay.
     * @ghidraAddress NTSC-U/C: 0x002826e0
     * @ghidraAddress PAL: 0x0028bf98
     */
    void PostIn(Command *pCommand, int nDelayTicks, bool bRecordable);

    /**
     * Run a command at a tick.
     *
     * @param pCommand The command.
     * @param nTick The tick.
     * @param bRecordable The command is recorded for replay.
     * @ghidraAddress NTSC-U/C: 0x00282690
     * @ghidraAddress PAL: 0x0028bf48
     */
    void PostAt(Command *pCommand, int nTick, bool bRecordable);

    /**
     * Withdraw every queued run of a command.
     *
     * @param pCommand The command.
     * @ghidraAddress NTSC-U/C: 0x002827c0
     * @ghidraAddress PAL: 0x0028c078
     */
    void Cancel(Command *pCommand);

    /**
     * Run a command a time from now.
     *
     * @param pCommand The command.
     * @param fDelay The delay, in milliseconds.
     * @param bRecordable The command is recorded for replay.
     * @ghidraAddress NTSC-U/C: 0x00282638
     * @ghidraAddress PAL: 0x0028bef0
     */
    void PostAfter(Command *pCommand, float fDelay, bool bRecordable);

    /**
     * Run a command at a time.
     *
     * The name is inferred.
     *
     * @param pCommand The command.
     * @param fTime The time, in milliseconds.
     * @param bRecordable The command is recorded for replay.
     * @ghidraAddress NTSC-U/C: 0x002825f0
     * @ghidraAddress PAL: 0x0028bea8
     */
    void PostAtTime(Command *pCommand, float fTime, bool bRecordable);

    /**
     * Run a command at a time, with a tag stored beside the entry.
     *
     * @param pCommand The command.
     * @param fTime The time, in milliseconds.
     * @param id The tag stored with the entry.
     * @param bRecordable The command is recorded for replay.
     * @ghidraAddress NTSC-U/C: 0x002825b0
     * @ghidraAddress PAL: 0x0028be68
     */
    void PostAtTime(Command *pCommand, float fTime, const CommandId &id, bool bRecordable);

    /**
     * Report whether the clock runs.
     *
     * @return Whether the clock runs.
     * @ghidraAddress NTSC-U/C: 0x002824c0
     * @ghidraAddress PAL: 0x0028bd78
     */
    bool IsRunning();

    /**
     * Stop the clock.
     *
     * @ghidraAddress NTSC-U/C: 0x00282478
     * @ghidraAddress PAL: 0x0028bd30
     */
    void Pause();

    /**
     * Restart the clock after Pause().
     *
     * @ghidraAddress NTSC-U/C: 0x00282440
     * @ghidraAddress PAL: 0x0028bcf8
     */
    void Resume();

    /**
     * Set the rate of the clock.
     *
     * @param fSpeed The rate, 1 for real time.
     * @ghidraAddress NTSC-U/C: 0x002824e0
     * @ghidraAddress PAL: 0x0028bd98
     */
    void SetSpeed(float fSpeed);

    /**
     * Report the clock time.
     *
     * @return The time, in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00282708
     * @ghidraAddress PAL: 0x0028bfc0
     */
    float GetClockTime();

    /**
     * Report the clock tick.
     *
     * @return The tick.
     * @ghidraAddress NTSC-U/C: 0x00282728
     * @ghidraAddress PAL: 0x0028bfe0
     */
    int GetClockTick();

    int mReserved00; // +0x00, not yet identified.
    int mReserved04; // +0x04, not yet identified.
    int mReserved08; // +0x08, not yet identified.
    int mReserved0C; // +0x0c, not yet identified.
    float mTime;     /*!< The song clock in milliseconds. */
    int mTick;       /*!< The song clock in ticks. */
};

/**
 * The scheduler of the song clock.
 *
 * @ghidraAddress NTSC-U/C: 0x00436218
 */
extern Scheduler TheSongScheduler;
