#pragma once

#include <vector>

#include "game/avatarpartset.h"
#include "game/inputmap.h"
#include "game/songstats.h"
#include "game/unlockableitem.h"
#include "os/binstream.h"
#include "os/datetime.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * Saved settings and progress of one player, among them the controller bindings.
 *
 * The class is not polymorphic. The name comes from the RTTI of the vectors that store it. Each
 * player entry of GameDb stores one at `+0x10`. The object is 0xf8 bytes.
 */
class Campaign {
public:
    /** Bits of mFlags. The names are inferred. */
    enum Flag {
        kFlagAutocatcher = 0,  /*!< The `autocatcher` power-up was granted. */
        kFlagMultiplier = 1,   /*!< The `multiplier` power-up was granted. */
        kFlagSlowdown = 2,     /*!< The `slowdown` power-up was granted. */
        kFlagFreestyle = 3,    /*!< The `freestyle` power-up was granted. */
        kFlagFreestyleTip = 4, /*!< The tip about the freestyle lap has shown. */
        kFlagNone = 5,         /*!< The bit GetPowerupBit() reports for an unknown power-up. */
    };

    /**
     * Values of GetMedal() and GetMedalForScore(), one more than the goals of the `score_goal` of
     * the song a score passes. The names are inferred.
     */
    enum Medal {
        kMedalNone = 0,     /*!< The song was not finished, or it has no goals. */
        kMedalFinished = 1, /*!< The song was finished below the first goal. */
        kMedalFirst = 2,    /*!< The score passed the first goal. */
        kMedalSecond = 3,   /*!< The score passed the second goal. */
        kMedalThird = 4,    /*!< The score passed the third goal. */
    };

    /**
     * Report whether a song is the one song of its campaign tier the player has not yet won.
     *
     * Bonus songs and secret songs do not count. The name is inferred.
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
     * @return The bindings.
     * @ghidraAddress NTSC-U/C: 0x0027afc8
     * @ghidraAddress PAL: 0x00284a58
     */
    InputMap *GetInputMap();

    /**
     * Replace the controller bindings and mark the profile modified.
     *
     * @param pMap The bindings to copy.
     * @ghidraAddress NTSC-U/C: 0x0027afd8
     * @ghidraAddress PAL: 0x00284a68
     */
    void SetInputMap(const InputMap *pMap);

    /**
     * Report whether the tip about the freestyle lap has shown. The name is inferred.
     *
     * @return Non-zero once the tip has shown.
     * @ghidraAddress NTSC-U/C: 0x0027aea8
     * @ghidraAddress PAL: 0x00284938
     */
    int HasSeenFreestyleTip();

    /**
     * Record that the tip about the freestyle lap has shown. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0027aec8
     * @ghidraAddress PAL: 0x00284958
     */
    void SetSeenFreestyleTip();

    /**
     * Report the first song of the unlocked arenas the player has not finished at a skill level.
     *
     * Bonus songs and secret songs do not count.
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
     * Only the songs of type SongEntry::kTypeNormal available at the skill level count.
     *
     * @param nSkillLevel The skill level.
     * @param pszGroup The group, the name of an arena.
     * @param pCleared Receives one flag for each song of the group.
     * @ghidraAddress NTSC-U/C: 0x00279cb8
     * @ghidraAddress PAL: 0x00283748
     */
    void GetClearedSongs(int nSkillLevel, const char *pszGroup, std::vector<bool> *pCleared);

    /**
     * Report whether the player has unlocked an item at a skill level.
     *
     * An item unlocked for every skill level counts at each.
     *
     * @param pszItem The item.
     * @param nSkillLevel The skill level, or GameDb::kSkillAny for any.
     * @return Whether the item is unlocked.
     * @ghidraAddress NTSC-U/C: 0x0027aee8
     * @ghidraAddress PAL: 0x00284978
     */
    bool IsUnlocked(const char *pszItem, int nSkillLevel);

    /**
     * List the choices of an avatar part the player may use.
     *
     * Every choice qualifies while g_dwUnlockAllParts is set. The name is inferred.
     *
     * @param nPart One of AvatarPartSet::Part.
     * @param pTypes Receives the symbols of the choices.
     * @ghidraAddress NTSC-U/C: 0x0027aaf0
     * @ghidraAddress PAL: 0x00284580
     */
    void GetUnlockedParts(int nPart, std::vector<const char *> *pTypes);

