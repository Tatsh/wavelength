#pragma once

#include <vector>

#include "game/inputmap.h"
#include "game/songrecord.h"
#include "os/string.h"

/**
 * Saved settings and progress of one player, among them the controller bindings.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Each player entry of GameDb
 * stores one at `+0x10`. The object is 0xf8 bytes. This header declares only the members its
 * callers here use.
 */
class PlayerProfile {
public:
    /**
     * Report whether a song is the one song of its campaign tier the player has not yet won.
     *
     * The name is inferred.
     *
     * @param pszSong The song.
     * @param nSkillLevel The skill level.
     * @return True when winning the song completes the tier.
     * @ghidraAddress NTSC-U/C: 0x00279660
     * @ghidraAddress PAL: 0x002830f0
     */
    bool CompletesTier(const char *pszSong, int nSkillLevel);

    /**
     * Report the controller bindings.
     *
     * @return The bindings, which the profile stores at `+0xd8`.
     * @ghidraAddress NTSC-U/C: 0x0027afc8
     * @ghidraAddress PAL: 0x00284a58
     */
    InputMap *GetInputMap();

    /**
     * Report the first song of a skill level the player has not finished.
     *
     * @param nSkillLevel The skill level.
     * @return The song, or null when every song is finished.
     * @ghidraAddress NTSC-U/C: 0x00279450
     * @ghidraAddress PAL: 0x00282ee0
     */
    const char *FindFirstUnfinishedSong(int nSkillLevel);

    /**
     * Report which songs of a group the player has cleared at a skill level.
     *
     * @param nSkillLevel The skill level.
     * @param nGroup The group.
     * @param pCleared Receives one flag for each song of the group.
     * @ghidraAddress NTSC-U/C: 0x00279cb8
     * @ghidraAddress PAL: 0x00283748
     */
    void GetClearedSongs(int nSkillLevel, int nGroup, std::vector<bool> *pCleared);

    /**
     * Report whether the player has unlocked an item at a skill level.
     *
     * @param pszItem The item.
     * @param nSkillLevel The skill level.
     * @return Whether the item is unlocked.
     * @ghidraAddress NTSC-U/C: 0x0027aee8
     * @ghidraAddress PAL: 0x00284978
     */
    bool IsUnlocked(const char *pszItem, int nSkillLevel);

    /**
     * Report the display option the player chose for the solo game.
     *
     * @return The option.
     * @ghidraAddress NTSC-U/C: 0x0027afc0
     * @ghidraAddress PAL: 0x00284a50
     */
    int GetSoloOption() const;

    /**
     * Report the display option the player chose for the online game.
     *
     * @return The option.
     * @ghidraAddress NTSC-U/C: 0x0027afd0
     * @ghidraAddress PAL: 0x00284a60
     */
    int GetOnlineOption() const;

    /**
     * Record the display option for the solo game and mark the profile changed.
     *
     * @param nOption The option.
     * @ghidraAddress NTSC-U/C: 0x0027b008
     * @ghidraAddress PAL: 0x00284a98
     */
    void SetSoloOption(int nOption);

    /**
     * Record the display option for the online game.
     *
     * @param nOption The option.
     * @ghidraAddress NTSC-U/C: 0x0027b018
     * @ghidraAddress PAL: 0x00284aa8
     */
    void SetOnlineOption(int nOption);

    /**
     * Construct a profile with the default settings.
     *
     * @ghidraAddress NTSC-U/C: 0x00276e68
     * @ghidraAddress PAL: 0x002808f8
     */
    PlayerProfile();

    /**
     * Copy a profile.
     *
     * @param other The profile to copy.
     * @ghidraAddress NTSC-U/C: 0x002771d8
     * @ghidraAddress PAL: 0x00280c68
     */
    PlayerProfile(const PlayerProfile &other);

    /**
     * Destroy the profile.
     *
     * @ghidraAddress NTSC-U/C: 0x002772d8
     * @ghidraAddress PAL: 0x00280d68
     */
    ~PlayerProfile();

