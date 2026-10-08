#pragma once

#include <set>

#include "os/binstream.h"
#include "os/command.h"
#include "os/commandid.h"
#include "os/ptr.h"
#include "os/schedulerclock.h"
#include "os/string.h"

/**
 * Queue of commands that run when the song clock reaches their tick.
 *
 * The RTTI includes the name in the nested classes CommandInfo, ByCommand, ByID, and CancelPred.
 * The class is not polymorphic. A recordable command can be written to a recording as it runs, and
 * a recording replays its commands into the queue.
 */
class Scheduler {
public:
    /**
     * One queued run of a command.
     *
     * The RTTI includes the nested name. The class is not polymorphic, and the object is 0x14
     * bytes. The queue orders the runs by time, and a recordable run before another at the same
     * time.
     */
    class CommandInfo {
    public:
        /**
         * Report whether this run comes before another.
         *
         * @param other The other run.
         * @return Whether this run comes first.
         * @ghidraAddress NTSC-U/C: 0x00281ad8
         * @ghidraAddress PAL: 0x0028b3d8
         */
        bool operator<(const CommandInfo &other) const;

        /**
         * Write the run to a recording.
         *
         * @param stream The stream to write to.
         * @ghidraAddress NTSC-U/C: 0x00281b28
         * @ghidraAddress PAL: 0x0028b428
         */
        void Save(BinStream &stream) const;

        /**
         * Read a run Save() wrote. The tag becomes TheDefaultCommandId.
         *
         * @param stream The stream to read from.
         * @ghidraAddress NTSC-U/C: 0x00281bc0
         * @ghidraAddress PAL: 0x0028b4b0
         */
        void Load(BinStream &stream);

        Ptr<Command> mCommand; /*!< The command. */
        float mTime;           /*!< The time it runs at, in milliseconds. */
        int mTick;             /*!< The tick it runs at. */
        CommandId mId;         /*!< The tag stored with the run. */
        bool mRecordable;      /*!< Whether a recording writes the run. */
    };

    /**
     * Test that selects the runs Cancel() withdraws.
     *
     * The RTTI includes the nested name. The class has no data member and no destructor.
     */
    class CancelPred {
    public:
        /**
         * Report whether a run is withdrawn.
         *
         * @param info The run.
         * @return Whether it is withdrawn.
         */
        virtual bool operator()(const CommandInfo &info) const = 0;
    };

    /**
     * Test that selects the runs of one command.
     *
     * The RTTI includes the nested name and records CancelPred as the base, and the vtable is at
     * `0x003d68b0`.
     */
    class ByCommand : public CancelPred {
    public:
        /**
         * Construct the test of a command.
         *
         * @param pCommand The command.
         */
        explicit ByCommand(Command *pCommand) : mCommand(pCommand) {
        }

        /**
         * Report whether a run is of the command.
         *
         * @param info The run.
         * @return Whether the run's command is the command.
         * @ghidraAddress NTSC-U/C: 0x003a6b38
         * @ghidraAddress PAL: 0x00415818
         */
        bool operator()(const CommandInfo &info) const override;

    private:
        Command *mCommand; // The command.
    };

    /**
     * Writer of the recordable runs to a file of a memory card, in parts.
     *
     * The class is not polymorphic and has no RTTI. The name is inferred. The shipped build never
     * constructs one, since ResetForRecording() does not create it.
     */
    class Recorder {
    public:
        /**
         * Save the runs not yet saved and release the stream.
         *
         * @ghidraAddress NTSC-U/C: 0x00281c78
         * @ghidraAddress PAL: 0x0028b560
         */
        ~Recorder();

        /**
         * Save the runs not yet saved, when a memory card is set.
         *
         * @ghidraAddress NTSC-U/C: 0x00281ce8
         * @ghidraAddress PAL: 0x0028b5d0
         */
        void Flush();

        /**
         * Write a run to the stream.
         *
         * @param info The run.
         * @ghidraAddress NTSC-U/C: 0x00281d18
         * @ghidraAddress PAL: 0x0028b600
         */
        void Record(const CommandInfo &info);

