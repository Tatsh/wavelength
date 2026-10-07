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
     * Construct a stopped scheduler with no command.
     *
     * @ghidraAddress NTSC-U/C: 0x002820c0
     * @ghidraAddress PAL: 0x0028b978
     */
    Scheduler();

    /**
     * Release the queued commands.
     *
     * @ghidraAddress NTSC-U/C: 0x00282198
     * @ghidraAddress PAL: 0x0028ba50
     */
    ~Scheduler();

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

    /**
     * Stop the clock, drop every command, and set the clock to a tick at normal speed.
     *
     * The name is inferred.
     *
     * @param pTickDuration The duration of one tick in milliseconds, which the scheduler retains.
     * @param nTick The tick.
     * @ghidraAddress NTSC-U/C: 0x00282298
     * @ghidraAddress PAL: 0x0028bb50
     */
    void Reset(const float *pTickDuration, int nTick);

    /**
     * Reset the clock for a recorded song.
     *
     * The shipped body passes only the tick duration and the tick to Reset(). The name is
     * inferred.
     *
     * @param pTickDuration The duration of one tick in milliseconds.
     * @param pszFile The file to record to.
     * @param nDevice The memory card to record to, or -1 for the host.
     * @param nTick The tick.
     * @ghidraAddress NTSC-U/C: 0x00282360
     * @ghidraAddress PAL: 0x0028bc18
     */
    void ResetForRecording(const float *pTickDuration, const char *pszFile, int nDevice, int nTick);

    /**
     * Reset the clock and replay the commands a file recorded.
     *
     * The name is inferred.
     *
     * @param pTickDuration The duration of one tick in milliseconds.
     * @param pszFile The recording.
     * @param nTick The tick.
     * @ghidraAddress NTSC-U/C: 0x00282380
     * @ghidraAddress PAL: 0x0028bc38
     */
    void ResetForPlayback(const float *pTickDuration, const char *pszFile, int nTick);

    /**
     * Reset the clock and replay the commands a buffer recorded.
     *
     * The name is inferred.
     *
     * @param pTickDuration The duration of one tick in milliseconds.
     * @param pBuffer The recording.
     * @param nSize The size of the recording in bytes.
     * @param nTick The tick.
     * @ghidraAddress NTSC-U/C: 0x002823d8
     * @ghidraAddress PAL: 0x0028bc90
     */
    void ResetForPlayback(const float *pTickDuration,
                          const unsigned char *pBuffer,
                          int nSize,
                          int nTick);

    /**
     * Drop every queued command.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00282768
     * @ghidraAddress PAL: 0x0028c020
     */
    void Clear();

    /**
     * Advance the clock to the current time and run the commands it passed.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002827f0
     * @ghidraAddress PAL: 0x0028c0a8
     */
    void Poll();

    int mReserved00;            // +0x00, not yet identified.
    int mReserved04;            // +0x04, not yet identified.
    int mReserved08;            // +0x08, not yet identified.
    const float *mTickDuration; /*!< The duration of one tick Reset() was given. */
    float mTime;                /*!< The song clock in milliseconds. */
    int mTick;                  /*!< The song clock in ticks. */
    float mFrameTime;           /*!< The clock time the last pump ran the commands up to. */
    int mFrameTick;             /*!< The clock tick the last pump ran the commands up to. */
    float mPrevFrameTime;       /*!< The clock time the pump before the last ran up to. */
    int mPrevFrameTick;         /*!< The clock tick the pump before the last ran up to. */
};

/**
 * The scheduler of the song clock.
 *
 * @ghidraAddress NTSC-U/C: 0x00436218
 */
extern Scheduler TheSongScheduler;

/**
 * The scheduler of the front end clock, which times the menu music.
 *
 * The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x00436910
 */
extern Scheduler TheMetaScheduler;
