#pragma once

#include "met/freqscreen.h"

/**
 * Screen that announces the song of the boss arena a result unlocked, `unlock_boss` of the
 * front-end description.
 *
 * The RTTI records the class as deriving from FreqScreen. Only the members the metagame uses are
 * declared, and the routines of the class are not reconstructed.
 */
class BossUnlockScreen : public FreqScreen {
public:
    int mReserved70[2]; // +0x70, not yet recovered.
    const char *mSong;  /*!< The unlocked song, a symbol. */
};
