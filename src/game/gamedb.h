#pragma once

#include "game/gameoptions.h"
#include "game/inputmap.h"
#include "game/playerprofile.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * Settings of the game being set up or played, among them the rule set and the timing the world
 * publishes.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the "db" section of the
 * configuration Init() reads. The one instance is the function-local static of shared(), and
 * TheGameDb addresses it. Only the members the recovered routines use are declared.
 * WorldMgr::UpdateTime() writes the two timing floats at `+0x158` and `+0x15c`.
 */
class GameDb {
public:
    /** Values of mRuleSet, one for each World subclass WorldMgr::Load() can build. */
    enum RuleSet {
        kRuleSetGame = 1,  /*!< WorldMgr::Load() builds a Game. */
        kRuleSetRemix = 2, /*!< WorldMgr::Load() builds a Remix. */
        kRuleSetDuel = 3,  /*!< WorldMgr::Load() builds a Duel. */
    };

    /** Values of mCommunity. */
    enum Community {
        kCommunitySolo = 1,   /*!< One player. */
        kCommunityLocal = 2,  /*!< Several players on this console. */
        kCommunityOnline = 3, /*!< Players on several consoles. */
    };

    /** The value of mPowerupLevel that places no power-up. */
    static constexpr int kPowerupLevelNone = 2;

    /**
     * Construct the database with its defaults.
     *
     * @ghidraAddress NTSC-U/C: 0x0026cf50
     * @ghidraAddress PAL: 0x00276af0
     */
    GameDb();

    /**
     * Report the single instance, constructing it on first use.
     *
     * @return The instance.
     * @ghidraAddress NTSC-U/C: 0x0026e4c0
     * @ghidraAddress PAL: 0x00278060
     */
    static GameDb *shared();

    /**
     * Read the "db" section of the configuration.
     *
     * @ghidraAddress NTSC-U/C: 0x0026e1b0
     * @ghidraAddress PAL: 0x00277d50
     */
    void Init();

    /**
     * Release what Init() created.
     *
     * @ghidraAddress NTSC-U/C: 0x0026e3f0
     * @ghidraAddress PAL: 0x00277f90
     */
    void Terminate();

    /**
     * Service the database once per frame.
     *
     * @ghidraAddress NTSC-U/C: 0x0026e518
     * @ghidraAddress PAL: 0x002780b8
     */
    void Poll();

    /**
     * Find the entry of a song in the "songs" section.
     *
     * @param pszSong The song.
     * @return The entry, or null.
     * @ghidraAddress NTSC-U/C: 0x0026de90
     * @ghidraAddress PAL: 0x00277a30
     */
    DataArray *FindSong(const char *pszSong);

    /**
     * Report the number of players.
     *
     * @return The number of players.
     * @ghidraAddress NTSC-U/C: 0x0026e938
     * @ghidraAddress PAL: 0x002784d8
     */
    int GetNumPlayers() const;

    /**
     * Report the number of controllers the players on this console use.
     *
     * @return The number of controllers.
     * @ghidraAddress NTSC-U/C: 0x0026e950
     * @ghidraAddress PAL: 0x002784f0
     */
    int GetNumPads() const;

    /**
     * Report whether a player plays on this console.
     *
     * @param nPlayer The player.
     * @return Whether the player is local.
     * @ghidraAddress NTSC-U/C: 0x0026e958
     * @ghidraAddress PAL: 0x002784f8
     */
    bool IsLocalPlayer(int nPlayer) const;

    /**
     * Report the position of a player in the order of the online session.
     *
     * @param nPlayer The player.
     * @return The position.
     * @ghidraAddress NTSC-U/C: 0x0026e9d0
     * @ghidraAddress PAL: 0x00278570
     */
    int GetPlayerNetOrder(int nPlayer) const;

    /**
     * Report the controller of a player.
     *
     * An online session gives every local player controller 0 and every other player none.
     *
     * @param nPlayer The player.
     * @return The controller, or -1 for none.
     * @ghidraAddress NTSC-U/C: 0x0026e9e8
     * @ghidraAddress PAL: 0x00278588
     */
    int GetPlayerPad(int nPlayer) const;

    /**
     * Report the profile of a player.
     *
     * @param nPlayer The player.
     * @return The profile.
     * @ghidraAddress NTSC-U/C: 0x0026ea20
     * @ghidraAddress PAL: 0x002785c0
     */
    PlayerProfile *GetProfile(int nPlayer);

    /**
     * Report the controller bindings in the profile of a player.
     *
     * @param nPlayer The player.
     * @return The bindings.
     * @ghidraAddress NTSC-U/C: 0x0026ea80
     * @ghidraAddress PAL: 0x00278620
     */
    InputMap *GetInputMap(int nPlayer);

    /**
     * Record the score of a player and rank the players by their scores.
     *
     * @param nPlayer The player.
     * @param nScore The score.
     * @ghidraAddress NTSC-U/C: 0x0026eab0
     * @ghidraAddress PAL: 0x00278650
     */
    void SetPlayerScore(int nPlayer, int nScore);

    /**
     * Report the demo being played.
     *
     * @return The demo, or 0 when no demo is played.
     * @ghidraAddress NTSC-U/C: 0x0026ec78
     * @ghidraAddress PAL: 0x00278818
     */
    int GetDemo() const;

    /**
     * Record whether the song was won.
     *
     * @param bWon Whether the song was won.
     * @ghidraAddress NTSC-U/C: 0x0026ecf8
     * @ghidraAddress PAL: 0x00278898
     */
    void SetWon(bool bWon);

    /**
     * Report the group of songs mSong belongs to.
     *
     * @return The group, or 0 when no group lists the song.
     * @ghidraAddress NTSC-U/C: 0x0026efb0
     * @ghidraAddress PAL: 0x00278b50
     */
    int FindSongGroup();

    /**
     * Report the index of mSong within its group, from the digit that ends its name.
     *
     * @return The index, or -1 for a song outside the groups.
     * @ghidraAddress NTSC-U/C: 0x0026f1d0
     * @ghidraAddress PAL: 0x00278d70
     */
    int GetSongIndex();

    /**
     * Report the settings of the game options screen.
     *
     * @return The settings.
     * @ghidraAddress NTSC-U/C: 0x0026f508
     * @ghidraAddress PAL: 0x002790a8
     */
    GameOptions *GetOptions();

    int mReserved00;     // +0x00, the player list. The element type is not yet recovered.
    int mReserved04;     // +0x04, the player list.
    int mReserved08;     // +0x08, the player list.
    int mReserved0C;     // +0x0c, the player list.
    String mSong;        /*!< The song of the game, the "song" entry of the "db" section. */
    int mLoadRemix;      /*!< `load_remix`, whether a saved remix is played. +0x24 */
    int mReserved28[6];  // +0x28, not yet recovered.
    int mPracticeMode;   /*!< `practice_mode`. +0x40 */
    int mTutorial;       /*!< Whether the tutorial is played. +0x44 */
    int mSkillLevel;     /*!< `skill_level`. +0x48 */
    int mPowerupLevel;   /*!< Index into GameConfig::mPowerupProbMulti. +0x4c */
    int mRuleSet;        /*!< `rule_set`, one of RuleSet. +0x50 */
    int mCommunity;      /*!< `community`, one of Community. +0x54 */
    int mReserved58[12]; // +0x58, not yet recovered.
    int mReserved88;     // +0x88, read by GameLogic::EndSong().
};

/**
 * The game database, GameDb::shared() as the unit's static initialiser stored it.
 *
 * @ghidraAddress NTSC-U/C: 0x00440d44
 */
extern GameDb *TheGameDb;