    /**
     * List the emblems the player may use.
     *
     * Every emblem qualifies while g_dwUnlockAllParts is set. The name is inferred.
     *
     * @param pTypes Receives the symbols of the emblems.
     * @ghidraAddress NTSC-U/C: 0x0027ab48
     * @ghidraAddress PAL: 0x002845d8
     */
    void GetUnlockedEmblems(std::vector<const char *> *pTypes);

    /**
     * Remove the items the player has not unlocked at any skill level from a list.
     *
     * The name is inferred.
     *
     * @param pItems The symbols of the items.
     * @ghidraAddress NTSC-U/C: 0x0027ace8
     * @ghidraAddress PAL: 0x00284778
     */
    void RemoveLockedItems(std::vector<const char *> *pItems);

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
     * Count every avatar part and emblem as unlocked for every profile.
     *
     * @ghidraAddress NTSC-U/C: 0x0027b040
     * @ghidraAddress PAL: 0x00284ad0
     */
    static void UnlockAllParts();

    /**
     * Count every song as unlocked for every profile, by setting g_dwUnlockAllSongs.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0027b030
     * @ghidraAddress PAL: 0x00284ac0
     */
    static void UnlockAllSongs();

    /**
     * Construct a profile with the default settings and the unlocks of the `always` entries.
     *
     * @ghidraAddress NTSC-U/C: 0x00276e68
     * @ghidraAddress PAL: 0x002808f8
     */
    Campaign();

    /**
     * Copy a profile.
     *
     * @param other The profile to copy.
     * @ghidraAddress NTSC-U/C: 0x002771d8
     * @ghidraAddress PAL: 0x00280c68
     */
    Campaign(const Campaign &other);

    /**
     * Destroy the profile.
     *
     * @ghidraAddress NTSC-U/C: 0x002772d8
     * @ghidraAddress PAL: 0x00280d68
     */
    ~Campaign();

    /**
     * Copy another profile into this one.
     *
     * @param other The profile to copy.
     * @return The profile.
     * @ghidraAddress NTSC-U/C: 0x00277498
     * @ghidraAddress PAL: 0x00280f28
     */
    Campaign &operator=(const Campaign &other);

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
     * Report whether the player has beaten the game at a skill level, the symbol `beat_game` among
     * the unlocks of that level. Every level counts while the unlock-all setting is on. The name
     * is inferred.
     *
     * @param nSkillLevel The skill level.
     * @return Whether the game was beaten.
     * @ghidraAddress NTSC-U/C: 0x00279888
     * @ghidraAddress PAL: 0x00283318
     */
    bool HasBeatenGame(int nSkillLevel);

    /**
     * Report the highest skill level at which the player has beaten the game. The name is
     * inferred.
     *
     * @return The skill level, or -1 when the game was not beaten.
     * @ghidraAddress NTSC-U/C: 0x00279908
     * @ghidraAddress PAL: 0x00283398
     */
    int GetHighestBeatenSkillLevel();

    /**
     * Copy the record of a song at a skill level.
     *
     * Without a record, the copy is a cleared record of the song and the skill level.
     *
     * @param pszSong The song.
     * @param nSkillLevel The skill level.
     * @param pRecord The record to fill.
     * @ghidraAddress NTSC-U/C: 0x00279a48
     * @ghidraAddress PAL: 0x002834d8
     */
    void GetSongStats(const char *pszSong, int nSkillLevel, SongStats *pRecord);

    /**
     * Record the result of a song and collect the items it unlocks.
     *
     * While the unlock-all-songs cheat is on, the result is not recorded and the list is emptied.
     *
     * @param pRecord The result.
     * @param pUnlocks Receives the items the result unlocks.
     * @ghidraAddress NTSC-U/C: 0x002778e8
     * @ghidraAddress PAL: 0x00281378
     */
    void RecordResult(const SongStats *pRecord, std::vector<UnlockableItem> *pUnlocks);

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
     * @return One of Medal.
     * @ghidraAddress NTSC-U/C: 0x0027ab98
     * @ghidraAddress PAL: 0x00284628
     */
    int GetMedal(const char *pszSong, int nSkillLevel);

    /**
     * Report the medal a score earns on a song at a skill level. The name is inferred.
     *
     * @param pszSong The song.
     * @param nSkillLevel The skill level.
     * @param nScore The score.
     * @param nPercent The share of the song played, in percent. Only 100 earns a medal.
     * @return One of Medal.
     * @ghidraAddress NTSC-U/C: 0x0027ac08
     * @ghidraAddress PAL: 0x00284698
     */
    int GetMedalForScore(const char *pszSong, int nSkillLevel, int nScore, int nPercent);

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

