#pragma once

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netchatroominfo.h"
#include "os/mem.h"

/**
 * Result of NetLobby::Login().
 *
 * The RTTI includes the class name and records Message as the base.
 */
class LoginResultMsg : public Message {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039e2e0
     * @ghidraAddress PAL: 0x0040cfc0
     */
    ~LoginResultMsg() override = default;

    /** Values of mResult. */
    enum Result {
        kResultServerError = -86,     /*!< The login server failed. */
        kResultAccountNotFound = -85, /*!< The account does not exist. */
        kResultInvalidPassword = -84, /*!< The password is wrong. */
        kResultAlreadyLoggedIn = -83, /*!< The account is logged in elsewhere. */
        kResultStatsFailed = -78,     /*!< The server did not take the statistics. */
        kResultSuccess = 0,           /*!< The player is logged in. */
    };

    /**
     * Construct a failure, whose chatroom has the identifier -1.
     *
     * Inline. The texts and the counts of the chatroom are not set.
     *
     * @param nResult The error.
     */
    explicit LoginResultMsg(int nResult) : mResult(nResult) {
    }

    /**
     * Construct a result.
     *
     * Inline.
     *
     * @param nResult One of Result.
     * @param pszNews The news.
     * @param pszEula The licence agreement.
     * @param chatroom The chatroom the login places the player in.
     */
    LoginResultMsg(int nResult,
                   const char *pszNews,
                   const char *pszEula,
                   const NetChatroomInfo &chatroom)
        : mResult(nResult), mNews(pszNews), mEula(pszEula), mChatroom(chatroom) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "LoginResultMsg", 0);
    }

    /**
     * Release a message.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Produce a heap copy of this message.
     *
     * @return The copy.
     */
    Message *Clone() override {
        return new LoginResultMsg(*this);
    }

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

    /**
     * Write the result, and after a success the chatroom, the news, and the licence agreement.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268c28
     * @ghidraAddress PAL: 0x00272830
     */
    void PrintExtra(PrnStream &stream) const override;

    int mResult;               /*!< One of Result, or another negative error. */
    const char *mNews;         /*!< The news the welcome screen shows. */
    const char *mEula;         /*!< The licence agreement the EULA screen shows. */
    NetChatroomInfo mChatroom; /*!< The chatroom the login places the player in. */
};
