#pragma once

#include "game/avatarpartset.h"
#include "os/datetime.h"
#include "os/string.h"

/**
 * A player of the lobby server, as FindPlayerMsg reports it.
 *
 * The class has no RTTI, and the name is inferred.
 */
class LobbyPlayer {
public:
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
     * The name is inferred.
     *
     * @return Whether mReserved28 is at most 999999.
     * @ghidraAddress NTSC-U/C: 0x00268958
     * @ghidraAddress PAL: 0x002725b0
     */
    bool IsNewbie() const;

    int mId;               /*!< The account identifier. */
    String mName;          /*!< The account name. */
    int mConnectionType;   /*!< The kind of network connection, 0 to 3. */
    int mRank;             /*!< The position in the ranking. */
    int mReserved20;       // +0x20, not yet identified.
    int mReserved24;       // +0x24, not yet identified.
    int mReserved28;       // +0x28, not yet identified. IsNewbie() reads it.
    int mGames;            /*!< The number of online games played. */
    int mRankIcon;         /*!< The rank icon FindRankMaterial() finds, -1 to 3. */
    DateTime mLastOnline;  /*!< When the player was last online. */
    AvatarPartSet mAvatar; /*!< The player's Freq. */
    char mReserved78[16];  // +0x78, not yet identified.
};
