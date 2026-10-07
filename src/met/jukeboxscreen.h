#pragma once

#include "met/freqscreen.h"

/**
 * The jukebox screen. It plays the songs of the game as a playlist.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x90 bytes and its vtable
 * is at `0x003d02b0`. The metagame registers the class for the screen type `jbox_screen`. Only the
 * members ModeScreen reads are declared, and the routines of the class are not reconstructed.
 */
class JukeboxScreen : public FreqScreen {
public:
    int mReserved70[4]; // +0x70, the playlist. The element type is not yet recovered.
    int mShowAllSongs;  /*!< Non-zero when every song is listed, unlocked or not. +0x80 */
    int mReserved84[3]; // +0x84, not yet recovered.
};
