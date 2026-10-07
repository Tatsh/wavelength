#pragma once

#include "game/playerprofile.h"
#include "met/freqmakerundoscreen.h"
#include "met/keyboarduser.h"
#include "os/string.h"

/**
 * The main screen of the Freq maker, where the player builds a Freq and gives it a name.
 *
 * The RTTI records the class as deriving from FreqMakerUndoScreen and from KeyboardUser at
 * `+0x70`. The object is 0x1c0 bytes. The metagame registers the class for the screen type
 * `f_maker_main_screen`. Only the members FreqConfirmScreen writes are declared, and the routines
 * of the class are not reconstructed.
 */
class FreqMakerMainScreen : public FreqMakerUndoScreen, public KeyboardUser {
public:
    int mReserved74[8]; // +0x74, not yet recovered.
    int mEditing;       /*!< 1 when the screen edits the player's Freq, 0 for a new one. +0x94 */
    int mReserved98[2]; // +0x98, not yet recovered.
    int mReservedA0;    // +0xa0, cleared before the screen opens. The purpose is not recovered.
    int mReservedA4[2]; // +0xa4, not yet recovered.
    String mFreqName;   /*!< The name of the Freq. +0xac */
    PlayerProfile mProfile; /*!< The profile the screen edits. +0xc0 */
};
