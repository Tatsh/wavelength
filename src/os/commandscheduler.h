#pragma once

#include "os/command.h"
#include "os/commandid.h"

/**
 * Queue of commands that run at a song time or a song tick.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The one instance is
 * TheCommandScheduler. Each queued entry stores the command, its time in milliseconds, and its
 * tick, and the queue runs an entry when the clock passes its time. While the queue runs an entry,
 * mTime and mTick report that entry's time and tick, and otherwise they report the clock.
 *
 * The members before mTime are not yet recovered, and only the two members its callers read are
 * declared.
 */
class CommandScheduler {
public:
    /**
     * Report whether the clock of the queue runs.
     *
     * @return Whether the clock runs.
     * @ghidraAddress NTSC-U/C: 0x002824c0
     * @ghidraAddress PAL: 0x0028bd78
     */
    bool IsRunning();

    /**
     * Withdraw every queued entry for a command.
     *
     * @param pCommand The command.
     * @ghidraAddress NTSC-U/C: 0x002827c0
     * @ghidraAddress PAL: 0x0028c078
     */
    void Remove(Command *pCommand);

    /**
     * Queue a command to run a number of milliseconds after mTime.
     *
     * @param fDelayMs The delay in milliseconds.
     * @param pCommand The command.
     * @param bRecordable Whether a recording of the session stores the command.
     * @ghidraAddress NTSC-U/C: 0x00282638
     * @ghidraAddress PAL: 0x0028bef0
     */
    void AddAfterMs(float fDelayMs, Command *pCommand, bool bRecordable);

    /**
     * Queue a command to run at a song time.
     *
     * @param fTimeMs The time in milliseconds.
     * @param pCommand The command.
     * @param id The tag stored with the entry.
     * @param bRecordable Whether a recording of the session stores the command.
     * @ghidraAddress NTSC-U/C: 0x002825b0
     * @ghidraAddress PAL: 0x0028be68
     */
    void AddAtMs(float fTimeMs, Command *pCommand, const CommandId &id, bool bRecordable);

    /**
     * Queue a command to run at a song tick.
     *
     * @param pCommand The command.
     * @param nTick The tick.
     * @param bRecordable Whether a recording of the session stores the command.
     * @ghidraAddress NTSC-U/C: 0x00282690
     * @ghidraAddress PAL: 0x0028bf48
     */
    void AddAtTick(Command *pCommand, int nTick, bool bRecordable);

    /**
     * Queue a command to run a number of ticks after mTick.
     *
     * @param pCommand The command.
     * @param nTicks The delay in ticks.
     * @param bRecordable Whether a recording of the session stores the command.
     * @ghidraAddress NTSC-U/C: 0x002826e0
     * @ghidraAddress PAL: 0x0028bf98
     */
    void AddAfterTicks(Command *pCommand, int nTicks, bool bRecordable);

    float mTime; /*!< The current song time in milliseconds. +0x10 */
    int mTick;   /*!< The current song tick. +0x14 */
};

/**
 * The command scheduler of the song.
 *
 * @ghidraAddress NTSC-U/C: 0x00436218
 */
extern CommandScheduler TheCommandScheduler;
