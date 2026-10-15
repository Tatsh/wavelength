#pragma once

#include <librtmedius.h>

#include "app/msgsink.h"
#include "msg/message.h"
#include "os/mem.h"

/**
 * Identity that ChatMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b88
 * @ghidraAddress PAL: 0x0042fa58
 */
extern int g_nChatMsgType;

/**
 * Message with one line of chat between the consoles of an online session or lobby.
 *
 * The RTTI records the class as deriving from Message, and its vtable is at `0x003d58c8`. The class
 * also keeps the sink of each channel, and joins the lines that arrive from the lobby server in
 * parts.
 */
class ChatMsg : public Message {
public:
    /**
     * Construct a line.
     *
     * Inline.
     *
     * @param nAccountId The sender's account identifier.
     * @param nPlayer The sender's identifier in the session, or -2 for none.
     * @param pszText The text.
     * @param pszSender The sender's name.
     */
    ChatMsg(int nAccountId, int nPlayer, const char *pszText, const char *pszSender)
        : mAccountId(nAccountId), mPlayer(nPlayer), mText(pszText), mSender(pszSender) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "ChatMsg", 0);
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
        return new ChatMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nChatMsgType.
     */
    int Type() override {
        return g_nChatMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `ChatMsg`.
     */
    const char *GetName() const override {
        return "ChatMsg";
    }

    /**
     * Register the sink that receives the chat of a channel.
     *
     * The type information of the registry identifies the pointer type as ChatHandler. A panel
     * passes itself, and the sink receives each line through Dispatch().
     *
     * @param nChannel The channel.
     * @param pHandler The sink.
     * @ghidraAddress NTSC-U/C: 0x00252220
     * @ghidraAddress PAL: 0x0025abc8
     */
    static void AddHandler(int nChannel, MsgSink *pHandler);

    /**
     * Forget the sink of a channel.
     *
     * The name is inferred.
     *
     * @param nChannel The channel.
     * @ghidraAddress NTSC-U/C: 0x00252408
     * @ghidraAddress PAL: 0x0025adb0
     */
    static void RemoveSink(int nChannel);

    /**
     * Send a line of chat.
     *
     * The names of the parameters are inferred.
     *
     * @param nChannel The channel, 2 for the notices of the session.
     * @param nRecipient The recipient, 0 for every console.
     * @param pszText The text.
     * @param nEcho Non-zero to show the line on this console too.
     * @return Whether the line was sent.
     * @ghidraAddress NTSC-U/C: 0x00252658
     * @ghidraAddress PAL: 0x0025b000
     */
    static bool Send(int nChannel, int nRecipient, const char *pszText, int nEcho);

    /**
     * Send a line of chat to one account only.
     *
     * The online session exchanges its key through the line. The name is inferred.
     *
     * @param nChannel The channel.
     * @param nAccountId The account that receives the line.
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x00252580
     * @ghidraAddress PAL: 0x0025af28
     */
    static void SendPrivate(int nChannel, int nAccountId, const char *pszText);

    /**
     * Discard the lines whose remaining parts have not arrived.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00252d58
     * @ghidraAddress PAL: 0x0025b700
     */
    static void ClearPartialMessages();

    /**
     * Deliver a line of chat from the lobby server to the sink of its channel.
     *
     * The lobby client calls the member. The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the line.
     * @param pMessage The line.
     * @ghidraAddress NTSC-U/C: 0x00252dd0
     * @ghidraAddress PAL: 0x0025b778
     */
    static void
    OnLobbyChat(int nConnection, int nType, int nSize, const rtMediusChatMessage *pMessage);

    /**
     * Write the channel and the text.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002691b8
     * @ghidraAddress PAL: 0x00272dc0
     */
    void PrintExtra(PrnStream &stream) const override;

    int mAccountId;      /*!< The sender's account identifier. */
    int mPlayer;         /*!< The sender's identifier in the session, or -2 for none. */
    const char *mText;   /*!< The text. */
    const char *mSender; /*!< The sender's name. */

private:
    /**
     * Report the sink of a channel.
     *
     * The name is inferred.
     *
     * @param nChannel The channel.
     * @return The sink, or null.
     * @ghidraAddress NTSC-U/C: 0x002521d8
     * @ghidraAddress PAL: 0x0025ab80
     */
    static MsgSink *FindHandler(int nChannel);

    /**
     * Limit the rate of the lines the console sends.
     *
     * Each line adds one to a level that drains by a third each second, and a line is refused while
     * the level exceeds four. The name is inferred.
     *
     * @return Whether the line may be sent.
     * @ghidraAddress NTSC-U/C: 0x00252498
     * @ghidraAddress PAL: 0x0025ae40
     */
    static bool AllowLine();
};
