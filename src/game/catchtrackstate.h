#pragma once

#include "game/gemcursor.h"

/**
 * Per-bar state of one catch track during a song.
 *
 * The RTTI includes the nested CatchTrackState::BarState. Only the members its callers here use
 * are declared.
 */
class CatchTrackState {
public:
    /**
     * Report whether the phrase of a bar is captured.
     *
     * @param nBar The bar the song plays.
     * @return Whether the phrase is captured.
     * @ghidraAddress NTSC-U/C: 0x0014e230
     * @ghidraAddress PAL: 0x0014fbd0
     */
    bool IsCaptured(int nBar);

    /**
     * Report whether the gems of a bar can be caught.
     *
     * @param nBar The bar the song plays.
     * @return Whether the gems can be caught.
     * @ghidraAddress NTSC-U/C: 0x0014e2e8
     * @ghidraAddress PAL: 0x0014fc88
     */
    bool IsEnabled(int nBar);

    /**
     * Construct a cursor at the first gem of a lane at or after a tick.
     *
     * @param nLane The lane.
     * @param nTick The tick the song plays at.
     * @return The cursor.
     * @ghidraAddress NTSC-U/C: 0x0014e458
     * @ghidraAddress PAL: 0x0014fdf8
     */
    GemCursor GetCursor(int nLane, int nTick);
};
