#pragma once

#include "game/player.h"

/**
 * Player on another console of an online session.
 *
 * The RTTI records the class as deriving from Player. Only the members GameLogic uses are
 * declared.
 */
class RemotePlayer : public Player {
public:
    /**
     * Construct a player that the network updates.
     *
     * @param nIndex The player's index.
     * @param nTicksPerBar The song ticks in one bar.
     * @ghidraAddress NTSC-U/C: 0x00137340
     * @ghidraAddress PAL: 0x00138ba0
     */
    RemotePlayer(int nIndex, int nTicksPerBar);
};
