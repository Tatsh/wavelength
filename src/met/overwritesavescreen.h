#pragma once

#include "met/errorscreen.h"
#include "script/dataarray.h"

/**
 * Memory card screen whose save may replace a saved copy.
 *
 * The RTTI records the class as deriving from ErrorScreen. The class adds one member, and its
 * constructor is expanded in the constructor of each derived class.
 */
class OverwriteSaveScreen : public ErrorScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     */
    explicit OverwriteSaveScreen(DataArray *pData) : ErrorScreen(pData) {
    }

    int mOverwriteStatus; /*!< Non-zero when a saved copy of the same name may be replaced. */
};
