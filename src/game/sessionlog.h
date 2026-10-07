#pragma once

/**
 * Log of the events of a song, written to a file once the song ends.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Only the members its
 * callers here use are declared.
 */
class SessionLog {
public:
    /**
     * Start a log.
     *
     * The body is empty in the shipped build.
     *
     * @param nTracks The number of tracks of the song.
     * @param nBars The length of the song in bars.
     * @ghidraAddress NTSC-U/C: 0x0013c178
     * @ghidraAddress PAL: 0x0013da48
     */
    void Begin(int nTracks, int nBars);

    /**
     * Write the log and clear it.
     *
     * @ghidraAddress NTSC-U/C: 0x0013c180
     * @ghidraAddress PAL: 0x0013da50
     */
    void End();

    /**
     * Log the end of a section.
     *
     * @param nTick The tick the section ends at.
     * @ghidraAddress NTSC-U/C: 0x0013ce00
     * @ghidraAddress PAL: 0x0013e6d0
     */
    void LogSectionEnd(int nTick);

    /**
     * Log a deployed power-up.
     *
     * @param nPlayer The player.
     * @param nTick The tick.
     * @param nPowerup The kind of power-up.
     * @ghidraAddress NTSC-U/C: 0x0013d670
     * @ghidraAddress PAL: 0x0013ef40
     */
    void LogPowerup(int nPlayer, int nTick, int nPowerup);

    /**
     * Log a caught gem.
     *
     * @param nTrack The track.
     * @param nGemTick The tick of the gem.
     * @param nTick The tick of the press.
     * @param fErrorMs The time from the press to the gem in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0013c338
     * @ghidraAddress PAL: 0x0013dc08
     */
    void LogGemHit(int nTrack, int nGemTick, int nTick, float fErrorMs);

    /**
     * Log a press that missed the nearest gem.
     *
     * @param nTrack The track.
     * @param nGemTick The tick of the nearest gem.
     * @param nTick The tick of the press.
     * @param fErrorMs The time from the press to the gem in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0013c508
     * @ghidraAddress PAL: 0x0013ddd8
     */
    void LogGemMiss(int nTrack, int nGemTick, int nTick, float fErrorMs);

    /**
     * Log a press with no gem near it.
     *
     * @param nTrack The track.
     * @param nTick The tick of the press.
     * @ghidraAddress NTSC-U/C: 0x0013c6d8
     * @ghidraAddress PAL: 0x0013dfa8
     */
    void LogMiss(int nTrack, int nTick);

    /**
     * Log a gem that passed without a press.
     *
     * @param nTrack The track.
     * @param nGemTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x0013c890
     * @ghidraAddress PAL: 0x0013e160
     */
    void LogGemPass(int nTrack, int nGemTick);
};

/**
 * The log of the song.
 *
 * @ghidraAddress NTSC-U/C: 0x004361d8
 */
extern SessionLog *TheSessionLog;
