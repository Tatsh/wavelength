#pragma once

#include <vector>

#include "netflow/netlaunchpadplayer.h"

/**
 * The rating of one player of an online game, and the results that rate the player afterwards.
 *
 * The RTTI includes the name, which the type information of the standard library vector of the type
 * records. The record is 0x1c bytes. A rating at least 1000000 marks a player who has played the
 * first games, and Rating() takes the bonus off.
 */
class RatingStat {
public:
    /** The games below which a player's rating comes from the average of the opponents. */
    static constexpr int kProvisionalGames = 16;

    /**
     * Copy the identifier, the rating, and the record of a player of the session.
     *
     * @param pPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x0026aaa0
     * @ghidraAddress PAL: 0x00274630
     */
    void Set(const NetLaunchpadPlayer *pPlayer);

    /**
     * Report the rating without the bonus of a player past the first games.
     *
     * @return 10000 for a player with no games, otherwise the rating less 1000000 when that is
     * not negative, otherwise the rating.
     * @ghidraAddress NTSC-U/C: 0x0026aa28
     * @ghidraAddress PAL: 0x002745b8
     */
    int Rating() const;

    /**
     * Report whether the player has played fewer than kProvisionalGames games.
     *
     * @return Whether the rating is still provisional.
     * @ghidraAddress NTSC-U/C: 0x0026aa58
     * @ghidraAddress PAL: 0x002745e8
     */
    bool IsProvisional() const;

    /**
     * Rate a player from the scores of a game.
     *
     * The routine copies the stats of the player into this one, rates the player against every
     * other player, and clamps the rating from 0 to 90000 before it adds the bonus of a player
     * past the first games. A provisional rating averages the opponents' ratings and moves them
     * by the share of games won. Another rating moves by an Elo step against each opponent.
     *
     * @param nId The player.
     * @param pStats The stats of every player of the game, with their scores.
     * @ghidraAddress NTSC-U/C: 0x0026ab10
     * @ghidraAddress PAL: 0x002746a0
     */
    void Rate(int nId, const std::vector<RatingStat> *pStats);

    /**
     * Report the outcome of a game for a player.
     *
     * @param nScore The player's score.
     * @param nOpponentScore The opponent's score.
     * @return 1 for a win, 0.5 for a draw, and 0 for a loss.
     * @ghidraAddress NTSC-U/C: 0x0026aad8
     * @ghidraAddress PAL: 0x00274668
     */
    static float Outcome(int nScore, int nOpponentScore);

    /**
     * Report the score a player expects against an opponent from the difference of the ratings.
     *
     * @param nDifference The player's rating less the opponent's.
     * @return The expected score, 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x0026aef8
     * @ghidraAddress PAL: 0x00274a88
     */
    static float ExpectedScore(int nDifference);

    /**
     * Report the difference of ratings that makes a player expect a score, inverting
     * ExpectedScore() by interpolation.
     *
     * @param fScore The score.
     * @return The difference.
     * @ghidraAddress NTSC-U/C: 0x0026afc8
     * @ghidraAddress PAL: 0x00274b58
     */
    static float RatingDifference(float fScore);

    int mId;        /*!< The player's identifier in the session. */
    int mRating;    /*!< The rating, as LobbyPlayer::mRating. */
    int mGames;     /*!< The games counted, as RatingRecord::mGames. */
    int mOpponents; /*!< The sum of the opponents' weights, as RatingRecord::mOpponents. */
    float mWins;    /*!< The games won, as RatingRecord::mWins. */
    int mPoints;    /*!< The weighted sum of the opponents' ratings, as RatingRecord::mPoints. */
    int mScore;     /*!< The score of the game being rated. */

    /** The number of entries of sDifferences. */
    static constexpr int kDifferenceCount = 51;

    /**
     * The differences of ratings that make a player expect scores of 0.5, 0.51, and so on to 1.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0bd8
     */
    static int sDifferences[kDifferenceCount];
};
