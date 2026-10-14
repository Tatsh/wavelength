#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"

/**
 * Message that reports the outcome of NetInet::LookupHost().
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d63f0`.
 */
class InetDNSLookupResultMsg : public Message {
public:
    /**
     * Write the result and the address.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268aa0
     * @ghidraAddress PAL: 0x002726a8
     */
    void PrintExtra(PrnStream &stream) const override;

    int mResult;          /*!< The outcome, zero on success. */
    const char *mAddress; /*!< The address of the host, as text. */
};
