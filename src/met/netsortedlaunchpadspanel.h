#pragma once

#include "met/freqpanel.h"

/**
 * Panel that lists the online sessions matching a search.
 *
 * The RTTI records the class as deriving from FreqPanel and NetFlashUpdate, and its vtable is at
 * `0x003cc9f0`. The FreqPanel part is at the start of the object. Only the routine NetSortedScreen
 * calls is declared, and the routines of the class are not reconstructed.
 */
class NetSortedLaunchpadsPanel : public FreqPanel {
public:
    /**
     * Set the search whose sessions the panel lists.
     *
     * @param pszArena The arena, or an empty string for any.
     * @param nRuleSet The game mode.
     * @param nSkillLevel The skill level, or -1 for any.
     * @ghidraAddress NTSC-U/C: 0x00176310
     * @ghidraAddress PAL: 0x00179770
     */
    void SetSearch(const char *pszArena, int nRuleSet, int nSkillLevel);
};