    /**
     * Write the profile, at version 6.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x0027b058
     * @ghidraAddress PAL: 0x00284ae8
     */
    void Save(BinStream &stream) const;

    /**
     * Read a profile Save() wrote, and rebuild the unlocks from the records.
     *
     * Versions before 4 take the bindings of the `input_map` of the "db" section, versions before
     * 5 clear mFlags, and versions before 6 reset the last unlock of each kind.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x0027b1e8
     * @ghidraAddress PAL: 0x00284c48
     */
    void Load(BinStream &stream);

    /** @return The online password, or empty when it is not saved. */
    const String &GetPassword() const {
        return mPassword;
    }

    /** @param pszPassword The online password, or empty when it is not saved. */
    void SetPassword(const char *pszPassword) {
        mPassword = pszPassword;
    }

    /** @return Non-zero when the player may not rename the Freq. */
    int GetNameLocked() const {
        return mNameLocked;
    }

    /** @param nNameLocked Non-zero when the player may not rename the Freq. */
    void SetNameLocked(int nNameLocked) {
        mNameLocked = nNameLocked;
    }

    /** @return The player's name. */
    const String &GetName() const {
        return mName;
    }

    /** @param pszName The player's name. */
    void SetName(const char *pszName) {
        mName = pszName;
    }

    /** @return Non-zero for a player's own profile, not a default one. */
    int GetCustom() const {
        return mCustom;
    }

    /** @param nCustom Non-zero for a player's own profile, not a default one. */
    void SetCustom(int nCustom) {
        mCustom = nCustom;
    }

    /** @return The date the Freq was created. */
    const DateTime &GetBorn() const {
        return mBorn;
    }

    /** @return The date the Freq was created. */
    DateTime &GetBorn() {
        return mBorn;
    }

    /** @return The Freq's parts and colours. */
    const AvatarPartSet &GetAvatar() const {
        return mAvatar;
    }

    /** @return The Freq's parts and colours. */
    AvatarPartSet &GetAvatar() {
        return mAvatar;
    }

private:
    // Add a result to the records, keeping the best of each statistic.
    // NTSC-U/C: 0x00277590, PAL: 0x00281020
    void AddRecord(const SongStats *pRecord);

    // Collect the unlocks of every `campaign` entry the records now pass at a skill level.
    // NTSC-U/C: 0x002777f8, PAL: 0x00281288
    void UpdateUnlocks(std::vector<UnlockableItem> *pUnlocks, int nSkillLevel);

    // Apply one `campaign` entry once its condition passes.
    // NTSC-U/C: 0x00277980, PAL: 0x00281410
    void ProcessEntry(DataArray *pEntry,
                      int nSkillLevel,
                      int nEntry,
                      std::vector<UnlockableItem> *pUnlocks);

    // Evaluate a `beat_num`, `beat_songs`, `beat_score`, or `always` condition.
    // NTSC-U/C: 0x00277ae8, PAL: 0x00281578
    bool CheckCondition(DataArray *pCondition, int nSkillLevel);

    // Report whether a number of songs other than the tutorial were finished.
    // NTSC-U/C: 0x00277ca8, PAL: 0x00281738
    bool CheckBeatNum(int nCount, int nSkillLevel);

    // List the songs a `filter`, `all_except`, or `songs` clause selects.
    // NTSC-U/C: 0x00277d78, PAL: 0x00281808
    void ParseFilter(DataArray *pFilter, int nSkillLevel, std::vector<const char *> *pSongs);

    // List the available songs of an arena, or of `all`, of one type, or of every type for 5.
    // NTSC-U/C: 0x00277f40, PAL: 0x002819d0
    void BuildFiltered(int nType,
                       const char *pszArena,
                       int nSkillLevel,
                       std::vector<const char *> *pSongs);

    // List the available songs of an arena, or of `all`, not of one type.
    // NTSC-U/C: 0x00278218, PAL: 0x00281ca8
    void
    ExceptType(int nType, const char *pszArena, int nSkillLevel, std::vector<const char *> *pSongs);

    // List the available songs of an arena, or of `all`, of neither of two types.
    // NTSC-U/C: 0x002784f8, PAL: 0x00281f88
    void ExceptTwoTypes(int nType,
                        int nOtherType,
                        const char *pszArena,
                        int nSkillLevel,
                        std::vector<const char *> *pSongs);

    // List the available songs a `songs` clause specifies.
    // NTSC-U/C: 0x002787f0, PAL: 0x00282280
    void FromList(DataArray *pList, int nSkillLevel, std::vector<const char *> *pSongs);