    /**
     * Copy another profile into this one.
     *
     * @param other The profile to copy.
     * @return The profile.
     * @ghidraAddress NTSC-U/C: 0x00277498
     * @ghidraAddress PAL: 0x00280f28
     */
    PlayerProfile &operator=(const PlayerProfile &other);

    /**
     * Report whether a song was finished at a skill level.
     *
     * A song counts when its record shows the whole song played. Skill level 4 accepts a finished
     * record at any skill level. Every song counts while the unlock-all setting is on. The name is
     * inferred.
     *
     * @param pszSong The song.
     * @param nSkillLevel The skill level, or 4 for any.
     * @return Whether the song was finished.
     * @ghidraAddress NTSC-U/C: 0x00279960
     * @ghidraAddress PAL: 0x002833f0
     */
    bool IsSongFinished(const char *pszSong, int nSkillLevel);

    /**
     * Copy the record of a song at a skill level. The record does not change when there is none.
     *
     * @param pszSong The song.
     * @param nSkillLevel The skill level.
     * @param pRecord The record to fill.
     * @ghidraAddress NTSC-U/C: 0x00279a48
     * @ghidraAddress PAL: 0x002834d8
     */
    void GetSongRecord(const char *pszSong, int nSkillLevel, SongRecord *pRecord);

    /**
     * Report the sum of the best scores of the songs of an arena. The name is inferred.
     *
     * @param pszArena The arena.
     * @param nSkillLevel The skill level.
     * @return The sum.
     * @ghidraAddress NTSC-U/C: 0x00279b08
     * @ghidraAddress PAL: 0x00283598
     */
    int GetArenaScore(const char *pszArena, int nSkillLevel);

    /**
     * Report the arena score to beat, from the `arena_scores` of the "db" section.
     *
     * The profile itself is not read. The name is inferred.
     *
     * @param pszArena The arena.
     * @param nSkillLevel The skill level.
     * @return The score.
     * @ghidraAddress NTSC-U/C: 0x00279c40
     * @ghidraAddress PAL: 0x002836d0
     */
    int GetArenaScoreToBeat(const char *pszArena, int nSkillLevel);

    /**
     * Report the medal of a song at a skill level. The name is inferred.
     *
     * @param pszSong The song.
     * @param nSkillLevel The skill level.
     * @return The medal.
     * @ghidraAddress NTSC-U/C: 0x0027ab98
     * @ghidraAddress PAL: 0x00284628
     */
    int GetMedal(const char *pszSong, int nSkillLevel);

    /**
     * Grant the power-up of mPendingPowerup once it is unlocked at a skill level, and clear it.
     *
     * The name is inferred.
     *
     * @param nSkillLevel The skill level.
     * @return The power-up granted, or null.
     * @ghidraAddress NTSC-U/C: 0x0027ae38
     * @ghidraAddress PAL: 0x002848c8
     */
    const char *TakePendingPowerup(int nSkillLevel);

    /**
     * Report mModified.
     *
     * @return Non-zero when the profile changed since it was saved.
     * @ghidraAddress NTSC-U/C: 0x0027b020
     * @ghidraAddress PAL: 0x00284ab0
     */
    int IsModified() const;

    /**
     * Set mModified.
     *
     * @param nModified Non-zero when the profile changed since it was saved.
     * @ghidraAddress NTSC-U/C: 0x0027b028
     * @ghidraAddress PAL: 0x00284ab8
     */
    void SetModified(int nModified);

    int mReserved00[12]; // +0x00, not yet recovered.
    String mName;        /*!< The player's name. +0x30 */
    int mReserved44[27]; // +0x44, not yet recovered.
    int mModified;       /*!< Non-zero when the profile changed since it was saved. +0xb0 */
    int mCustom; /*!< Non-zero for a player's own profile rather than a default one. +0xb4 */
    const char *mPendingPowerup; /*!< The power-up TakePendingPowerup() grants, or null. +0xb8 */
    int mReservedBC[5];          // +0xbc, not yet recovered.
    int mSoloOption;             /*!< The value GetSoloOption() reports. +0xd0 */
    int mOnlineOption;           /*!< The value GetOnlineOption() reports. +0xd4 */
    int mReservedD8[8];          // +0xd8, not yet recovered.
};
