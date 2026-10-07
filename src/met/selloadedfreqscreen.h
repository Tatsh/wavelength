#pragma once

#include <vector>

#include "game/playerprofile.h"
#include "met/freqscreen.h"
#include "script/dataarray.h"

/**
 * The screen that lists the Freqs of the memory card to load one.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x180 bytes and its
 * vtable is at `0x003cede0`. The metagame registers the class for the screen type
 * `sel_loaded_f_screen`, and the front-end description's `f_load` screen is one. Only the members
 * FreqConfirmScreen uses are declared, and the routines of the class are not reconstructed.
 */
class SelLoadedFreqScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no Freqs.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018eb38
     * @ghidraAddress PAL: 0x00195e50
     */
    explicit SelLoadedFreqScreen(DataArray *pData);

    /**
     * Replace the listed Freqs.
     *
     * @param profiles The Freqs.
     * @ghidraAddress NTSC-U/C: 0x0035d730
     */
    virtual void SetProfiles(const std::vector<PlayerProfile> &profiles);

    std::vector<PlayerProfile> mProfiles; /*!< The Freqs of the memory card. +0x70 */
    PlayerProfile mProfile;               /*!< The profile the player had before loading. +0x80 */
};
