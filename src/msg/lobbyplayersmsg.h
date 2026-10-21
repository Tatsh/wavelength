#pragma once

#include <list>

#include "msg/message.h"
#include "netflow/lobbyplayer.h"
#include "os/mem.h"

/**
 * Identity that LobbyPlayersMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b38
 * @ghidraAddress PAL: 0x0042fa08
 */
extern int g_nLobbyPlayersMsgType;

/**
 * Message that delivers the players of an online session the lobby lists.
 *
 * The RTTI records the class as deriving from Message. The vtable is at `0x003d6240`. The members
 * Clone(), Type(), and GetName() are inline.
 */
class LobbyPlayersMsg : public Message {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039e7a0
     * @ghidraAddress PAL: 0x0040d480
     */
    ~LobbyPlayersMsg() override = default;

    /** The value of mLaunchpad when the players are not those of a session. */
    static constexpr int kNoLaunchpad = -1;

    /**
     * Construct a message that points at the players.
     *
     * Inline.
     *
     * @param pPlayers The players. The caller retains them.
     * @param nLaunchpad The session, or kNoLaunchpad.
     * @param nHasPrevious Non-zero when more players precede a ranking page.
     */
    LobbyPlayersMsg(std::list<LobbyPlayer> *pPlayers, int nLaunchpad, int nHasPrevious)
        : mPlayers(pPlayers), mLaunchpad(nLaunchpad), mHasPrevious(nHasPrevious) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "LobbyPlayersMsg", 0);
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
     * Produce a heap copy of this message, which points at the same players.
     *
     * @return The copy.
     */
    Message *Clone() override {
        return new LobbyPlayersMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nLobbyPlayersMsgType.
     */
    int Type() override {
        return g_nLobbyPlayersMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `LobbyPlayersMsg`.
     */
    const char *GetName() const override {
        return "LobbyPlayersMsg";
    }

    /**
     * Write the session when there is one, and the players.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00268e20
     * @ghidraAddress PAL: 0x00272a28
     */
    void PrintExtra(PrnStream &stream) const override;

    /** @return Non-zero when more players precede a ranking page. */
    int GetHasPrevious() const {
        return mHasPrevious;
    }

    /** @return The index of the session in the lobby list. */
    int GetLaunchpad() const {
        return mLaunchpad;
    }

    /** @return The players, in 0x90-byte list nodes. */
    std::list<LobbyPlayer> *GetPlayers() const {
        return mPlayers;
    }

private:
    std::list<LobbyPlayer> *mPlayers; /*!< The players, in 0x90-byte list nodes. */
    int mLaunchpad;                   /*!< The index of the session in the lobby list. */
    int mHasPrevious;                 /*!< Non-zero when more players precede a ranking page. */
};
