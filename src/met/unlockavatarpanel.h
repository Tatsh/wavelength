#pragma once

#include "game/avatarpartset.h"
#include "met/avatarpanel.h"
#include "script/dataarray.h"

/**
 * Avatar panel that shows a copy of a set of parts, such as an unlocked prefabricated Freq.
 *
 * The RTTI records the class as deriving from AvatarPanel. The object is 0x140 bytes and its
 * vtable is at `0x003cbec0`. The metagame registers the class for the panel type
 * `unlock_avatar_panel`. The destructor at `0x00355420` is compiler-generated and has no
 * declaration here.
 */
class UnlockAvatarPanel : public AvatarPanel {
public:
    /**
     * Construct the panel from its script description, showing no Freq.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x003551e8
     * @ghidraAddress PAL: 0x003c2498
     */
    UnlockAvatarPanel(DataArray *pData, const char *pszDir);

    /**
     * Create a panel from its script description.
     *
     * The metagame registers the routine for the entry type `unlock_avatar_panel`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x00355038
     * @ghidraAddress PAL: 0x003c22e8
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new UnlockAvatarPanel(pData, pszDir);
    }

    /**
     * Copy a set of parts and show the copy.
     *
     * @param parts The parts.
     * @ghidraAddress NTSC-U/C: 0x001975f8
     * @ghidraAddress PAL: 0x0019eb18
     */
    void SetParts(const AvatarPartSet &parts);

    /**
     * Apply the parts that have loaded to the Freq that shows.
     *
     * @return Whether no part is left pending, true when no Freq shows.
     * @ghidraAddress NTSC-U/C: 0x00197638
     * @ghidraAddress PAL: 0x0019eb58
     */
    bool UpdateAvatar();

    AvatarPartSet mParts; /*!< The copy SetParts() shows. +0x100 */
};
