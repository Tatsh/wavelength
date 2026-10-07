#pragma once

#include "game/unlockableitem.h"
#include "met/freqscreen.h"

/**
 * Screen that lists the avatar parts and emblems a result unlocked, `unlock_parts` of the
 * front-end description.
 *
 * The RTTI records the class as deriving from FreqScreen. Only the members the metagame uses are
 * declared, and the routines of the class are not reconstructed.
 */
class PartUnlockScreen : public FreqScreen {
public:
    /**
     * Add an item to the list.
     *
     * @param item The item.
     * @return Whether the item is one the screen lists.
     * @ghidraAddress NTSC-U/C: 0x00197f00
     * @ghidraAddress PAL: 0x0019f420
     */
    bool AddItem(UnlockableItem item);
};
