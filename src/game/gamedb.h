#pragma once

#include <vector>

#include "game/avatarpartset.h"
#include "game/campaign.h"
#include "game/gameoptions.h"
#include "game/inputmap.h"
#include "game/playerinfo.h"
#include "game/remixinfo.h"
#include "game/songentry.h"
#include "netflow/netgameparams.h"
#include "os/scheduler.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * Settings of the game being set up or played, among them the rule set and the timing the world
 * publishes.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred from the "db" section of the
 * configuration Init() reads. The one instance is the function-local static of shared(), and
 * TheGameDb addresses it. WorldMgr::Load() reads the rule set from `+0x50`, and
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
     * Build TheFxMidi from the `fx_midi_file` entry of the "db" section, and start mSfxScheduler
     * at the tempo of that file. Init() calls it. The PAL build reads the
     * `<language>_fx_midi_file` entry of the console's language instead.
     *
     * @ghidraAddress NTSC-U/C: 0x0027f8e0
     * @ghidraAddress PAL: 0x002891c8
     */
    void InitSfx();

    /**
     * Stop mSfxScheduler and release TheFxMidi. Terminate() calls it.
     *
     * @ghidraAddress NTSC-U/C: 0x0027f998
     * @ghidraAddress PAL: 0x00289298
     */
    void TerminateSfx();

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
     * Report the buffer of the remix data.
     *
     * @return mRemixBuffer.
     * @ghidraAddress NTSC-U/C: 0x0026e0e0
     * @ghidraAddress PAL: 0x00277c80
     */
    char *GetRemixBuffer() {
        return mRemixBuffer;
    }

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
     * Report the flag SetRemixActive() set.
     *
     * The name is inferred.
     *
     * @return The flag.
     * @ghidraAddress NTSC-U/C: 0x0026e190
     * @ghidraAddress PAL: 0x00277d30
     */
    int GetRemixActive() const;

    /**
     * Set mRemixName.
     *
     * @param pszName The name of the saved remix played.
     * @ghidraAddress NTSC-U/C: 0x0026ec10
     * @ghidraAddress PAL: 0x002787b0
     */
    void SetRemixName(const char *pszName);

    /**
     * List the songs of every arena but the tutorial that are available at a skill level, without
     * the songs of type 3 that the first profile has not unlocked.
     *
     * The name is inferred.
     *
     * @param pSongs The list to fill.
     * @param nSkillLevel The skill level, or 4 for any.
     * @param bDuelOnly Whether only the songs a duel can play are listed.
     * @ghidraAddress NTSC-U/C: 0x0026d8a0
     * @ghidraAddress PAL: 0x00277440
     */
    void GetHostSongs(std::vector<SongEntry> *pSongs, int nSkillLevel, bool bDuelOnly);

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
     * @return PlayerInfo::mLocal of the player, non-zero for a local player.
     * @ghidraAddress NTSC-U/C: 0x0026e958
     * @ghidraAddress PAL: 0x002784f8
     */
    int IsLocalPlayer(int nPlayer) const;

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
    Campaign *GetProfile(int nPlayer);

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
     * Report the arena whose songs include mSong, the tutorial included.
     *
     * @return The name of the arena, or null when no arena lists the song.
     * @ghidraAddress NTSC-U/C: 0x0026efb0
     * @ghidraAddress PAL: 0x00278b50
     */
    const char *FindSongGroup();

    /**
     * Report the index of the arena of the game, from the digit that ends mArena, less one.
     *
     * @return The index, or -1 when mSong has a `type` or mArena is shorter than two characters.
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
     * Set the depth range of the camera the avatars share, with Avatar::SetDepthRange().
     *
     * The metagame passes 0.05 and 1 as the front end starts. The name is inferred.
     *
     * @param fNear The near end of the range.
     * @param fFar The far end of the range.
     * @ghidraAddress NTSC-U/C: 0x0026f4e8
     * @ghidraAddress PAL: 0x00279088
     */
    void SetAvatarCameraParams(float fNear, float fFar);

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
    void AddPlayer(const Campaign *pProfile);

    /**
     * Add a player of an online game with a copy of a profile.
     *
     * @param bLocal Non-zero for the player of this console, which also counts it as local.
     * @param pProfile The profile to copy.
     * @param nSlot The value GetPlayerSlot() reports for the player.
     * @param nNetOrder The value GetPlayerNetOrder() reports for the player.
     * @ghidraAddress NTSC-U/C: 0x0026e790
     * @ghidraAddress PAL: 0x00278330
     */
    void AddPlayer(int bLocal, const Campaign *pProfile, int nSlot, int nNetOrder);

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
     * Set mMaxPlayers.
     *
     * @param nMaxPlayers The number of players the session takes.
     * @ghidraAddress NTSC-U/C: 0x0026ec70
     * @ghidraAddress PAL: 0x00278810
     */
    void SetMaxPlayers(int nMaxPlayers);

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

    /**
     * List the songs of every arena but the tutorial that are available at a skill level.
     *
     * @param pSongs The list to fill.
     * @param bDuelOnly Whether only the songs a duel can play are listed.
     * @param nSkillLevel The skill level, or kSkillAny.
     * @ghidraAddress NTSC-U/C: 0x0026d580
     * @ghidraAddress PAL: 0x00277120
     */
    void GetAllSongs(std::vector<SongEntry> *pSongs, bool bDuelOnly, int nSkillLevel);

    /**
     * Report whether an arena lists a song.
     *
     * @param pszSong The song.
     * @return Whether an arena lists the song.
     * @ghidraAddress NTSC-U/C: 0x0026df10
     * @ghidraAddress PAL: 0x00277ab0
     */
    bool HasSong(const char *pszSong);

    /**
     * Find the song whose `id` matches.
     *
     * @param ppSong Receives the entry of the song found in the last arena that lists one.
     * @param nId The identifier.
     * @return Whether a song matched.
     * @ghidraAddress NTSC-U/C: 0x0026df90
     * @ghidraAddress PAL: 0x00277b30
     */
    bool FindSongById(DataArray **ppSong, int nId);

    std::vector<PlayerInfo *> mPlayers; /*!< The players, which the database allocates. */
    String mSong;            /*!< The song of the game, the "song" entry of the "db" section. */
    int mLoadRemix;          /*!< `load_remix`, whether a saved remix is played. */
    int mRemixReadOnly;      /*!< Non-zero when the remix may not be saved over. */
    String mRemixName;       /*!< The name of the saved remix played. */
    int mPracticeMode;       /*!< `practice_mode`. */
    int mTutorial;           /*!< Whether the tutorial is played. */
    int mSkillLevel;         /*!< `skill_level`. */
    int mPowerupLevel;       /*!< Index into GameConfig::mPowerupProbMulti. */
    int mRuleSet;            /*!< `rule_set`, one of RuleSet. */
    int mCommunity;          /*!< `community`, one of Community. */
    int mMaxPlayers;         /*!< The number of players an online session takes. */
    String mArena;           /*!< The arena of the game. */
    int mNumPads;            /*!< The number of local players, which GetNumPads() reports. */
    String mDemo;            /*!< The demo recording played, or empty. */
    int mWinSequence;        /*!< Whether the campaign win sequence runs. */
    int mUnlockAll;          /*!< Non-zero once UnlockAllSongs() ran, which lifts every lock. */
    int mWon;                /*!< The value of SetWon(). */
    float mEnergized;        /*!< The value of SetEnergized(). */
    int mFullMixBars;        /*!< The value of SetFullMixBars(). */
    int mBestStreak;         /*!< The value of SetBestStreak(). */
    float mProgress;         /*!< The value of SetProgress(). */
    int mReservedA4;         // +0xa4, cleared with the results and not yet identified.
    DataArray *mSongs;       /*!< The `songs` array of the "db" section, one entry per arena. */
    char *mRemixBuffer;      /*!< The packed data of the remix, which SetRemixBuffer() replaces. */
    RemixInfo mRemixInfo;    /*!< The description of the remix played, which SetRemix() copies. */
    int mRemixActive;        /*!< The flag SetRemixActive() sets, 1 at construction. */
    int mNetRemixEnded;      /*!< Whether the remix of an online game has ended. */
    float mSongTick;         /*!< The running world's position in ticks. */
    float mSongTime;         /*!< The running world's position on its song clock. */
    Scheduler mSfxScheduler; /*!< The scheduler the interface sounds play on. */
    GameOptions mOptions;    /*!< The settings of the game options screen. */

