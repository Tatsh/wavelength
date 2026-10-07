#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netchatroominfo.h"

/**
 * Result of NetLobby::Login().
 *
 * The RTTI includes the class name and records Message as the base.
 */
class LoginResultMsg : public Message {
public:
    /** Values of mResult. */
    enum Result {
        kResultServerError = -86,     /*!< The login server failed. */
        kResultAccountNotFound = -85, /*!< The account does not exist. */
        kResultInvalidPassword = -84, /*!< The password is wrong. */
        kResultAlreadyLoggedIn = -83, /*!< The account is logged in elsewhere. */
        kResultSuccess = 0,           /*!< The player is logged in. */
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
     * @return g_nLoginResultMsgType.
     */
    int Type() override {
        return g_nLoginResultMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `LoginResultMsg`.
     */
    const char *GetName() const override {
        return "LoginResultMsg";
    }

    int mResult;               /*!< One of Result, or another negative error. */
    const char *mNews;         /*!< The news the welcome screen shows. */
    const char *mEula;         /*!< The licence agreement the EULA screen shows. */
    NetChatroomInfo mChatroom; /*!< The chatroom the login places the player in. */
};
