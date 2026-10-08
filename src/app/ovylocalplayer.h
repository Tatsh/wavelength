#pragma once

#include "app/hudpoints.h"
#include "app/ovyremixpanel.h"

/**
 * Parts of the head-up display of a player on this console: the pending points and the remix
 * menu.
 *
 * The RTTI records the class, which is not polymorphic, as the pointee of the overlay's pointer
 * vector. The object is 0x18 bytes.
 */
class OvyLocalPlayer {
public:
    /**
     * Construct the parts of a player.
     *
     * A local player of a remix has a remix menu in the letterboxed view, and a player of a game
     * has the pending points.
     *
     * @param nPlayer The player.
     * @param nPosition The place of the player's parts, from 0.
     * @ghidraAddress NTSC-U/C: 0x001c2638
     * @ghidraAddress PAL: 0x001cb3d8
     */
    OvyLocalPlayer(int nPlayer, int nPosition);

    /**
     * Destroy the parts.
     *
     * @ghidraAddress NTSC-U/C: 0x001c2778
     * @ghidraAddress PAL: 0x001cb518
     */
    ~OvyLocalPlayer();

    /**
     * Advance the parts.
     *
     * @param fUnused Not read.
     * @param fUnused2 Not read.
     * @param fSongDelta The song time since the last poll.
     * @param fRealDelta The time since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001c27e8
     * @ghidraAddress PAL: 0x001cb588
     */
    void Poll(float fUnused, float fUnused2, float fSongDelta, float fRealDelta);

    /**
     * Draw the remix menu.
     *
     * @ghidraAddress NTSC-U/C: 0x001c2848
     * @ghidraAddress PAL: 0x001cb5e8
     */
    void DrawHud();

    /**
     * Draw nothing. The local parts have no avatar.
     *
     * @ghidraAddress NTSC-U/C: 0x001c2880
     * @ghidraAddress PAL: 0x001cb620
     */
    void DrawAvatar();

    /**
     * Empty the pending points.
     *
     * @ghidraAddress NTSC-U/C: 0x001c2888
     * @ghidraAddress PAL: 0x001cb628
     */
    void Reset();

    int mPlayer;           /*!< The player. */
    int mPosition;         /*!< The place of the parts. */
    HudPoints *mPoints;    /*!< The pending points, or null. */
    OvyRemixPanel *mRemix; /*!< The remix menu, or null. */
    int mReserved10[2];    // +0x10, cleared by the constructor and not yet identified.
};
