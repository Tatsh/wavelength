#pragma once

#include "game/track.h"

/**
 * Track a player improvises on while the freestyle power-up lasts.
 *
 * The RTTI records the class as deriving from Track. AxeTrack and ScratchTrack derive from it.
 * Only the members GameLogic uses are declared.
 */
class FreestyleTrack : public Track {
public:
    using Track::HandleInput;

    /**
     * Ignore the input.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0034b6d8
     * @ghidraAddress PAL: 0x003b8b08
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const BtnEvent<8> &event) override {
    }

    /**
     * Act on a stick event of a player on this track.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0014f3b8
     * @ghidraAddress PAL: 0x00150d18
     */
    void HandleInput(Player *pPlayer, const StickEvent<6> &event) override;
};
