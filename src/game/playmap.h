#pragma once

#include <vector>

/**
 * Map from the bars a song plays to the bars of the song as written, with the loops that repeat
 * stretches of bars.
 *
 * The class is not polymorphic. The RTTI includes the nested PlayMap::Loop.
 */
class PlayMap {
public:
    /** Stretch of the song as written that plays from a bar on. */
    class Loop {
    public:
        /**
         * Order loops by the bar they play from.
         *
         * Inline. The searches of mLoops expand it.
         *
         * @param other The other loop.
         * @return Whether this loop plays first.
         */
        bool operator<(const Loop &other) const {
            return mPlayBar < other.mPlayBar;
        }

        int mStartBar; /*!< The first bar of the stretch as written, or -1 for silence. */
        int mNumBars;  /*!< The length of the stretch in bars. */
        int mPlayBar;  /*!< The bar the song plays the stretch from. */
    };

    /**
     * Construct a map that plays the song as written, from bar 0 without end.
     *
     * @param nTicksPerBar The song ticks in one bar.
     * @param nEndBar The bar the song ends at while no loop was added.
     * @ghidraAddress NTSC-U/C: 0x0012e3a0
     * @ghidraAddress PAL: 0x0012fb78
     */
    PlayMap(int nTicksPerBar, int nEndBar);

    /**
     * Play a range of bars of the song as written from a bar on, repeating without end.
     *
     * The repetition of the current last loop under way at that bar plays only up to the bar.
     *
     * @param nStartBar The first bar of the range as written.
     * @param nNumBars The length of the range in bars.
     * @param nPlayBar The bar the song plays the range from.
     * @ghidraAddress NTSC-U/C: 0x0012e5e0
     * @ghidraAddress PAL: 0x0012fdb8
     */
    void AddLoop(int nStartBar, int nNumBars, int nPlayBar);

    /**
     * Map a bar the song plays to the bar of the song as written.
     *
     * @param nBar The bar the song plays.
     * @return The bar as written, which is negative inside a stretch of silence.
     * @ghidraAddress NTSC-U/C: 0x0012ea38
     * @ghidraAddress PAL: 0x00130210
     */
    int MapBar(int nBar) const;

    /**
     * Map a tick the song plays at to the tick of the song as written.
     *
     * @param nTick The tick the song plays at.
     * @return The tick of the song as written.
     * @ghidraAddress NTSC-U/C: 0x0012eab8
     * @ghidraAddress PAL: 0x00130290
     */
    int MapTick(int nTick) const;

    /**
     * Report the stretch of the song as written that plays at a tick, and the stretch after it.
     *
     * A loop that repeats without end stores 0x20000001 in pChangeTick, stores 1 in pIsLast, and
     * does not write the other outputs.
     *
     * @param nTick The tick the song plays at.
     * @param pChangeTick Receives the tick the song plays at where the next stretch begins.
     * @param pEnd Receives the tick of the song as written where the current stretch ends.
     * @param pNextStart Receives the tick of the song as written where the next stretch begins.
     * @param pNextLength Receives the length of the next stretch in ticks.
     * @param pIsLast Receives whether no loop follows the current one.
     * @ghidraAddress NTSC-U/C: 0x0012eb20
     * @ghidraAddress PAL: 0x001302f8
     */
    void GetSegment(
        int nTick, int *pChangeTick, int *pEnd, int *pNextStart, int *pNextLength, int *pIsLast);

    /**
     * Report the bar the song ends at.
     *
     * @return The last bar, or 0x20000001 while a loop repeats.
     * @ghidraAddress NTSC-U/C: 0x0012ec98
     * @ghidraAddress PAL: 0x00130470
     */
    int GetEndBar() const;

    /**
     * Remove every loop, and play the song as written again.
     *
     * mLooping remains set.
     *
     * @ghidraAddress NTSC-U/C: 0x0012ecb8
     * @ghidraAddress PAL: 0x00130490
     */
    void ClearLoops();

    int mTicksPerBar;         /*!< The song ticks in one bar. */
    std::vector<Loop> mLoops; /*!< The loops, by the bar they play from. */
    int mEndBar;              /*!< The bar the song ends at while no loop was added. */
    bool mLooping;            /*!< Whether a loop was added. */
};
