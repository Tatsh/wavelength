#pragma once

#include "game/player.h"

/**
 * Player on this console, controlled through one controller port.
 *
 * The RTTI records the class as deriving from Player. Only the members GameLogic uses are
 * declared.
 */
class LocalPlayer : public Player {
public:
    /**
     * Construct a player for a controller port.
     *
     * @param nIndex The player's index.
     * @param nTicksPerBar The song ticks in one bar.
     * @param nPadNum The controller port.
     * @ghidraAddress NTSC-U/C: 0x001225e0
     * @ghidraAddress PAL: 0x00123d60
     */
    LocalPlayer(int nIndex, int nTicksPerBar, int nPadNum);

    /**
     * Release the broadcast command.
     *
     * @ghidraAddress NTSC-U/C: 0x00122650
     * @ghidraAddress PAL: 0x00123dd0
     */
    ~LocalPlayer() override;
};