private:
    /**
     * Read an integer of a section of the configuration.
     *
     * @param pszName The name of the integer.
     * @param pData The section.
     * @param bFail Treat a missing integer as an error.
     * @return The integer, or 0 when it is missing.
     * @ghidraAddress NTSC-U/C: 0x0026ce20
     * @ghidraAddress PAL: 0x002769c0
     */
    static int FindInt(const char *pszName, DataArray *pData, bool bFail);

    /**
     * Read a symbol of a section of the configuration.
     *
     * @param pszName The name of the symbol.
     * @param pData The section.
     * @param bFail Treat a missing symbol as an error.
     * @return The symbol, or null when it is missing.
     * @ghidraAddress NTSC-U/C: 0x0026ce58
     * @ghidraAddress PAL: 0x002769f8
     */
    static const char *FindSymbol(const char *pszName, DataArray *pData, bool bFail);

    /**
     * Report the colour symbol of a player slot.
     *
     * @param nSlot The slot, 0 to 3.
     * @return `green`, `purple`, `red`, or `yellow`, or null with a warning for another slot.
     * @ghidraAddress NTSC-U/C: 0x0026ce90
     * @ghidraAddress PAL: 0x00276a30
     */
    static const char *GetSlotColor(int nSlot);

    /**
     * Report the skill levels an unlock query tests.
     *
     * @param nSkillLevel The skill level of the query, or kSkillAny.
     * @param bExact Whether only nSkillLevel counts, rather than every level up to it.
     * @param pnFirst Receives the first level tested.
     * @param pnEnd Receives the level after the last one tested.
     * @ghidraAddress NTSC-U/C: 0x0026cf20
     * @ghidraAddress PAL: 0x00276ac0
     */
    static void GetSkillRange(int nSkillLevel, bool bExact, int *pnFirst, int *pnEnd);

    /**
     * Rank the players by mScore, 0 for the best. Equal scores share a rank.
     *
     * @ghidraAddress NTSC-U/C: 0x0026ed00
     * @ghidraAddress PAL: 0x002788a0
     */
    void RankPlayers();
};

/**
 * The game database, GameDb::shared() as the unit's static initialiser stored it.
 *
 * @ghidraAddress NTSC-U/C: 0x00440d44
 */
extern GameDb *TheGameDb;
