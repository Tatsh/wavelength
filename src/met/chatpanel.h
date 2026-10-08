#pragma once

#include <list>
#include <utility>
#include <vector>

#include "math/vector3.h"
#include "met/freqpanel.h"
#include "met/keyboarduser.h"
#include "msg/chatmsg.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "rnd/text.h"
#include "script/dataarray.h"
#include "ui/uitextentrycompletemsg.h"

/**
 * One line of a chat panel.
 *
 * The list node of a line is 0x34 bytes.
 */
struct ChatMessage {
    String mSender; /*!< The name of the player who sent the line. */
    String mText;   /*!< The text of the line. */
    int mPlayer;    /*!< The sender's identifier in the session, as ChatMsg::mPlayer reports it. */
};

/**
 * Panel that shows the chat of an online lobby or launchpad and sends the lines the player types.
 *
 * The RTTI records the class as deriving from FreqPanel and KeyboardUser, with KeyboardUser at
 * `+0xe0`. The vtables are at `0x003cff40` and `0x003cff20`. The destructor at `0x00361238` (PAL
 * `0x003cf750`) is compiler-generated.
 *
 * The description provides `chat_type`, `num_chat_lines`, and `spacing`. The panel's file provides
 * the first row, the texts `<panel>_username01.txt` and `<panel>_message01.txt`, and FinishLoad()
 * clones one more row for each further line, `spacing` units lower each time. A line whose text
 * wraps takes the rows below it.
 */
class ChatPanel : public FreqPanel, public KeyboardUser {
public:
    /**
     * The chat channels of the `chat_type` setting.
     */
    enum ChatType {
        kChatLobby = 1,     /*!< `lobby`, the chat of the online lobby. */
        kChatLaunchpad = 2, /*!< `launchpad`, the chat of a launchpad. */
    };

    /**
     * Construct the panel from its script description and register it for the chat of its
     * channel.
     *
     * A `chat_type` other than `lobby` or `launchpad` leaves mChatType unset.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x001a2a78
     * @ghidraAddress PAL: 0x001aa758
     */
    ChatPanel(DataArray *pData, const char *pszDir);

    /**
     * Create a panel from its script description.
     *
     * Metagame::RegisterScreenClasses() registers the routine for the entry type `chat_panel`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x003614e8
     * @ghidraAddress PAL: 0x003cfa00
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new ChatPanel(pData, pszDir);
    }

    /**
     * Route the chat, the controller, and a completed entry to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001a3b50
     * @ghidraAddress PAL: 0x001ab830
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Release the cloned rows once the last reference to the panel's file is released.
     *
     * @ghidraAddress NTSC-U/C: 0x001a2c40
     * @ghidraAddress PAL: 0x001aa920
     */
    void Unload() override;

    /**
     * Fit the entry after the player's name, show the lines, and send a line typed on the
     * front-end keyboard.
     *
     * A line that cannot be sent goes back into the entry.
     *
     * @param bForce Whether to enter even when the panel already shows.
     * @param fTime The time of the entry.
     * @ghidraAddress NTSC-U/C: 0x001a3848
     * @ghidraAddress PAL: 0x001ab528
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Clear the rows and stop the entry.
     *
     * @param bForce Whether to exit even when the panel is already hidden.
     * @param fTime The time of the exit.
     * @ghidraAddress NTSC-U/C: 0x001a3a10
     * @ghidraAddress PAL: 0x001ab6f0
     */
    void Exit(bool bForce, float fTime) override;

    /**
     * Find the first row and clone the others.
     *
     * @ghidraAddress NTSC-U/C: 0x001a2ec0
     * @ghidraAddress PAL: 0x001aaba0
     */
    void FinishLoad() override;

    /**
     * Show a line at the current row and advance the row past it.
     *
     * In a launchpad, the name takes the colour of the sender. The text wraps to the width that
     * the name leaves.
     *
     * @param pszSender The sender's name, or an empty text for none.
     * @param pszText The text.
     * @param nPlayer The sender's identifier in the session.
     * @ghidraAddress NTSC-U/C: 0x001a4378
     * @ghidraAddress PAL: 0x001ac068
     */
    virtual void ShowLine(const char *pszSender, const char *pszText, int nPlayer);

    /**
     * Retain the text typed on the front-end keyboard for Enter() to send.
     *
     * @param pszText The text.
     * @return 1, for the keyboard to return to the screen that opened it.
     * @ghidraAddress NTSC-U/C: 0x001a3b30
     * @ghidraAddress PAL: 0x001ab810
     */
    int ReceiveKeyboardText(const char *pszText) override;

    /**
     * Send the text typed into the chat entry, and clear the entry once the line is sent.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001a42e8
     * @ghidraAddress PAL: 0x001abfd8
     */
    bool HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg);

    /**
     * Append a line of chat, dropping the oldest line beyond the row count.
     *
     * @param pMsg The line.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001a3c00
     * @ghidraAddress PAL: 0x001ab8e0
     */
    bool HandleChat(ChatMsg *pMsg);

    /**
     * Open the front-end keyboard for the chat when circle goes down.
     *
     * The keyboard returns to the screen of the lobby or the launchpad.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001a4160
     * @ghidraAddress PAL: 0x001abe40
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Show the newest lines that fit in the rows, oldest first.
     *
     * @ghidraAddress NTSC-U/C: 0x001a3df0
     * @ghidraAddress PAL: 0x001abad0
     */
    void Refresh();

    /**
     * Clone a text of the first row for a further row, under the prefix `t_<row>_`.
     *
     * The name is inferred.
     *
     * @param nRow The row, from 1.
     * @param pTemplate The text of the first row.
     * @return The clone, or null.
     * @ghidraAddress NTSC-U/C: 0x001a2db0
     * @ghidraAddress PAL: 0x001aaa90
     */
    Rnd::Text *CloneText(int nRow, Rnd::Text *pTemplate);

    String mText;                     /*!< The last line typed into the entry. */
    std::list<ChatMessage> mMessages; /*!< The lines shown, oldest first. */
    int mNumLines;                    /*!< The `num_chat_lines` setting, the number of rows. */
    int mReserved110;                 // +0x110, Enter() clears it, and no reader is known.
    std::vector<std::pair<Rnd::Text *, Rnd::Text *>> mRows; /*!< The name and text of each row. */
    int mRow;                                               /*!< The row ShowLine() fills next. */
    int mChatType;                                          /*!< One of ChatType. */
    alignas(16) Vector3 mTextStart; /*!< Where the text of the first row starts after the name. */
    alignas(16) Vector3 mRowStart;  /*!< The position of the name of the first row. */
    std::vector<Vector3> mRowPositions; /*!< The position of the name of each row. */
    int mSpacing;                       /*!< The `spacing` setting, the height of a row. */
    String mKeyboardText;               /*!< The text from the keyboard that Enter() sends. */
};