    private:
        /**
         * Append the stream to the file, creating it with the first part, and start a new stream.
         *
         * @ghidraAddress NTSC-U/C: 0x00281d68
         * @ghidraAddress PAL: 0x0028b650
         */
        void Save();

        BinStream *mStream; // The runs not yet saved.
        String mFile;       // The file on the memory card.
        int mDevice;        // The memory card, or -1 for none.
        int mParts;         // The number of parts saved.
    };

    /**
     * Reader of a recording that queues its runs on a scheduler.
     *
     * The class is not polymorphic and has no RTTI. The name is inferred.
     */
    class Playbacker {
    public:
        /**
         * Open a recording in a file.
         *
         * @param pszFile The file.
         * @param pScheduler The scheduler the runs are queued on.
         * @ghidraAddress NTSC-U/C: 0x00281ec8
         * @ghidraAddress PAL: 0x0028b780
         */
        Playbacker(const char *pszFile, Scheduler *pScheduler);

        /**
         * Open a recording in a buffer.
         *
         * @param pBuffer The recording. The caller retains it.
         * @param nSize The size of the recording in bytes.
         * @param pScheduler The scheduler the runs are queued on.
         * @ghidraAddress NTSC-U/C: 0x00281f30
         * @ghidraAddress PAL: 0x0028b7e8
         */
        Playbacker(char *pBuffer, int nSize, Scheduler *pScheduler);

        /**
         * Release the stream.
         *
         * @ghidraAddress NTSC-U/C: 0x00281fa0
         * @ghidraAddress PAL: 0x0028b858
         */
        ~Playbacker();

        /**
         * Queue every run of the recording.
         *
         * The name is inferred.
         *
         * @ghidraAddress NTSC-U/C: 0x00282000
         * @ghidraAddress PAL: 0x0028b8b8
         */
        void QueueAll();

    private:
        BinStream *mStream;    // The recording.
        Scheduler *mScheduler; // The scheduler the runs are queued on.
    };

    /**
     * Construct a stopped scheduler with no command.
     *
     * @ghidraAddress NTSC-U/C: 0x002820c0
     * @ghidraAddress PAL: 0x0028b978
     */
    Scheduler();

    /**
     * Release the queued commands, the recorder, and the playback.
     *
     * @ghidraAddress NTSC-U/C: 0x00282198
     * @ghidraAddress PAL: 0x0028ba50
     */
    ~Scheduler();

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
    void ResetForPlayback(const float *pTickDuration, char *pBuffer, int nSize, int nTick);

    /**
     * Restart the clock after Pause().
     *
     * @ghidraAddress NTSC-U/C: 0x00282440
     * @ghidraAddress PAL: 0x0028bcf8
     */
    void Resume();

    /**
     * Stop the clock, and save what the recorder holds.
     *
     * @ghidraAddress NTSC-U/C: 0x00282478
     * @ghidraAddress PAL: 0x0028bd30
     */
    void Pause();

    /**
     * Report whether the clock runs.
     *
     * @return Whether the clock runs.
     * @ghidraAddress NTSC-U/C: 0x002824c0
     * @ghidraAddress PAL: 0x0028bd78
     */
    bool IsRunning();

    /**
     * Set the rate of the clock.
     *
     * @param fSpeed The rate, 1 for real time.
     * @ghidraAddress NTSC-U/C: 0x002824e0
     * @ghidraAddress PAL: 0x0028bd98
     */
    void SetSpeed(float fSpeed);

