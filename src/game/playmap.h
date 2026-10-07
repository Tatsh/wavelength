#pragma once

/**
 * Map from the bars a song plays to the bars of the song, with the loops that repeat sections.
 *
 * The RTTI includes the nested PlayMap::Loop. Only the members GameLogic uses are declared.
 */
class PlayMap {
public:
    /**
     * Repeat a range of bars.
     *
     * @param nStartBar The first bar of the loop.
     * @param nNumBars The length of the loop in bars.
     * @param nTick The tick the loop is aligned to.
     * @ghidraAddress NTSC-U/C: 0x0012e5e0
     * @ghidraAddress PAL: 0x0012fdb8
     */
    void AddLoop(int nStartBar, int nNumBars, int nTick);

    /**
     * Report the bar the song ends at.
     *
     * @return The last bar, or 0x20000001 while a loop repeats.
     * @ghidraAddress NTSC-U/C: 0x0012ec98
     * @ghidraAddress PAL: 0x00130470
     */
    int GetEndBar() const;

    /**
     * Map a tick the song plays at to the tick of the song as written.
     *
     * @param nTick The tick the song plays at.
     * @return The tick of the song as written.
     * @ghidraAddress NTSC-U/C: 0x0012eab8
     * @ghidraAddress PAL: 0x00130290
     */
    int MapTick(int nTick);

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
     * Remove every loop.
     *
     * @ghidraAddress NTSC-U/C: 0x0012ecb8
     * @ghidraAddress PAL: 0x00130490
     */
    void ClearLoops();
};
