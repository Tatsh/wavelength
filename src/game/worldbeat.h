#pragma once

#include "game/worldtrack.h"
#include "os/commandscheduler.h"

/**
 * Player of the events of the track named "WORLD", which runs each event at its tick.
 *
 * The RTTI includes the nested WorldBeat::WorldBeatCmd. Only the members GameLogic uses are
 * declared.
 */
class WorldBeat {
public:
    /**
     * Construct the player of a track.
     *
     * @param pScheduler The scheduler the events run on.
     * @param pTrack The events.
     * @param nLengthTicks The length of the song in ticks.
     * @ghidraAddress NTSC-U/C: 0x00280c10
     * @ghidraAddress PAL: 0x0028a510
     */
    WorldBeat(CommandScheduler *pScheduler, WorldTrack *pTrack, int nLengthTicks);

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
};
