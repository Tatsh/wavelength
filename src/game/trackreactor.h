#pragma once

#include "game/gemcursor.h"
#include "game/player.h"

/**
 * Interface through which the rules react to the phrases of a player's track.
 *
 * The RTTI includes the class name. DuelTrack derives from it as its second base. Only the members
 * its callers here use are declared, and their names are inferred.
 */
class TrackReactor {
public:
    /** Release the interface. */
    virtual ~TrackReactor() {
    }

    /**
     * Report the song ticks in one bar.
     *
     * The member is pure in this class.
     *
     * @return The ticks.
     */
    virtual int GetTicksPerBar() const = 0;

    /**
     * Report the player who plays the track.
     *
     * The member is pure in this class.
     *
     * @return The player.
     */
    virtual Player *GetPlayer() const = 0;

    /**
     * Report whether the phrase of a bar can still be played.
     *
     * The member is pure in this class.
     *
     * @param nBar The bar.
     * @return Whether the bar's phrase can be played.
     */
    virtual bool IsBarActive(int nBar) = 0;

    /**
     * Act on a gem the player hit.
     *
     * The member is pure in this class.
     *
     * @param nTick The song tick of the hit.
     * @param cursor The gem.
     * @param bRemote Whether the hit came from another console.
     */
    virtual void HitGem(int nTick, GemCursor &cursor, bool bRemote) = 0;

    /**
     * Act on a gem the player missed.
     *
     * The member is pure in this class.
     *
     * @param nTick The song tick of the miss.
     * @param cursor The gem.
     * @param bRemote Whether the miss came from another console.
     */
    virtual void MissGem(int nTick, GemCursor &cursor, bool bRemote) = 0;
};
