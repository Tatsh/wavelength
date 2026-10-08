#pragma once

#include "os/string.h"

/**
 * One network configuration of the console, as InetConfigsResultMsg lists them.
 *
 * The type has no RTTI of its own. The name comes from the type information the standard library
 * list of the type records. The object is 0x30 bytes.
 */
struct InetConfig {
    int mId;             /*!< The identifier NetInet::Connect() takes. */
    String mDescription; /*!< The description the configuration screen shows. */
    String mName;        /*!< The name on the button of the configuration. */
    int mReserved2C;     // +0x2c, not yet recovered.
};
