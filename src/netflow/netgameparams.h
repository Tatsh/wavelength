#pragma once

#include "os/string.h"

/**
 * The settings of an online game that the host publishes with its session.
 *
 * The record is 0x4c bytes. GameDb::SetGameParams() copies it member by member into the members
 * of the game database from mSong onwards.
 */
struct NetGameParams {
    String mSong;       /*!< The song of the game. */
    int mLoadRemix;     /*!< Whether a saved remix is played. */
    int mRemixReadOnly; /*!< Whether the remix of the game cannot be changed. */
    String mReserved1C; // +0x1c, not yet recovered.
    int mPracticeMode;  /*!< The practice mode. */
    int mTutorial;      /*!< Whether the tutorial is played. */
    int mSkillLevel;    /*!< The skill level. */
    int mPowerupLevel;  /*!< The power-up level. */
    int mRuleSet;       /*!< The game mode. */
    int mCommunity;     /*!< The community of the game. */
    int mReserved48;    // +0x48, not yet recovered.
};
