#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"

/**
 * Message that reports that the online launchpad of this console ended before the game started.
 *
 * The RTTI includes the class name and records Message as the base. The payload is the reason, a
 * negative error code of the session or 0 for none. Only the payload is declared.
 */
class LaunchpadAbortedMsg : public Message {
public:
    /** Values of mReason. */
    enum Reason {
        kReasonNone = 0,           /*!< The launchpad did not end. */
        kReasonIncompatible = -66, /*!< The `net_lpad_incompatible` error. */
        kReasonDied = -65,         /*!< The `net_lpad_abort_died` error. */
        kReasonHost = -64,         /*!< The `net_lpad_abort_host` error. */
        kReasonBoot = -63,         /*!< The `net_lpad_abort_boot` error. */
        kReasonBad = -62,          /*!< The `net_lpad_abort_bad` error. */
    };

    int mReason; /*!< One of Reason, or another error code. */
};
