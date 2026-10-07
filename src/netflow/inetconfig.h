#pragma once

#include "os/string.h"

/**
 * One network configuration of the console, as InetConfigsResultMsg lists them.
 *
 * The type has no RTTI of its own. The name comes from the type information the standard library
 * list of the type records. The object is 0x30 bytes. Its members are not yet recovered.
 */
struct InetConfig {
    int mReserved00;    // +0x00, not yet recovered.
    String mReserved04; // +0x04, not yet recovered.
    String mReserved18; // +0x18, not yet recovered.
    int mReserved2C;    // +0x2c, not yet recovered.
};
