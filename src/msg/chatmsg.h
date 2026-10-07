#pragma once

#include "msg/message.h"

/**
 * Message with one line of chat between the consoles of an online session or lobby.
 *
 * The RTTI records the class as deriving from Message, and its vtable is at `0x003d58c8`. Only the
 * routine its senders here use is declared.
 */
class ChatMsg : public Message {
public:
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
};
