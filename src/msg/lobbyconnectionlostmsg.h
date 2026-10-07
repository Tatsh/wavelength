#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"

/**
 * Message that reports that the connection to the online lobby was lost.
 *
 * The RTTI includes the class name and records Message as the base. The object is 8 bytes and its
 * vtable is at `0x003d4e30`. The payload is the error code of the network library. The destructor
 * at `0x0039afb8` is compiler-generated and has no declaration here.
 */
class LobbyConnectionLostMsg : public Message {
public:
    /** The error code of a connection to the internet that went down. */
    static constexpr int kInternetDown = -91;

    /**
     * Produce a heap copy of this message.
     *
     * @return The copy.
     * @ghidraAddress NTSC-U/C: 0x0039b060
     * @ghidraAddress PAL: 0x00409ca8
     */
    Message *Clone() override;

    /**
     * Report this message's registered identity.
     *
     * @return g_nLobbyConnectionLostMsgType.
     * @ghidraAddress NTSC-U/C: 0x0039b0b0
     * @ghidraAddress PAL: 0x00409cf8
     */
    int Type() override;

    /**
     * Report this message's class name.
     *
     * @return The literal `LobbyConnectionLostMsg`.
     * @ghidraAddress NTSC-U/C: 0x0039b0c0
     * @ghidraAddress PAL: 0x00409d08
     */
    const char *GetName() const override;

    int mError; /*!< The error code of the network library. */
};
