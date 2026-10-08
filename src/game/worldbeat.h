#pragma once

#include <vector>

#include "game/worldtrack.h"
#include "os/command.h"
#include "os/ptr.h"
#include "os/scheduler.h"

/**
 * Player of the events of the track named "WORLD", which runs each event at its tick.
 *
 * The RTTI includes the nested WorldBeat::WorldBeatCmd. Only the members GameLogic uses are
 * declared.
 */
class WorldBeat {
public:
    /**
     * Command that reports the event of one letter to TheTriggerMgr and schedules the next event.
     *
     * The RTTI includes the nested name and records Command as the base, and the vtable is at
     * `0x003d67c0`. The constructor is expanded in the constructor of WorldBeat, and the destructor
     * at `0x003a6458` (PAL `0x00415138`) is compiler-generated. After the last event of a loop the
     * events repeat from the first event at or after tick 0, one song length later.
     */
    class WorldBeatCmd : public Command {
    public:
        /**
         * Construct the command of a letter.
         *
         * @param pScheduler The scheduler the events run on.
         * @param nLengthTicks The length of the song in ticks.
         * @param pEvents The ticks of the events.
         * @param cLetter The letter.
         */
        WorldBeatCmd(Scheduler *pScheduler,
                     int nLengthTicks,
                     std::vector<int> *pEvents,
                     char cLetter);

        /**
         * Report the event and schedule the next one.
         *
         * @ghidraAddress NTSC-U/C: 0x003a64d0
         * @ghidraAddress PAL: 0x004151b0
         */
        void Execute() override;

    private:
        Scheduler *mScheduler;     // The scheduler the events run on.
        int mLengthTicks;          // The length of the song in ticks.
        std::vector<int> *mEvents; // The ticks of the events.
        char mLetter;              // The letter.
        int mFirst;                // The first event at or after tick 0, or -1 for none.
        int mIndex;                // The event that ran last.
        int mLoop;                 // The number of times the events have repeated.
    };

    /**
     * Construct the player of a track.
     *
     * @param pScheduler The scheduler the events run on.
     * @param pTrack The events.
     * @param nLengthTicks The length of the song in ticks.
     * @ghidraAddress NTSC-U/C: 0x00280c10
     * @ghidraAddress PAL: 0x0028a510
     */
    WorldBeat(Scheduler *pScheduler, WorldTrack *pTrack, int nLengthTicks);

    /**
     * Release the player.
     *
     * @ghidraAddress NTSC-U/C: 0x00280e48
     * @ghidraAddress PAL: 0x0028a748
     */
    ~WorldBeat();

    /**
     * Schedule the first event.
     *
     * @ghidraAddress NTSC-U/C: 0x00280ef8
     * @ghidraAddress PAL: 0x0028a7f8
     */
    void Start();

    /**
     * Withdraw the scheduled event.
     *
     * @ghidraAddress NTSC-U/C: 0x00280fa0
     * @ghidraAddress PAL: 0x0028a8a0
     */
    void Stop();

    Scheduler *mScheduler;               /*!< The scheduler the events run on. */
    WorldTrack *mTrack;                  /*!< The events. */
    std::vector<Ptr<Command>> mCommands; /*!< The command of each letter. */
};
