#pragma once

#include <map>
#include <vector>

#include "gs/muse.h"
#include "met/mixtrack.h"
#include "os/command.h"
#include "os/ptr.h"
#include "os/string.h"

/**
 * The front-end music: the tracks of the current menu song and the mixes that switch them on and
 * off on the bar.
 *
 * The RTTI includes the class name in the command MemFunCommand that calls StopTracks(). The class
 * is not polymorphic. The object is 0x34 bytes. Metagame builds one for each visit to the front
 * end, and MetaMusicSong adds the tracks. A mix is a set of track names, and switching mixes fades
 * out the tracks of the old mix and fades in the tracks of the new one, in step with the loop.
 */
class Mix {
public:
    /** The number of mixes of a menu song. */
    static constexpr int kNumMixes = 10;

    /** The value of mMix before any mix plays. */
    static constexpr int kNoMix = -1;

    /**
     * Build the music with no tracks and no mix.
     *
     * The `music_bars` and `music_fade_ticks` entries of the metagame configuration give the
     * length of the loop and of a fade.
     *
     * @ghidraAddress NTSC-U/C: 0x00169380
     * @ghidraAddress PAL: 0x0016c508
     */
    Mix();

    /**
     * Stop the tracks and release them.
     *
     * @ghidraAddress NTSC-U/C: 0x00169698
     * @ghidraAddress PAL: 0x0016c820
     */
    ~Mix();

    /**
     * Add a track under a name.
     *
     * @param pMuse The music of the track.
     * @param nChannel The MIDI channel of the music.
     * @param name The name the mixes use.
     * @ghidraAddress NTSC-U/C: 0x00169868
     * @ghidraAddress PAL: 0x0016c9f0
     */
    void AddTrack(Muse *pMuse, unsigned char nChannel, const String &name);

    /**
     * Set the names of the tracks of a mix.
     *
     * @param nMix The mix.
     * @param names The names.
     * @ghidraAddress NTSC-U/C: 0x00169998
     * @ghidraAddress PAL: 0x0016cb20
     */
    void SetMixTracks(int nMix, const std::vector<String> &names);

    /**
     * Switch to a mix with a fade of `music_fade_ticks`.
     *
     * @param nMix The mix, an index of the `mix` entry of the menu song.
     * @ghidraAddress NTSC-U/C: 0x001699c0
     * @ghidraAddress PAL: 0x0016cb48
     */
    void ChangeMix(int nMix);

    /**
     * Switch to a mix unless it already plays, fading out the tracks of the current mix.
     *
     * @param nMix The mix, an index of the `mix` entry of the menu song.
     * @param nFadeTicks The ticks the fades last.
     * @ghidraAddress NTSC-U/C: 0x001699e0
     * @ghidraAddress PAL: 0x0016cb68
     */
    void SwitchMix(int nMix, int nFadeTicks);

    /**
     * Start the sound effect of the tube the camera flies through.
     *
     * @ghidraAddress NTSC-U/C: 0x00169a48
     * @ghidraAddress PAL: 0x0016cbd0
     */
    void EnterTube();

    /**
     * Stop the sound effect of the tube.
     *
     * @param bSpeedingUp Whether the front end runs faster than real time. The body does not read
     * it.
     * @ghidraAddress NTSC-U/C: 0x00169af0
     * @ghidraAddress PAL: 0x0016cc78
     */
    void ExitTube(bool bSpeedingUp);

    /**
     * Start the music at the last bar of the front-end clock with a mix, at full volume.
     *
     * @param nMix The mix, an index of the `mix` entry of the menu song.
     * @ghidraAddress NTSC-U/C: 0x00169b98
     * @ghidraAddress PAL: 0x0016cd20
     */
    void Start(int nMix);

    /**
     * Withdraw the scheduled stop and stop every track at once.
     *
     * @ghidraAddress NTSC-U/C: 0x00169c48
     * @ghidraAddress PAL: 0x0016cdd0
     */
    void StopTracks();

    /**
     * Fade out the tracks of a mix.
     *
     * @param nMix The mix.
     * @param nFadeTicks The ticks the fades last.
     * @ghidraAddress NTSC-U/C: 0x00169ce0
     * @ghidraAddress PAL: 0x0016ce68
     */
    void FadeOutMix(int nMix, int nFadeTicks);

    /**
     * Fade in the tracks of a mix at the current position of the loop.
     *
     * @param nMix The mix.
     * @param nFadeTicks The ticks the fades last.
     * @ghidraAddress NTSC-U/C: 0x00169e68
     * @ghidraAddress PAL: 0x0016cff0
     */
    void FadeInMix(int nMix, int nFadeTicks);

    /**
     * Fade every track out over one bar and stop the music after it.
     *
     * @ghidraAddress NTSC-U/C: 0x0016a028
     * @ghidraAddress PAL: 0x0016d1b0
     */
    void Stop();

    int mLoopTicks;                           /*!< The ticks of the loop, `music_bars` bars. */
    int mFadeTicks;                           /*!< `music_fade_ticks`, the fade of ChangeMix(). */
    int mStartTick;                           /*!< The tick of the front-end clock Start() used. */
    std::vector<std::vector<String> > mMixes; /*!< The track names of each mix. */
    std::map<String, MixTrack *> mTracks;     /*!< The tracks by name. */
    int mMix;                                 /*!< The mix that plays, or kNoMix. */
    Ptr<Command> mStopCmd;                    /*!< The command that calls StopTracks(). */
};
