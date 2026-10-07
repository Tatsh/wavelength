#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"

/**
 * Result of NetLobby::ChangePassword().
 *
 * The RTTI includes the class name and records Message as the base. Only the members its readers
 * here use are declared.
 */
class ChangePasswordResultMsg : public Message {
public:
    /** Values of mResult. */
    enum Result {
        kResultUpdateFailed = -77, /*!< The server did not update the password. */
        kResultSuccess = 0,        /*!< The password was changed. */
    };

    /**
     * Produce a heap copy of this message.
     *
     * @return The copy.
     */
    Message *Clone() override;

    /**
     * Report this message's registered identity.
     *
     * @return g_nChangePasswordResultMsgType.
     */
    int Type() override {
        return g_nChangePasswordResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `ChangePasswordResultMsg`.
     */
    const char *GetName() const override {
        return "ChangePasswordResultMsg";
    }

    int mResult; /*!< One of Result, or another negative error. */
};
