#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"

/**
 * Freq maker screen whose changes the player can abandon.
 *
 * The RTTI records the class as deriving from FreqScreen. The class adds no member. Its
 * constructor is expanded in each derived constructor, and no vtable for the class is emitted.
 * FreqMakerErrorScreen calls Undo() when the player confirms leaving without the changes.
 */
class FreqMakerUndoScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     */
    explicit FreqMakerUndoScreen(DataArray *pData) : FreqScreen(pData) {
    }

    /** Abandon the changes the player made on the screen. */
    virtual void Undo() = 0;
};
