#pragma once

#include <vector>

/**
 * Saved settings and progress of one player, among them the controller bindings.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Each player entry of GameDb
 * stores one at `+0x10`. This header declares only the members GameLogic uses.
 */
class PlayerProfile {
public:
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
};
