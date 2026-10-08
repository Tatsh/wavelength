#pragma once

#include "os/string.h"

/**
 * The settings of an online game that the host publishes with its session.
 *
 * The record is 0x4c bytes. GameDb::SetGameParams() copies it member by member into the members
 * of the game database from mSong onwards.
 */
class NetGameParams {
public:
    /**
     * Construct the settings of a game of one player at the first skill level, for up to four
     * players.
     *
     * Inline. NetHostingScreen's constructor expands it.
     */
    NetGameParams() {
        mLoadRemix = 0;
        mRemixReadOnly = 0;
        mNetPlayers = kDefaultNetPlayers;
        mCommunity = 1;
        mPowerupLevel = 1;
        mRuleSet = 1;
        mPracticeMode = 0;
        mTutorial = 0;
        mSkillLevel = 0;
    }

    /** The default of mNetPlayers. */
    static constexpr int kDefaultNetPlayers = 4;

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
    int mNetPlayers;    /*!< The number of players of the game. */
};
