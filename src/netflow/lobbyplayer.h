#pragma once

#include "game/avatarpartset.h"
#include "netflow/ratingrecord.h"
#include "os/datetime.h"
#include "os/prnstream.h"
#include "os/string.h"

class RatingStat;

/**
 * A player of the lobby server, as FindPlayerMsg reports it.
 *
 * The class has no RTTI, and the name is inferred. The object is 0x88 bytes.
 */
class LobbyPlayer {
public:
    /** Values of mConnectionType. */
    enum ConnectionType {
        kConnectionNone = 0,    /*!< No connection is known. */
        kConnectionSlow = 1,    /*!< A slow connection. */
        kConnectionFast = 2,    /*!< A fast connection. */
        kConnectionOffline = 3, /*!< The player is offline. */
    };

    /**
     * Construct a player with no name, the default Freq, and no rank.
     *
     * @ghidraAddress NTSC-U/C: 0x002688b0
     * @ghidraAddress PAL: 0x00272508
     */
    LobbyPlayer();

    /**
     * Report whether the player is new to the online game.
     *
     * A player leaves the first games behind once mRating gains 1000000.
     *
     * @return Whether mRating is at most 999999.
     * @ghidraAddress NTSC-U/C: 0x00268958
     * @ghidraAddress PAL: 0x002725b0
     */
    bool IsNewbie() const;

    /**
     * Take the rating and the record of a rated player.
     *
     * The name is inferred.
     *
     * @param pStat The player's stats after RatingStat::Rate().
     * @ghidraAddress NTSC-U/C: 0x0026aa70
     * @ghidraAddress PAL: 0x00274600
     */
    void SetRating(const RatingStat *pStat);

    int mId;               /*!< The account identifier. */
    String mName;          /*!< The account name. */
    int mConnectionType;   /*!< The kind of network connection, 0 to 3. */
    int mRank;             /*!< The position in the ranking. */
    int mMuted;            /*!< Non-zero when this console mutes the player's chat. */
    int mChannelId;        /*!< The chat channel the player is in, or -1. */
    int mRating;           /*!< The rating, plus 1000000 once the player is no longer new. */
    int mGames;            /*!< The number of online games played. */
    int mRankIcon;         /*!< The rank icon FindRankMaterial() finds, -1 to 3. */
    DateTime mLastOnline;  /*!< When the player was last online. */
    AvatarPartSet mAvatar; /*!< The player's Freq. */
    RatingRecord mRecord;  /*!< The results the next rating weighs. */
};

/**
 * Write a player as its identifier, name, rating, rank, rank icon, games, record, mute state, and
 * connection.
 *
 * @param stream The stream.
 * @param player The player.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00267ea0
 * @ghidraAddress PAL: 0x00271b78
 */
PrnStream &operator<<(PrnStream &stream, const LobbyPlayer &player);
