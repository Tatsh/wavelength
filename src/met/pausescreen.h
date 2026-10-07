#pragma once

#include "met/freqscreen.h"

/**
 * Pause menu of a song, such as `s_pause` and `m_g_pause` of the front-end description.
 *
 * The RTTI records the class as deriving from FreqScreen. Only the members the metagame uses are
 * declared, and the routines of the class are not reconstructed.
 */
class PauseScreen : public FreqScreen {
public:
    int mPad; /*!< The controller the menu listens to, or -1 for every controller. */
};
