#pragma once

#include "os/binstream.h"
#include "os/prnstream.h"
#include "os/string.h"

/**
 * The settings of an online game that the host publishes with its session.
 *
 * The record is 0x4c bytes. GameDb::SetGameParams() copies it member by member into the members
 * of the game database from mSong onwards.
 */
class NetGameParams {
public:
    /** The power-up level, the rule set, and the community of default settings. */
    static constexpr int kDefaultSelection = 1;

    /** The number of players default settings take. */
    static constexpr int kDefaultMaxPlayers = 4;

    /**
     * Construct the settings of a game of the first rule set, for four players.
     *
     * The constructor has no out-of-line copy. The defaults match those of GameDb.
     */
    NetGameParams()
        : mLoadRemix(0), mRemixReadOnly(0), mPracticeMode(0), mTutorial(0), mSkillLevel(0),
          mPowerupLevel(kDefaultSelection), mRuleSet(kDefaultSelection),
          mCommunity(kDefaultSelection), mMaxPlayers(kDefaultMaxPlayers) {
    }

    String mSong;       /*!< The song of the game. */
    int mLoadRemix;     /*!< Whether a saved remix is played. */
    int mRemixReadOnly; /*!< Whether the remix of the game cannot be changed. */
    String mRemixName;  /*!< The name of the remix the game plays. */
    int mPracticeMode;  /*!< The practice mode. */
    int mTutorial;      /*!< Whether the tutorial is played. */
    int mSkillLevel;    /*!< The skill level. */
    int mPowerupLevel;  /*!< The power-up level. */
    int mRuleSet;       /*!< The game mode. */
    int mCommunity;     /*!< The community of the game. */
    int mMaxPlayers;    /*!< The number of players the session takes. */
};

/**
 * Write the settings of a game.
 *
 * @param stream The stream.
 * @param params The settings.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027c418
 * @ghidraAddress PAL: 0x00285e48
 */
PrnStream &operator<<(PrnStream &stream, const NetGameParams &params);

/**
 * Write the settings of a game to the packets of a session.
 *
 * @param stream The stream.
 * @param params The settings.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027c688
 * @ghidraAddress PAL: 0x002860b8
 */
BinStream &operator<<(BinStream &stream, const NetGameParams &params);

/**
 * Read the settings operator<<() wrote.
 *
 * @param stream The stream.
 * @param params Receives the settings.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027c820
 * @ghidraAddress PAL: 0x002861f0
 */
BinStream &operator>>(BinStream &stream, NetGameParams &params);
