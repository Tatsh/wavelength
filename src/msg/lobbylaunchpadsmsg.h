#pragma once

#include <list>

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netlaunchpadinfo.h"

/**
 * List of launchpads, the result of NetLobby::RequestLaunchpads().
 *
 * The RTTI includes the class name and records Message as the base. Only the members its readers
 * here use are declared.
 */
class LobbyLaunchpadsMsg : public Message {
public:
    /**
     * Produce a heap copy of this message.
     *
     * @return The copy.
     */
    Message *Clone() override;

    /**
     * Report this message's registered identity.
     *
     * @return g_nLobbyLaunchpadsMsgType.
     */
    int Type() override {
        return g_nLobbyLaunchpadsMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `LobbyLaunchpadsMsg`.
     */
    const char *GetName() const override {
        return "LobbyLaunchpadsMsg";
    }

    std::list<NetLaunchpadInfo> *mLaunchpads; /*!< The launchpads. */
    int mHasPrevious;                         /*!< Non-zero when launchpads precede the list. */
};
