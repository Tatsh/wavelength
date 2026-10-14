#pragma once

#include "os/prnstream.h"
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
    int mDefault;        /*!< Non-zero for the configuration the memory card marks as default. */
};

/**
 * Write a configuration as its identifier, description, and name, marking the default one.
 *
 * @param stream The stream.
 * @param config The configuration.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00267cc8
 * @ghidraAddress PAL: 0x002719a0
 */
PrnStream &operator<<(PrnStream &stream, const InetConfig &config);
