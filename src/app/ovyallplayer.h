#pragma once

#include "app/ovyavatar.h"
#include "app/ovyduelscore.h"
#include "app/ovyscore.h"
#include "rnd/view.h"

/**
 * Parts of the head-up display every player has: the score and the avatar.
 *
 * The RTTI records the class, which is not polymorphic, as the pointee of the overlay's pointer
 * vector. The object is 0x10 bytes.
 */
class OvyAllPlayer {
public:
    /**
     * Construct the parts of a player.
     *
     * A game has a score and a duel the letters. A solo game, a duel, and a local player of an
     * online remix have an avatar.
     *
     * @param nPlayer The player.
     * @param nIndex The number of the player's parts, from 0.
     * @param pHudView The view of the head-up display.
     * @ghidraAddress NTSC-U/C: 0x001c28b0
     * @ghidraAddress PAL: 0x001cb650
     */
    OvyAllPlayer(int nPlayer, int nIndex, Rnd::View *pHudView);

    /**
     * Destroy the parts.
     *
     * @ghidraAddress NTSC-U/C: 0x001c29e8
     * @ghidraAddress PAL: 0x001cb788
     */
    ~OvyAllPlayer();

    /**
     * Advance the parts.
     *
     * @param fSongDelta The song time since the last poll.
     * @param fRealDelta The time since the last poll.
     * @param fTickDelta The ticks since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001c2a88
     * @ghidraAddress PAL: 0x001cb828
     */
    void Poll(float fSongDelta, float fRealDelta, float fTickDelta);

    /**
     * Draw the score.
     *
     * @ghidraAddress NTSC-U/C: 0x001c2af8
     * @ghidraAddress PAL: 0x001cb898
     */
    void DrawHud();

    /**
     * Draw the avatar.
     *
     * @ghidraAddress NTSC-U/C: 0x001c2b20
     * @ghidraAddress PAL: 0x001cb8c0
     */
    void DrawAvatar();

    /**
     * Empty the score and the letters, and reset the avatar.
     *
     * @ghidraAddress NTSC-U/C: 0x001c2b48
     * @ghidraAddress PAL: 0x001cb8e8
     */
    void Reset();

    int mPlayer;              /*!< The player. */
    OvyScore *mScore;         /*!< The score, or null. */
    OvyDuelScore *mDuelScore; /*!< The letters of a duel, or null. */
    OvyAvatar *mAvatar;       /*!< The avatar, or null. */
};
