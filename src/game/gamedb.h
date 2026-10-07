#pragma once

#include <vector>

#include "game/avatarpartset.h"
#include "game/gameoptions.h"
#include "game/inputmap.h"
#include "game/playerprofile.h"
#include "game/remixinfo.h"
#include "game/songentry.h"
#include "netflow/netgameparams.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * Settings of the game being set up or played, among them the rule set and the timing the world
 * publishes.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the "db" section of the
 * configuration Init() reads. The one instance is the function-local static of shared(), and
 * TheGameDb addresses it. Only the members the recovered routines use are declared.
 * WorldMgr::Load() reads the rule set from `+0x50`, and WorldMgr::UpdateTime() writes the two
 * timing floats at `+0x158` and `+0x15c`.
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
        kCommunityNone = 0,   /*!< No game is being set up, as in the options menu. */
        kCommunitySolo = 1,   /*!< One player. */
        kCommunityLocal = 2,  /*!< Several players on this console. */
        kCommunityOnline = 3, /*!< Players on several consoles. */
    };

    /** Values of mSkillLevel. The names follow the buttons of the skill menu. */
    enum SkillLevel {
        kSkillNovice = 0,       /*!< The `novice_but` level. */
        kSkillIntermediate = 1, /*!< The `intermediate_but` level. */
        kSkillAdvanced = 2,     /*!< The `advanced_but` level. */
        kSkillInsane = 3,       /*!< The `insane_but` level. */
        kSkillAny = 4,          /*!< Any level, for the song and arena queries. */
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
     * Take the settings of an online game the host published.
     *
     * @param pParams The settings.
     * @ghidraAddress NTSC-U/C: 0x0026eae0
     * @ghidraAddress PAL: 0x00278680
     */
    void SetGameParams(const NetGameParams *pParams);

    /**
     * Replace the buffer of the remix data with a new one, billed to `RemixBuf`.
     *
     * @param nSize The size of the new buffer, or 0 for none.
     * @ghidraAddress NTSC-U/C: 0x0026e0f8
     * @ghidraAddress PAL: 0x00277c98
     */
    void SetRemixBuffer(int nSize);

    /**
     * Copy the description of the remix the game plays.
     *
     * @param pInfo The description.
     * @ghidraAddress NTSC-U/C: 0x0026e150
     * @ghidraAddress PAL: 0x00277cf0
     */
    void SetRemix(const RemixInfo *pInfo);

    /**
     * Set the flag that the screens which choose an existing or a tutorial remix set.
     *
     * The name is inferred.
     *
     * @param nActive The flag.
     * @ghidraAddress NTSC-U/C: 0x0026e198
     * @ghidraAddress PAL: 0x00277d38
     */
    void SetRemixActive(int nActive);

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
     * Report the slot of a player, which selects the player's sounds and colours.
     *
     * @param nPlayer The player.
     * @return The slot.
     * @ghidraAddress NTSC-U/C: 0x0026e970
     * @ghidraAddress PAL: 0x00278510
     */
    int GetPlayerSlot(int nPlayer) const;

    /**
     * Report the name of a player.
     *
     * @param nPlayer The player.
     * @return The name.
     * @ghidraAddress NTSC-U/C: 0x0026e9b8
     * @ghidraAddress PAL: 0x00278558
     */
    const char *GetPlayerName(int nPlayer) const;

    /**
     * Report the colour symbol of a player's difficulty.
     *
     * The name is inferred.
     *
     * @param nPlayer The player.
     * @return The symbol.
     * @ghidraAddress NTSC-U/C: 0x0026e988
     * @ghidraAddress PAL: 0x00278528
     */
    const char *GetPlayerColor(int nPlayer) const;

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
     * Report the avatar parts of a player, which its profile stores.
     *
     * @param nPlayer The player.
     * @return The parts.
     * @ghidraAddress NTSC-U/C: 0x0026ea38
     * @ghidraAddress PAL: 0x002785d8
     */
    AvatarPartSet *GetAvatar(int nPlayer);

    /**
     * Report the score SetPlayerScore() recorded for a player.
     *
     * @param nPlayer The player.
     * @return The score.
     * @ghidraAddress NTSC-U/C: 0x0026ea50
     * @ghidraAddress PAL: 0x002785f0
     */
    int GetPlayerScore(int nPlayer) const;

    /**
     * Report the rank SetPlayerScore() gave a player, 0 for the best score.
     *
     * @param nPlayer The player.
     * @return The rank.
     * @ghidraAddress NTSC-U/C: 0x0026ea68
     * @ghidraAddress PAL: 0x00278608
     */
    int GetPlayerRank(int nPlayer) const;

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
     * Report the file of the demo being played.
     *
     * @return The file, or null when no demo is played.
     * @ghidraAddress NTSC-U/C: 0x0026ec78
     * @ghidraAddress PAL: 0x00278818
     */
    const char *GetDemo() const;

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

    /**
     * Replace the settings of the game options screen and apply the speaker output mode.
     *
     * @param pOptions The settings.
     * @ghidraAddress NTSC-U/C: 0x0026f510
     * @ghidraAddress PAL: 0x002790b0
     */
    void SetOptions(const GameOptions *pOptions);

    /**
     * Pass two camera values to the renderer of the players' avatars.
     *
     * The metagame passes 0.05 and 1 as the front end starts. The meaning of the two values is
     * not yet recovered, and the name is inferred.
     *
     * @param fFirst The first value.
     * @param fSecond The second value.
     * @ghidraAddress NTSC-U/C: 0x0026f4e8
     * @ghidraAddress PAL: 0x00279088
     */
    void SetAvatarCameraParams(float fFirst, float fSecond);

    /**
     * Unlock every song.
     *
     * The `unlock_all songs` cheat calls it. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0026e0b0
     * @ghidraAddress PAL: 0x00277c50
     */
    void UnlockAllSongs();

    /**
     * Report whether the campaign win sequence runs.
     *
     * @return Non-zero while the sequence runs.
     * @ghidraAddress NTSC-U/C: 0x0026e0d0
     * @ghidraAddress PAL: 0x00277c70
     */
    int IsWinSequence() const;

    /**
     * Start or end the campaign win sequence.
     *
     * @param bWinSequence Start the sequence.
     * @ghidraAddress NTSC-U/C: 0x0026e0d8
     * @ghidraAddress PAL: 0x00277c78
     */
    void SetWinSequence(bool bWinSequence);

    /**
     * Set the practice mode.
     *
     * @param bPracticeMode Play in practice mode.
     * @ghidraAddress NTSC-U/C: 0x0026eba8
     * @ghidraAddress PAL: 0x00278748
     */
    void SetPracticeMode(bool bPracticeMode);

    /**
     * Remove every player.
     *
     * @ghidraAddress NTSC-U/C: 0x0026e548
     * @ghidraAddress PAL: 0x002780e8
     */
    void ClearPlayers();

    /**
     * Add a player with a copy of a profile.
     *
     * @param pProfile The profile to copy.
     * @ghidraAddress NTSC-U/C: 0x0026e5e0
     * @ghidraAddress PAL: 0x00278180
     */
    void AddPlayer(const PlayerProfile *pProfile);

    /**
     * Set mSkillLevel.
     *
     * @param nSkillLevel The skill level.
     * @ghidraAddress NTSC-U/C: 0x0026eb98
     * @ghidraAddress PAL: 0x00278738
     */
    void SetSkillLevel(int nSkillLevel);

    /**
     * Set mPowerupLevel.
     *
     * @param nPowerupLevel The power-up level.
     * @ghidraAddress NTSC-U/C: 0x0026eba0
     * @ghidraAddress PAL: 0x00278740
     */
    void SetPowerupLevel(int nPowerupLevel);

    /**
     * Set whether the tutorial is played.
     *
     * The solo arena screen sets it when the player confirms the "Tutorial" arena, and the
     * song screen sets it for a song whose `type` is 4. Every other way into a song clears it.
     *
     * @param nTutorial Non-zero to play the tutorial.
     * @ghidraAddress NTSC-U/C: 0x0026ebb0
     * @ghidraAddress PAL: 0x00278750
     */
    void SetTutorial(int nTutorial);

    /**
     * Set mSong.
     *
     * @param pszSong The song.
     * @ghidraAddress NTSC-U/C: 0x0026ebb8
     * @ghidraAddress PAL: 0x00278758
     */
    void SetSong(const char *pszSong);

    /**
     * Set mLoadRemix.
     *
     * @param bLoadRemix Whether a saved remix is played.
     * @ghidraAddress NTSC-U/C: 0x0026ec58
     * @ghidraAddress PAL: 0x002787f8
     */
    void SetLoadRemix(bool bLoadRemix);

    /**
     * Set mRuleSet.
     *
     * @param nRuleSet One of RuleSet.
     * @ghidraAddress NTSC-U/C: 0x0026ec60
     * @ghidraAddress PAL: 0x00278800
     */
    void SetRuleSet(int nRuleSet);

    /**
     * Set mCommunity.
     *
     * @param nCommunity One of Community.
     * @ghidraAddress NTSC-U/C: 0x0026ec68
     * @ghidraAddress PAL: 0x00278808
     */
    void SetCommunity(int nCommunity);

    /**
     * Choose the demo to play, or none.
     *
     * @param pszDemo The demo recording, or null for none.
     * @ghidraAddress NTSC-U/C: 0x0026ec98
     * @ghidraAddress PAL: 0x00278838
     */
    void SetDemo(const char *pszDemo);

    /**
     * Record the fraction of the possible capture bars a solo song captured.
     *
     * @param fEnergized The fraction.
     * @ghidraAddress NTSC-U/C: 0x0026ecb8
     * @ghidraAddress PAL: 0x00278858
     */
    void SetEnergized(float fEnergized);

    /**
     * Record the bars a solo song played with every track captured.
     *
     * @param nBars The bars.
     * @ghidraAddress NTSC-U/C: 0x0026ecc8
     * @ghidraAddress PAL: 0x00278868
     */
    void SetFullMixBars(int nBars);

    /**
     * Record the longest streak of a solo song.
     *
     * @param nStreak The streak.
     * @ghidraAddress NTSC-U/C: 0x0026ecd8
     * @ghidraAddress PAL: 0x00278878
     */
    void SetBestStreak(int nStreak);

    /**
     * Record the fraction of the song played.
     *
     * @param fProgress The fraction.
     * @ghidraAddress NTSC-U/C: 0x0026ece8
     * @ghidraAddress PAL: 0x00278888
     */
    void SetProgress(float fProgress);

    /**
     * Report the value of SetEnergized().
     *
     * @return The value.
     * @ghidraAddress NTSC-U/C: 0x0026ecc0
     * @ghidraAddress PAL: 0x00278860
     */
    float GetEnergized() const;

    /**
     * Report the value of SetFullMixBars().
     *
     * @return The bars.
     * @ghidraAddress NTSC-U/C: 0x0026ecd0
     * @ghidraAddress PAL: 0x00278870
     */
    int GetFullMixBars() const;

    /**
     * Report the value of SetBestStreak().
     *
     * @return The streak.
     * @ghidraAddress NTSC-U/C: 0x0026ece0
     * @ghidraAddress PAL: 0x00278880
     */
    int GetBestStreak() const;

    /**
     * Report the value of SetProgress().
     *
     * @return The fraction.
     * @ghidraAddress NTSC-U/C: 0x0026ecf0
     * @ghidraAddress PAL: 0x00278890
     */
    float GetProgress() const;

    /**
     * Set the arena of the game.
     *
     * @param pszArena The arena.
     * @ghidraAddress NTSC-U/C: 0x0026ec38
     * @ghidraAddress PAL: 0x002787d8
     */
    void SetArena(const char *pszArena);

    /**
     * Report the description of the remix being played.
     *
     * @return The description.
     * @ghidraAddress NTSC-U/C: 0x0026e0e8
     * @ghidraAddress PAL: 0x00277c88
     */
    RemixInfo *GetRemixInfo();

    /**
     * Set mRemixReadOnly.
     *
     * @param nReadOnly Non-zero when the remix may not be saved over.
     * @ghidraAddress NTSC-U/C: 0x0026ec30
     * @ghidraAddress PAL: 0x002787d0
     */
    void SetRemixReadOnly(int nReadOnly);

    /**
     * Report whether the remix of an online game has ended.
     *
     * @return Non-zero once the remix has ended.
     * @ghidraAddress NTSC-U/C: 0x0026e1a0
     * @ghidraAddress PAL: 0x00277d40
     */
    int GetNetRemixEnded() const;

    /**
     * Record whether the remix of an online game has ended.
     *
     * @param nEnded Non-zero once the remix has ended.
     * @ghidraAddress NTSC-U/C: 0x0026e1a8
     * @ghidraAddress PAL: 0x00277d48
     */
    void SetNetRemixEnded(int nEnded);

    /**
     * Report the localised name of the difficulty of the game.
     *
     * @return The name.
     * @ghidraAddress NTSC-U/C: 0x0026f238
     * @ghidraAddress PAL: 0x00278dd8
     */
    const char *GetDifficultyName() const;

    /**
     * Report the localised name of a difficulty.
     *
     * @param nSkillLevel The skill level.
     * @param nRuleSet The rule set, one of RuleSet.
     * @return The name.
     * @ghidraAddress NTSC-U/C: 0x0026f290
     * @ghidraAddress PAL: 0x00278e30
     */
    const char *GetDifficultyName(int nSkillLevel, int nRuleSet) const;

    /**
     * Report the localised name of the mode of the game, from mRuleSet.
     *
     * @return The name.
     * @ghidraAddress NTSC-U/C: 0x0026f3d0
     * @ghidraAddress PAL: 0x00278f70
     */
    const char *GetModeName() const;

    /**
     * Report the localised name of a mode.
     *
     * @param nRuleSet The rule set, one of RuleSet.
     * @return The name, or the empty string for another value.
     * @ghidraAddress NTSC-U/C: 0x0026f3f0
     * @ghidraAddress PAL: 0x00278f90
     */
    const char *GetModeName(int nRuleSet) const;

    /**
     * Report the localised name of a power-up level.
     *
     * The name is inferred.
     *
     * @param nPowerupLevel The level, 0 to 2.
     * @return The name, or the empty string for another value.
     * @ghidraAddress NTSC-U/C: 0x0026f470
     * @ghidraAddress PAL: 0x00279010
     */
    const char *GetPowerupName(int nPowerupLevel) const;

    /**
     * Append the names of the arenas of the "arenas" section, in their order there.
     *
     * @param pArenas The list to append to.
     * @param bIncludeTutorial Whether the `Tutorial` arena is listed.
     * @ghidraAddress NTSC-U/C: 0x0026d0a0
     * @ghidraAddress PAL: 0x00276c40
     */
    void GetArenaNames(std::vector<const char *> *pArenas, bool bIncludeTutorial);

    /**
     * List the arenas the players have unlocked at a skill level.
     *
     * @param pArenas The list to fill.
     * @param nSkillLevel The skill level, or 4 for any.
     * @param bFirstPlayerOnly Whether only the first player's unlocks count.
     * @param bIncludeTutorial Whether the `Tutorial` arena is considered.
     * @ghidraAddress NTSC-U/C: 0x0026dd40
     * @ghidraAddress PAL: 0x002778e0
     */
    void GetUnlockedArenas(std::vector<const char *> *pArenas,
                           int nSkillLevel,
                           bool bFirstPlayerOnly,
                           bool bIncludeTutorial);

    /**
     * Append the songs of an arena that are available at a skill level.
     *
     * @param pSongs The list to append to.
     * @param pszArena The arena.
     * @param nSkillLevel The skill level, or 4 for any.
     * @param nFilter Which songs are listed. The callers pass 0 and 1.
     * @ghidraAddress NTSC-U/C: 0x0026d2b0
     * @ghidraAddress PAL: 0x00276e50
     */
    void GetArenaSongs(std::vector<SongEntry> *pSongs,
                       const char *pszArena,
                       int nSkillLevel,
                       int nFilter);

    /**
     * List the songs the players have unlocked in an arena, or in every unlocked arena.
     *
     * @param pSongs The list to fill.
     * @param pszArena The arena, or an empty string for every unlocked arena.
     * @param nSkillLevel The skill level, or 4 for any.
     * @param bFirstPlayerOnly Whether only the first player's unlocks count.
     * @ghidraAddress NTSC-U/C: 0x0026d9a0
     * @ghidraAddress PAL: 0x00277540
     */
    void GetUnlockedSongs(std::vector<SongEntry> *pSongs,
                          const char *pszArena,
                          int nSkillLevel,
                          bool bFirstPlayerOnly);

    int mReserved00;     // +0x00, the player list. The element type is not yet recovered.
    int mReserved04;     // +0x04, the player list.
    int mReserved08;     // +0x08, the player list.
    int mReserved0C;     // +0x0c, the player list.
    String mSong;        /*!< The song of the game, the "song" entry of the "db" section. */
    int mLoadRemix;      /*!< `load_remix`, whether a saved remix is played. +0x24 */
    int mRemixReadOnly;  /*!< Non-zero when the remix may not be saved over. +0x28 */
    String mRemixName;   /*!< The name of the saved remix played. +0x2c */
    int mPracticeMode;   /*!< `practice_mode`. +0x40 */
    int mTutorial;       /*!< Whether the tutorial is played. +0x44 */
    int mSkillLevel;     /*!< `skill_level`. +0x48 */
    int mPowerupLevel;   /*!< Index into GameConfig::mPowerupProbMulti. +0x4c */
    int mRuleSet;        /*!< `rule_set`, one of RuleSet. +0x50 */
    int mCommunity;      /*!< `community`, one of Community. +0x54 */
    int mReserved58;     // +0x58, not yet recovered.
    String mArena;       /*!< The arena of the game. +0x5c */
    int mReserved70[6];  // +0x70, not yet recovered.
    int mWinSequence;    /*!< Whether the campaign win sequence runs. +0x88 */
    int mReserved8C[2];  // +0x8c, not yet recovered.
    float mEnergized;    /*!< The value of SetEnergized(). +0x94 */
    int mFullMixBars;    /*!< The value of SetFullMixBars(). +0x98 */
    int mBestStreak;     /*!< The value of SetBestStreak(). +0x9c */
    float mProgress;     /*!< The value of SetProgress(). +0xa0 */
    int mReservedA4[44]; // +0xa4, not yet recovered.
    int mNetRemixEnded;  /*!< Whether the remix of an online game has ended. */
    float mSongTick;     /*!< The running world's position in ticks. +0x158 */
    float mSongTime;     /*!< The running world's position on its song clock. +0x15c */
};

/**
 * The game database, GameDb::shared() as the unit's static initialiser stored it.
 *
 * @ghidraAddress NTSC-U/C: 0x00440d44
 */
extern GameDb *TheGameDb;
