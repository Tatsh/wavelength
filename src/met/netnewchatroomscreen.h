#pragma once

#include "met/freqscreen.h"
#include "msg/joinchatroomresultmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that creates a chatroom of the lobby and moves the player into it, or shows the error.
 *
 * The RTTI records the class as deriving from FreqScreen.
 */
class NetNewChatroomScreen : public FreqScreen {
public:
    /**
     * Construct a screen with no name.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017d618
     * @ghidraAddress PAL: 0x001812d8
     */
    explicit NetNewChatroomScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035a538
     * @ghidraAddress PAL: 0x003c8088
     */
    ~NetNewChatroomScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     * @ghidraAddress NTSC-U/C: 0x0035a5e0
     * @ghidraAddress PAL: 0x003c8130
     */
    static UIScreen *New(DataArray *pData) {
        return new NetNewChatroomScreen(pData);
    }

    /**
     * Route the result of the creation and the end of the entry.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017d680
     * @ghidraAddress PAL: 0x00181340
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report no title.
     *
     * @return The empty string.
     * @ghidraAddress NTSC-U/C: 0x0035a620
     * @ghidraAddress PAL: 0x003c8170
     */
    const char *Title() override {
        return "";
    }

    String mChatroomName; /*!< The name of the new chatroom. */

private:
    /**
     * Clear the chat and create the chatroom once the screen has entered.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017d710
     * @ghidraAddress PAL: 0x001813d0
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Enter the chatroom, or return the name to the setup and show the error.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017d808
     * @ghidraAddress PAL: 0x001814c8
     */
    bool HandleJoinChatroomResult(JoinChatroomResultMsg *pMsg);
};
