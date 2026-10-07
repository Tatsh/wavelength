#pragma once

#include "met/freqscreen.h"
#include "os/string.h"

/**
 * Screen of an error message that leads on through its transition table.
 *
 * The RTTI records the class as deriving from FreqScreen. The front-end description's
 * `lobby_error` and `lpad_error` screens are ones. Only the members the metagame uses are
 * declared, and the routines of the class are not reconstructed.
 */
class TransitionErrorScreen : public FreqScreen {
public:
    String mMessage; /*!< The message the screen shows. */
};
