#pragma once

#include <librtmedius.h>

#include "app/msgsink.h"
#include "msg/message.h"

/**
 * Identity that ChatMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b88
 */
extern int g_nChatMsgType;

/**
 * Message with one line of chat between the consoles of an online session or lobby.
 *
 * The RTTI records the class as deriving from Message, and its vtable is at `0x003d58c8`. Only the
 * routines and members its users here need are declared.
 */
class ChatMsg : public Message {
public:
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
     * Forget every registered sink.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00252d58
     * @ghidraAddress PAL: 0x0025b700
     */
    static void ClearHandlers();

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
};
