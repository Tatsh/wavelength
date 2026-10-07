#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"

/**
 * Result of NetLobby::CreateAccount().
 *
 * The RTTI includes the class name and records Message as the base. Only the members its readers
 * here use are declared.
 */
class CreateAccountResultMsg : public Message {
public:
    /** Values of mResult. */
    enum Result {
        kResultAlreadyExists = -81,      /*!< An account of the name exists. */
        kResultRegistrationFailed = -80, /*!< The server refused the registration. */
        kResultVulgarName = -76,         /*!< The name is not acceptable. */
        kResultSuccess = 0,              /*!< The account was created. */
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
     * @return g_nCreateAccountResultMsgType.
     */
    int Type() override {
        return g_nCreateAccountResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `CreateAccountResultMsg`.
     */
    const char *GetName() const override {
        return "CreateAccountResultMsg";
    }

    int mResult; /*!< One of Result, or another negative error. */
};
