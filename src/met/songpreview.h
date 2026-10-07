#pragma once

/**
 * The streamed clip of a song that plays while the song screen shows the song.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The clip is a state machine
 * whose state is stored in globals. Every member is static. Only the members the metagame and the
 * song and arena screens use are declared, and the routines are not reconstructed.
 */
class SongPreview {
public:
    /**
     * Fade out a playing clip over a time and release it.
     *
     * @param fFadeSeconds The length of the fade.
     * @ghidraAddress NTSC-U/C: 0x00196570
     * @ghidraAddress PAL: 0x0019da00
     */
    static void Stop(float fFadeSeconds);

    /**
     * Fade out the clip, forget the clip to play, and wait for the fade to end.
     *
     * @param bRestoreMusic Whether the menu music returns once the clip ended.
     * @ghidraAddress NTSC-U/C: 0x00196668
     * @ghidraAddress PAL: 0x0019daf8
     */
    static void End(bool bRestoreMusic);

    /**
     * Report whether the clip is in state 1, the state of waiting for a clip to play.
     *
     * @return Whether the clip waits.
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
     * Report whether the clip is in state 4, the state Stop() fades out.
     *
     * @return Whether the clip plays.
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
};
