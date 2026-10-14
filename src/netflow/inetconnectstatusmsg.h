#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/string.h"

/**
 * Message that reports the progress of NetInet::Connect().
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d6480`. Only the
 * members its callers here use are declared.
 */
class InetConnectStatusMsg : public Message {
public:
    /**
     * Write the text of the step.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002689e0
     * @ghidraAddress PAL: 0x00272638
     */
    void PrintExtra(PrnStream &stream) const override;

    String mStatus; /*!< The localised text of the step under way. */
};