    // Report whether every song was finished.
    // NTSC-U/C: 0x00278a58, PAL: 0x002824e8
    bool CheckAllBeaten(const std::vector<const char *> *pSongs, int nSkillLevel);

    // Report whether the best scores of the songs add up to a total.
    // NTSC-U/C: 0x00278b10, PAL: 0x002825a0
    bool CheckScoreTotal(const std::vector<const char *> *pSongs, int nSkillLevel, int nTotal);

    // Unlock an item once at a skill level, or at every level for GameDb::kSkillAny.
    // NTSC-U/C: 0x00278bb8, PAL: 0x00282648
    void RecordUnlock(int nKind,
                      const char *pszItem,
                      int nSkillLevel,
                      std::vector<UnlockableItem> *pUnlocks);

    // Apply an `unlock` or `skill_unlock` clause.
    // NTSC-U/C: 0x00278f50, PAL: 0x002829e0
    void ApplyUnlockNode(DataArray *pNode, int nSkillLevel, std::vector<UnlockableItem> *pUnlocks);

    // Apply a `prefab_unlock` clause.
    // NTSC-U/C: 0x00279090, PAL: 0x00282b20
    void PrefabUnlock(DataArray *pNode, std::vector<UnlockableItem> *pUnlocks);

    // Unlock the parts of a prefabricated Freq of the "metagame" section.
    // NTSC-U/C: 0x002791a0, PAL: 0x00282c30
    void ExpandPrefab(const char *pszPrefab, std::vector<UnlockableItem> *pUnlocks);

    // Unlock one part of a prefabricated Freq, and the emblem of a torso.
    // NTSC-U/C: 0x00279330, PAL: 0x00282dc0
    void RecordPrefabPart(DataArray *pPart, std::vector<UnlockableItem> *pUnlocks);

    // NTSC-U/C: 0x0027ad70, PAL: 0x00284800
    int GetFlagBit(int nFlag);

    // NTSC-U/C: 0x0027ad80, PAL: 0x00284810
    void SetFlagBit(int nFlag);

    // Report the Flag of a power-up.
    // NTSC-U/C: 0x0027ad98, PAL: 0x00284828
    int GetPowerupBit(const char *pszPowerup);

    std::vector<const char *> mUnlocks; /*!< The items unlocked at every skill level. */
    /** The items unlocked at each skill level. */
    std::vector<std::vector<const char *>> mSkillUnlocks;
    /** One bit for each skill level at which each `campaign` entry has passed. */
    std::vector<unsigned char> mPassedEntries;
    String mName;                    /*!< The player's name. */
    DateTime mBorn;                  /*!< The date the Freq was created. */
    String mPassword;                /*!< The online password, or empty when it is not saved. */
    int mNameLocked;                 /*!< Non-zero when the player may not rename the Freq. */
    std::vector<SongStats> mRecords; /*!< The best result of each song at each skill level. */
    AvatarPartSet mAvatar;           /*!< The Freq's parts and colours. */
    int mModified;                   /*!< Non-zero when the profile changed since it was saved. */
    int mCustom;                     /*!< Non-zero for a player's own profile, not a default one. */
    const char *mPendingPowerup;     /*!< The power-up TakePendingPowerup() grants, or null. */
    /** The last item unlocked of each UnlockableItem::Kind. */
    std::vector<const char *> mLastUnlocks;
    unsigned char mFlags; /*!< Bits of Flag. */
    int mSoloOption;      /*!< The value GetSoloOption() reports. */
    int mOnlineOption;    /*!< The value GetOnlineOption() reports. */
    InputMap mInputMap;   /*!< The controller bindings. */
};

/**
 * Write a profile to a stream.
 *
 * @param stream The stream.
 * @param profile The profile.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027b608
 * @ghidraAddress PAL: 0x00285038
 */
BinStream &operator<<(BinStream &stream, const Campaign &profile);

/**
 * Read a profile from a stream.
 *
 * @param stream The stream.
 * @param profile Receives the profile.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027b638
 * @ghidraAddress PAL: 0x00285068
 */
BinStream &operator>>(BinStream &stream, Campaign &profile);

/**
 * Non-zero when every song counts as unlocked and finished.
 *
 * @ghidraAddress NTSC-U/C: 0x003b1504
 * @ghidraAddress PAL: 0x004303d4
 */
extern unsigned int g_dwUnlockAllSongs;

/**
 * Non-zero when every avatar part and emblem counts as unlocked.
 *
 * @ghidraAddress NTSC-U/C: 0x003b1508
 * @ghidraAddress PAL: 0x004303d8
 */
extern unsigned int g_dwUnlockAllParts;
