#pragma once

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

    int mReserved04;     // +0x04, not yet identified.
    int mPlayer;         /*!< The sender's identifier in the session, or -2 for none. */
    const char *mText;   /*!< The text. */
    const char *mSender; /*!< The sender's name. */
};
