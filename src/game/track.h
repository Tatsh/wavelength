#pragma once

#include "game/btnevent.h"
#include "game/playnoteevent.h"
#include "game/stickevent.h"

// Player and Track refer to each other.
class Player;

/**
 * One instrument track of the song that a player can play.
 *
 * The RTTI includes the class name, and the class has no base. The derived classes include
 * CatchTrack, FreestyleTrack, and RemixTrack. Two data words precede the vptr. The controller input
 * a player receives is handed to the player's track through the five HandleInput() overloads, which
 * this class declares pure.
 */
class Track {
public:
    /**
     * Construct a track with no player.
     *
     * @param nIndex The track's index in the song.
     * @ghidraAddress NTSC-U/C: 0x0013e6c0
     * @ghidraAddress PAL: 0x0013ff90
     */
    explicit Track(int nIndex);

    /**
     * Release the track.
     *
     * @ghidraAddress NTSC-U/C: 0x00343758
     * @ghidraAddress PAL: 0x003b0c90
     */
    virtual ~Track() {
    }

    /** Start the track with the song. */
    virtual void Start() = 0;

    /** Stop the track with the song. */
    virtual void Stop() = 0;

    /**
     * Assign the player, and record the assignment in the session statistics.
     *
     * @param pPlayer The player, or null.
     * @ghidraAddress NTSC-U/C: 0x0013e6e0
     * @ghidraAddress PAL: 0x0013ffb0
     */
    virtual void SetPlayer(Player *pPlayer);

    /**
     * Act on a note a player played on this track.
     *
     * @param pPlayer The player.
     * @param event The event.
     */
    virtual void HandleInput(Player *pPlayer, const PlayNoteEvent &event) = 0;

    /**
     * Act on a button event of a player on this track.
     *
     * @param pPlayer The player.
     * @param event The event.
     */
    virtual void HandleInput(Player *pPlayer, const BtnEvent<8> &event) = 0;

    /**
     * Act on a stick event of a player on this track.
     *
     * @param pPlayer The player.
     * @param event The event.
     */
    virtual void HandleInput(Player *pPlayer, const StickEvent<2> &event) = 0;

    /**
     * Act on a stick event of a player on this track.
     *
     * @param pPlayer The player.
     * @param event The event.
     */
    virtual void HandleInput(Player *pPlayer, const StickEvent<6> &event) = 0;

    /**
     * Act on a button event of a player on this track.
     *
     * @param pPlayer The player.
     * @param event The event.
     */
    virtual void HandleInput(Player *pPlayer, const BtnEvent<10> &event) = 0;

    int mIndex;      /*!< The track's index in the song. +0x00 */
    Player *mPlayer; /*!< The player on the track, or null. +0x04 */
};
