#pragma once

#include "utl/MsgSource.h"

/**
 * Source of the controller messages.
 *
 * The RTTI records the class as deriving from MsgSource. The object is 0x10 bytes.
 */
class JoypadMsgSource : public MsgSource {
public:
    /**
     * Create the source. The constructor does not set #mInitialized.
     *
     * @ghidraAddress 0x1000ec40
     */
    JoypadMsgSource();

    /**
     * Release the source.
     *
     * @ghidraAddress 0x1000ec80
     */
    virtual ~JoypadMsgSource();

    bool mInitialized; /*!< Whether JoypadConfigInit() ran. */
};
