#pragma once

#include <vector>

/**
 * Volume control of the song's tracks.
 *
 * The RTTI includes the class name. The one instance is the function-local static of its
 * singleton accessor, and TheMixer addresses it. Only the members GameLogic uses are declared.
 */
class Mixer {
public:
    /**
     * Set the mixer up for a song and read the "mixer" section of the configuration.
     *
     * @param nTracks The number of tracks in the song.
     * @param nInstrument The track type of the freestyle track.
     * @ghidraAddress NTSC-U/C: 0x00123ad0
     * @ghidraAddress PAL: 0x00125250
     */
    void Init(int nTracks, int nInstrument);

    /**
     * Replace a channel's volume list.
     *
     * @param nChannel The channel.
     * @param volumes The volumes.
     * @ghidraAddress NTSC-U/C: 0x00123e78
     * @ghidraAddress PAL: 0x001255f8
     */
    void SetVolumes(int nChannel, const std::vector<unsigned char> &volumes);

    /**
     * Replace the volume list the other tracks use while the instrument track is occupied.
     *
     * @param volumes The volumes.
     * @ghidraAddress NTSC-U/C: 0x00123ea0
     * @ghidraAddress PAL: 0x00125620
     */
    void SetInstrumentVolumes(const std::vector<unsigned char> &volumes);

    /**
     * Hold a track until the tick before a song tick, and refresh every other channel.
     *
     * Nothing happens once the song has passed the tick.
     *
     * @param nTrack The track's index.
     * @param nUntilTick The tick the hold ends at.
     * @ghidraAddress NTSC-U/C: 0x00124070
     * @ghidraAddress PAL: 0x001257f0
     */
    void HoldTrack(int nTrack, int nUntilTick);

    /**
     * Start following the players and register the `track_solo` cheat.
     *
     * @ghidraAddress NTSC-U/C: 0x001242d0
     * @ghidraAddress PAL: 0x00125a50
     */
    void Activate();

    /**
     * Stop following the players, unregister the cheat, and return every track to full volume.
     *
     * @ghidraAddress NTSC-U/C: 0x00124328
     * @ghidraAddress PAL: 0x00125aa8
     */
    void Deactivate();

    /**
     * Fade every track but the effects to silence.
     *
     * @param nTicks The ticks the fade lasts.
     * @ghidraAddress NTSC-U/C: 0x001243b0
     * @ghidraAddress PAL: 0x00125b30
     */
    void FadeOut(int nTicks);
};

/**
 * The track mixer.
 *
 * @ghidraAddress NTSC-U/C: 0x00436144
 */
extern Mixer *TheMixer;
