#pragma once

#include "met/freqscreen.h"

/**
 * Base of FreqMakerMainScreen among the Freq maker screens.
 *
 * The RTTI records the class as deriving from FreqScreen. The class adds no member, and its
 * routines are not reconstructed.
 */
class FreqMakerUndoScreen : public FreqScreen {};
