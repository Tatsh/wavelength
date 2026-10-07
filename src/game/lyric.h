#pragma once

#include <set>

#include "os/command.h"
#include "os/ptr.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * Lyrics of a song loop, each shown when the song reaches its tick.
 *
 * The RTTI includes the class name. The lyrics repeat every loop of mLoopBars bars. Each lyric is
 * handed to TheTriggerMgr, and also shown on the first lyric line of TheGfxManager while the
 * `lyric` cheat shows lyrics. A lyric before the first tick of the loop is shown only on the first
 * pass.
 */
class Lyric {
public:
    /** One lyric and the tick it is shown at. */
    class LyricData {
    public:
        /**
         * Order lyrics by tick.
         *
         * @param other The other lyric.
         * @return Whether this lyric comes first.
         */
        bool operator<(const LyricData &other) const {
            return mTick < other.mTick;
        }

        int mTick;    /*!< The tick within the loop, negative before the loop's first pass. */
        String mText; /*!< The text. */
    };

    /**
     * Construct an empty lyric track and register the `lyric` script command.
     *
     * @param nTicksPerBar The song ticks in one bar.
     * @param nLoopBars The bars in one loop.
     * @ghidraAddress NTSC-U/C: 0x00122a50
     * @ghidraAddress PAL: 0x001241d0
     */
    Lyric(int nTicksPerBar, int nLoopBars);

    /**
     * Stop showing lyrics and unregister the `lyric` script command.
     *
     * @ghidraAddress NTSC-U/C: 0x00122b40
     * @ghidraAddress PAL: 0x001242c0
     */
    ~Lyric();

    /**
     * Add a lyric.
     *
     * A lyric whose tick falls at or past the end of the loop is ignored, and so is a second lyric
     * at a tick already used.
     *
     * @param nTick The tick within the loop.
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x00122be8
     * @ghidraAddress PAL: 0x00124368
     */
    void AddLyric(int nTick, const char *pszText);

    /**
     * Clear the lyric and schedule the first one.
     *
     * @ghidraAddress NTSC-U/C: 0x00122c68
     * @ghidraAddress PAL: 0x001243e8
     */
    void Start();

    /**
     * Withdraw the scheduled lyric.
     *
     * @ghidraAddress NTSC-U/C: 0x00122ce8
     * @ghidraAddress PAL: 0x00124468
     */
    void Stop();

    /**
     * Show the current lyric and schedule the next one.
     *
     * After the last lyric of the loop the first lyric at or past the loop's first tick is
     * scheduled in the next loop.
     *
     * @ghidraAddress NTSC-U/C: 0x00122d20
     * @ghidraAddress PAL: 0x001244a0
     */
    void Advance();

    /**
     * Toggle whether lyrics show on the display, the `lyric` script command.
     *
     * @param pCommand The command.
     * @param pUserData The Lyric that registered the command.
     * @ghidraAddress NTSC-U/C: 0x00122e40
     * @ghidraAddress PAL: 0x001245c0
     */
    static void ToggleShowing(DataArray *pCommand, void *pUserData);

private:
    int mTicksPerBar;                       /*!< The song ticks in one bar. */
    int mLoopBars;                          /*!< The bars in one loop. */
    std::set<LyricData> mLyrics;            /*!< The lyrics in tick order. */
    std::set<LyricData>::iterator mCurrent; /*!< The lyric Advance() shows next. */
    Ptr<Command> mAdvanceCommand;           /*!< The command that calls Advance(). */
    bool mShowing;                          /*!< Whether lyrics show on the display. */
};
