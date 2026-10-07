#pragma once

#include <list>

#include "msg/message.h"
#include "netflow/inetconfig.h"

/**
 * Identity that InetConfigsResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b04
 */
extern int g_nInetConfigsResultMsgType;

/**
 * Message that reports the network configurations NetInet::RequestConfigs() found.
 *
 * The RTTI records the class as deriving from Message. Only the members its callers here use are
 * declared.
 */
class InetConfigsResultMsg : public Message {
public:
    /**
     * Release the message and its configurations.
     *
     * @ghidraAddress NTSC-U/C: 0x0039dc98
     */
    ~InetConfigsResultMsg() override;

    /**
     * Produce a heap copy of this message.
     *
     * @return The copy.
     * @ghidraAddress NTSC-U/C: 0x0039dd70
     */
    Message *Clone() override;

    /**
     * Report this message's registered identity.
     *
     * @return g_nInetConfigsResultMsgType.
     * @ghidraAddress NTSC-U/C: 0x0039ddb0
     */
    int Type() override;

    /**
     * Report this message's class name.
     *
     * @return The literal `InetConfigsResultMsg`.
     * @ghidraAddress NTSC-U/C: 0x0039ddc0
     */
    const char *GetName() const override;

    int mResult;                    /*!< The outcome, negative when the search failed. +0x04 */
    std::list<InetConfig> mConfigs; /*!< The configurations found. +0x08 */
};
