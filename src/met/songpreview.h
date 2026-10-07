#pragma once

#include "os/string.h"
#include "synth/streamplayer.h"

/**
 * The streamed clip of a song that plays while the song screen shows the song.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The clip is a state machine
 * whose state is stored in globals, and every member is static. While the menu music plays the
 * state is kStateMenu. Stop() switches the sound output to the effects of the clip, and a job the
 * output runs afterwards sets kStateReady. Load() chooses a clip, and Poll() plays it, fading its
 * volume in and out. End() fades the clip out. Once the clip has stopped, Poll() restores the
 * effects of the menu music, and a job the output runs afterwards sets kStateMenu again.
 */
class SongPreview {
public:
    /** The states of the clip, the values of sState. */
    enum State {
        kStateStarting = 0,  /*!< Stop() waits for the output to take the effects of the clip. */
        kStateReady = 1,     /*!< A clip may be chosen and played. */
        kStateEnding = 2,    /*!< End() waits for the clip to stop. */
        kStateRestoring = 3, /*!< Poll() waits for the output to take the effects of the menu. */
        kStateMenu = 4,      /*!< The menu music plays, and no clip does. */
    };

    /**
     * Mute the menu music and prepare the sound output for clips.
     *
     * In kStateMenu the output level drops to 0 and the `song_preview_effects` entry of the
     * metagame configuration applies. The state becomes kStateStarting until the output runs a
     * job that sets kStateReady.
     *
     * @param fFadeStep The volume a clip gains or loses on each Poll().
     * @ghidraAddress NTSC-U/C: 0x00196570
     * @ghidraAddress PAL: 0x0019da00
     */
    static void Stop(float fFadeStep);

    /**
     * Fade out the clip, forget the clip to play, and enter kStateEnding.
     *
     * @param bRestoreMusic Whether the effects of the menu music return once the clip has stopped.
     * @ghidraAddress NTSC-U/C: 0x00196668
     * @ghidraAddress PAL: 0x0019daf8
     */
    static void End(bool bRestoreMusic);

    /**
     * Report whether the clip is in kStateReady.
     *
     * @return Whether a clip may play.
     * @ghidraAddress NTSC-U/C: 0x001966a0
     * @ghidraAddress PAL: 0x0019db30
     */
    static bool IsWaiting();

    /**
     * Choose the clip of a song and fade out the clip that plays.
     *
     * The two encrypted songs name their stream file directly. Any other song plays
     * `Songs/<song>/<song>_metaclip.str` when bMetaClip is set, and `audio/<song>.str`, with the
     * song name cut to eight characters, otherwise.
     *
     * @param pszSong The song, or sLevelEncrypt or sBonusEncrypt.
     * @param bMetaClip Whether the song's own clip plays.
     * @ghidraAddress NTSC-U/C: 0x001966b8
     */
    static void Load(const char *pszSong, bool bMetaClip);

    /**
     * Start fading the clip out.
     *
     * @ghidraAddress NTSC-U/C: 0x00196798
     * @ghidraAddress PAL: 0x0019dcb8
     */
    static void FadeOut();

    /**
     * Report whether no clip plays and none is chosen.
     *
     * @return Whether the clip is idle.
     * @ghidraAddress NTSC-U/C: 0x001967b0
     * @ghidraAddress PAL: 0x0019dcd0
     */
    static bool IsIdle();

    /**
     * Report whether the clip is in kStateMenu, the state before Stop().
     *
     * @return Whether the menu music plays.
     * @ghidraAddress NTSC-U/C: 0x001967d8
     * @ghidraAddress PAL: 0x0019dcf8
     */
    static bool IsPlaying();

    /**
     * Advance the clip, fading its volume and starting or stopping its stream as requested.
     *
     * @ghidraAddress NTSC-U/C: 0x001967f0
     * @ghidraAddress PAL: 0x0019dd10
     */
    static void Poll();

    /**
     * The stream that stands in for the clip of a locked song.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8a8
     */
    static const char *sLevelEncrypt;

    /**
     * The stream that stands in for the clip of a locked bonus song.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8ac
     */
    static const char *sBonusEncrypt;

private:
    /**
     * Enter kStateMenu. The sound output calls it once the effects of the menu music apply.
     *
     * @param nArg The argument of the job. The routine does not read it.
     * @ghidraAddress NTSC-U/C: 0x00196550
     */
    static void OnMenuRestored(int nArg);

    /**
     * Enter kStateReady. The sound output calls it once the effects of the clips apply.
     *
     * @param nArg The argument of the job. The routine does not read it.
     * @ghidraAddress NTSC-U/C: 0x00196560
     */
    static void OnClipReady(int nArg);

    /**
     * The state, one of State.
     *
     * @ghidraAddress NTSC-U/C: 0x003af890
     */
    static int sState;

    /**
     * The stream of the clip that plays, or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003af894
     */
    static StreamPlayer *sPlayer;

    /**
     * The output level of the clip.
     *
     * @ghidraAddress NTSC-U/C: 0x003af898
     */
    static float sVolume;

    /**
     * The volume the clip gains on each Poll(), negative while it fades out.
     *
     * @ghidraAddress NTSC-U/C: 0x003af89c
     */
    static float sFadeRate;

    /**
     * The size of a fade step, the value Stop() was given.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8a0
     */
    static float sFadeStep;

    /**
     * Whether the effects of the menu music return once the clip has stopped.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8a4
     */
    static int sRestoreMusic;

    /**
     * The stream file of the clip to play next, or empty.
     *
     * @ghidraAddress NTSC-U/C: 0x004368a0
     */
    static String sClip;
};