    /**
     * Report the rate of the clock.
     *
     * The name is inferred.
     *
     * @return The rate.
     * @ghidraAddress NTSC-U/C: 0x00282500
     * @ghidraAddress PAL: 0x0028bdb8
     */
    float GetSpeed();

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
     * Run a command a time from now, with a tag stored beside the entry.
     *
     * @param pCommand The command.
     * @param fDelay The delay, in milliseconds.
     * @param id The tag stored with the entry.
     * @param bRecordable The command is recorded for replay.
     * @ghidraAddress NTSC-U/C: 0x00282618
     * @ghidraAddress PAL: 0x0028bed0
     */
    void PostAfter(Command *pCommand, float fDelay, const CommandId &id, bool bRecordable);

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
     * Run a command at a tick, with a tag stored beside the entry.
     *
     * @param pCommand The command.
     * @param nTick The tick.
     * @param id The tag stored with the entry.
     * @param bRecordable The command is recorded for replay.
     * @ghidraAddress NTSC-U/C: 0x00282660
     * @ghidraAddress PAL: 0x0028bf18
     */
    void PostAt(Command *pCommand, int nTick, const CommandId &id, bool bRecordable);

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
     * Run a command a number of ticks from now.
     *
     * The entry is stored with TheDefaultCommandId, whatever id is.
     *
     * @param pCommand The command.
     * @param nDelayTicks The delay.
     * @param id Not used.
     * @param bRecordable The command is recorded for replay.
     * @ghidraAddress NTSC-U/C: 0x002826b8
     * @ghidraAddress PAL: 0x0028bf70
     */
    void PostIn(Command *pCommand, int nDelayTicks, const CommandId &id, bool bRecordable);

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
     * Drop every queued command.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00282768
     * @ghidraAddress PAL: 0x0028c020
     */
    void Clear();

    /**
     * Withdraw every queued run of a command.
     *
     * @param pCommand The command.
     * @ghidraAddress NTSC-U/C: 0x002827c0
     * @ghidraAddress PAL: 0x0028c078
     */
    void Cancel(Command *pCommand);

    /**
     * Advance the clock to the current time and run the commands it passed.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002827f0
     * @ghidraAddress PAL: 0x0028c0a8
     */
    void Poll();

    std::multiset<CommandInfo> *mCommands; /*!< The queued runs. */
    Recorder *mRecorder;                   /*!< The recorder, or null. */
    Playbacker *mPlaybacker;               /*!< The playback, or null. */
    const float *mTickDuration;            /*!< The duration of one tick Reset() was given. */
    float mTime;                           /*!< The song clock in milliseconds. */
    int mTick;                             /*!< The song clock in ticks. */
    float mFrameTime;      /*!< The clock time the last pump ran the commands up to. */
    int mFrameTick;        /*!< The clock tick the last pump ran the commands up to. */
    float mPrevFrameTime;  /*!< The clock time the pump before the last ran up to. */
    int mPrevFrameTick;    /*!< The clock tick the pump before the last ran up to. */
    SchedulerClock mClock; /*!< The clock. */

private:
    /**
     * Queue a run of a command.
     *
     * The name is inferred.
     *
     * @param pCommand The command.
     * @param fTime The time, in milliseconds.
     * @param nTick The tick.
     * @param id The tag stored with the run.
     * @param bRecordable The command is recorded for replay.
     * @ghidraAddress NTSC-U/C: 0x00282520
     * @ghidraAddress PAL: 0x0028bdd8
     */
    void Insert(Command *pCommand, float fTime, int nTick, const CommandId &id, bool bRecordable);

    /**
     * Withdraw the runs a test selects.
     *
     * The name is inferred.
     *
     * @param pred The test.
     * @ghidraAddress NTSC-U/C: 0x00282960
     * @ghidraAddress PAL: 0x0028c230
     */
    void RemoveIf(const CancelPred &pred);
};

/**
 * Write a command to a recording: `1`, its serial identifier, and what Command::Save() writes, or
 * `0` for no command.
 *
 * @param stream The stream.
 * @param pCommand The command, or null.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00282d48
 * @ghidraAddress PAL: 0x0028c620
 */
BinStream &operator<<(BinStream &stream, Command *pCommand);

/**
 * Read a command operator<<() wrote, building it through Factory<Command>.
 *
 * A first byte other than `0` or `1` reports a warning and leaves pCommand unchanged.
 *
 * @param stream The stream.
 * @param pCommand Receives the command, or null.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00282e20
 * @ghidraAddress PAL: 0x0028c6e0
 */
BinStream &operator>>(BinStream &stream, Command *&pCommand);

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
