#pragma once

/**
 * One item a song unlocked: a song, an avatar part, or an arena.
 *
 * The RTTI includes the name through the vectors of the class. The object is 8 bytes.
 * Campaign collects the items a result unlocks, and the metagame shows an unlock screen for
 * each.
 */
struct UnlockableItem {
    /**
     * Kinds of item, named from the unlock screens Metagame::QueueUnlocks() shows for them.
     *
     * Every other kind is an avatar part or an emblem. The names are inferred.
     */
    enum Kind {
        kKindArena = 0,       /*!< An arena. */
        kKindSong = 1,        /*!< A song. */
        kKindBossSong = 2,    /*!< The song of a boss arena. */
        kKindBonusSong = 3,   /*!< A song beyond the campaign. */
        kKindCampaignEnd = 5, /*!< The end of the campaign at a skill level. */
        kKindHead = 7,        /*!< A head. */
        kKindTorso = 8,       /*!< A torso. */
        kKindLowerBody = 9,   /*!< A pair of legs. */
        kKindArms = 10,       /*!< A pair of arms. */
        kKindHeadGear = 11,   /*!< A head gear. */
        kKindFaceGear = 12,   /*!< A face gear. */
        kKindEmblem = 13,     /*!< An emblem. */
        kKindPrefab = 14,     /*!< A prefabricated Freq. */
    };

    const char *mName;   /*!< The unlocked song, part, or arena, a symbol. */
    unsigned char mKind; /*!< One of Kind, which selects the unlock screen. */
};
