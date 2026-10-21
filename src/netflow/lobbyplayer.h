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

    /** @return The player's Freq. */
    const AvatarPartSet &GetAvatar() const {
        return mAvatar;
    }

    /** @return The player's Freq, for filling. */
    AvatarPartSet &GetAvatar() {
        return mAvatar;
    }

    /** @param avatar The player's Freq. */
    void SetAvatar(const AvatarPartSet &avatar) {
        mAvatar = avatar;
    }

    /** @return The chat channel the player is in, or -1. */
    int GetChannelId() const {
        return mChannelId;
    }

    /** @param nChannelId The chat channel the player is in, or -1. */
    void SetChannelId(int nChannelId) {
        mChannelId = nChannelId;
    }

    /** @return The kind of network connection, 0 to 3. */
    int GetConnectionType() const {
        return mConnectionType;
    }

    /** @param nConnectionType The kind of network connection, 0 to 3. */
    void SetConnectionType(int nConnectionType) {
        mConnectionType = nConnectionType;
    }

    /** @return The number of online games played. */
    int GetGames() const {
        return mGames;
    }

    /** @param nGames The number of online games played. */
    void SetGames(int nGames) {
        mGames = nGames;
    }

    /** @return The account identifier. */
    int GetId() const {
        return mId;
    }

    /** @param nId The account identifier. */
    void SetId(int nId) {
        mId = nId;
    }

    /** @return When the player was last online. */
    const DateTime &GetLastOnline() const {
        return mLastOnline;
    }

    /** @return When the player was last online, for filling. */
    DateTime &GetLastOnline() {
        return mLastOnline;
    }

    /** @param lastOnline When the player was last online. */
    void SetLastOnline(const DateTime &lastOnline) {
        mLastOnline = lastOnline;
    }

    /** @return Non-zero when this console mutes the player's chat. */
    int GetMuted() const {
        return mMuted;
    }

    /** @param nMuted Non-zero when this console mutes the player's chat. */
    void SetMuted(int nMuted) {
        mMuted = nMuted;
    }

    /** @return The account name. */
    const String &GetName() const {
        return mName;
    }

    /** @return The account name, for filling. */
    String &GetName() {
        return mName;
    }

    /** @param name The account name. */
    void SetName(const char *pszName) {
        mName = pszName;
    }

    /** @return The position in the ranking. */
    int GetRank() const {
        return mRank;
    }

    /** @param nRank The position in the ranking. */
    void SetRank(int nRank) {
        mRank = nRank;
    }

    /** @return The rank icon FindRankMaterial() finds, -1 to 3. */
    int GetRankIcon() const {
        return mRankIcon;
    }

    /** @param nRankIcon The rank icon FindRankMaterial() finds, -1 to 3. */
    void SetRankIcon(int nRankIcon) {
        mRankIcon = nRankIcon;
    }

    /** @return The rating, plus 1000000 once the player is no longer new. */
    int GetRating() const {
        return mRating;
    }

    /** @param nRating The rating. */
    void SetRating(int nRating) {
        mRating = nRating;
    }

    /** @return The results the next rating weighs. */
    const RatingRecord &GetRecord() const {
        return mRecord;
    }

    /** @return The results the next rating weighs, for filling. */
    RatingRecord &GetRecord() {
        return mRecord;
    }

    /** @param record The results the next rating weighs. */
    void SetRecord(const RatingRecord &record) {
        mRecord = record;
    }

private:
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
