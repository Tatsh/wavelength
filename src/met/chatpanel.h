#pragma once

#include <list>

#include "met/freqpanel.h"
#include "met/keyboarduser.h"
#include "os/string.h"
#include "ui/uitextentrycompletemsg.h"

/**
 * One line of a chat panel.
 *
 * The list node of a line is 0x34 bytes.
 */
struct ChatMessage {
    String mText;   /*!< The text of the line. */
    String mSender; /*!< The name of the player who sent the line. */
    int mPlayer;    /*!< The integer the chat message included with the name. */
};

/**
 * Panel that shows the chat of an online lobby or session.
 *
 * The RTTI records the class as deriving from FreqPanel and KeyboardUser, with KeyboardUser at
 * `+0xe0`. Only the members its users here touch are declared, and the routines of the class are
 * not reconstructed.
 */
class ChatPanel : public FreqPanel, public KeyboardUser {
public:
    /**
     * Send the text typed into the chat entry.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001a42e8
     * @ghidraAddress PAL: 0x001abfd8
     */
    bool HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg);

    std::list<ChatMessage> mMessages; /*!< The lines shown, oldest first. +0x104 */
};
