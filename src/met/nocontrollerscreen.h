#pragma once

#include "met/freqscreen.h"

/**
 * Pause menu of a song whose controller was disconnected, `no_controller` of the front-end
 * description.
 *
 * The RTTI records the class as deriving from FreqScreen. Only the members the metagame uses are
 * declared, and the routines of the class are not reconstructed.
 */
class NoControllerScreen : public FreqScreen {
public:
    int mPad; /*!< The controller that was disconnected. */
};
