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

    /** @return The community of the game. */
    int GetCommunity() const {
        return mCommunity;
    }

    /** @param nCommunity The community of the game. */
    void SetCommunity(int nCommunity) {
        mCommunity = nCommunity;
    }

    /** @return Whether a saved remix is played. */
    int GetLoadRemix() const {
        return mLoadRemix;
    }

    /** @param nLoadRemix Whether a saved remix is played. */
    void SetLoadRemix(int nLoadRemix) {
        mLoadRemix = nLoadRemix;
    }

    /** @return The number of players the session takes. */
    int GetMaxPlayers() const {
        return mMaxPlayers;
    }

    /** @param nMaxPlayers The number of players the session takes. */
    void SetMaxPlayers(int nMaxPlayers) {
        mMaxPlayers = nMaxPlayers;
    }

    /** @return The power-up level. */
    int GetPowerupLevel() const {
        return mPowerupLevel;
    }

    /** @param nPowerupLevel The power-up level. */
    void SetPowerupLevel(int nPowerupLevel) {
        mPowerupLevel = nPowerupLevel;
    }

    /** @return The practice mode. */
    int GetPracticeMode() const {
        return mPracticeMode;
    }

    /** @param nPracticeMode The practice mode. */
    void SetPracticeMode(int nPracticeMode) {
        mPracticeMode = nPracticeMode;
    }

    /** @return The name of the remix the game plays. */
    const String &GetRemixName() const {
        return mRemixName;
    }

    /** @return The name of the remix the game plays, for filling. */
    String &GetRemixName() {
        return mRemixName;
    }

    /** @param remixName The name of the remix the game plays. */
    void SetRemixName(const char *pszRemixName) {
        mRemixName = pszRemixName;
    }

    /** @return Whether the remix of the game cannot be changed. */
    int GetRemixReadOnly() const {
        return mRemixReadOnly;
    }

    /** @param nRemixReadOnly Whether the remix of the game cannot be changed. */
    void SetRemixReadOnly(int nRemixReadOnly) {
        mRemixReadOnly = nRemixReadOnly;
    }

    /** @return The game mode. */
    int GetRuleSet() const {
        return mRuleSet;
    }

    /** @param nRuleSet The game mode. */
    void SetRuleSet(int nRuleSet) {
        mRuleSet = nRuleSet;
    }

    /** @return The skill level. */
    int GetSkillLevel() const {
        return mSkillLevel;
    }

    /** @param nSkillLevel The skill level. */
    void SetSkillLevel(int nSkillLevel) {
        mSkillLevel = nSkillLevel;
    }

    /** @return The song of the game. */
    const String &GetSong() const {
        return mSong;
    }

    /** @return The song of the game, for filling. */
    String &GetSong() {
        return mSong;
    }

    /** @param song The song of the game. */
    void SetSong(const char *pszSong) {
        mSong = pszSong;
    }

    /** @return Whether the tutorial is played. */
    int GetTutorial() const {
        return mTutorial;
    }

    /** @param nTutorial Whether the tutorial is played. */
    void SetTutorial(int nTutorial) {
        mTutorial = nTutorial;
    }

private:
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
