#pragma once

#include "met/freqpanel.h"

/**
 * Panel that shows the statistics of one player of the last game of several players, one row of
 * the ranking.
 *
 * The RTTI records the class as deriving from FreqPanel. Its vtable is at `0x003cc540`. Only the
 * members the end-of-game screens reach are declared.
 */
class MultiGameStatsPanel : public FreqPanel {
public:
    /**
     * Fill the row with a player.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001708a0
     * @ghidraAddress PAL: 0x00173ba8
     */
    virtual void SetPlayer(int nPlayer);

    int mTied; /*!< Whether the player shares the rank with a neighbouring row. +0x108 */
};
