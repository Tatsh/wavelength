#pragma once

#include "game/inputevents.h"

// Player and Track refer to each other.
class Player;

/**
 * One instrument track of the song that a player can play.
 *
 * The RTTI includes the class name, and the derived classes include CatchTrack, FreestyleTrack,
 * and RemixTrack. The controller input a player receives is handed to the player's track through
 * the five HandleInput() overloads, which this class declares pure. Only the members its callers
 * here use are declared.
 */
class Track {
public:
    /** Release the track. */
    virtual ~Track();

    /** Start the track with the song. */
    virtual void Start() = 0;

    /** Stop the track with the song. */
    virtual void Stop() = 0;

    /**
     * Give the track to the player it plays for.
     *
     * @param pPlayer The player, or null when no player is on the track.
     */
    virtual void SetPlayer(Player *pPlayer) = 0;

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

    int mIndex;       /*!< The track's index in the song. +0x00 */
    Player *mPlayer;  /*!< The player on the track, or null. +0x04 */
};
