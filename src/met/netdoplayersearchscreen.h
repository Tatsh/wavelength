#pragma once

#include "met/freqscreen.h"
#include "msg/findplayermsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that finds a player by name and shows the result.
 *
 * The RTTI records the class as deriving from FreqScreen.
 */
class NetDoPlayerSearchScreen : public FreqScreen {
public:
    /**
     * Construct a screen with no name.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017da80
     * @ghidraAddress PAL: 0x00181740
     */
    explicit NetDoPlayerSearchScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035aa80
     * @ghidraAddress PAL: 0x003c85d0
     */
    ~NetDoPlayerSearchScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     */
    static UIScreen *New(DataArray *pData) {
        return new NetDoPlayerSearchScreen(pData);
    }

    /**
     * Route the result of the search and the end of the entry.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017dae8
     * @ghidraAddress PAL: 0x001817a8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report no title.
     *
     * @return The empty string.
     * @ghidraAddress NTSC-U/C: 0x0035ab70
     * @ghidraAddress PAL: 0x003c86c0
     */
    const char *Title() override {
        return "";
    }

    String mPlayerName; /*!< The name of the player to find. */

private:
    /**
     * Start the search once the screen has entered.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017db78
     * @ghidraAddress PAL: 0x00181838
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Show the player found, or the error of the search.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017dbd8
     * @ghidraAddress PAL: 0x00181898
     */
    bool HandleFindPlayer(FindPlayerMsg *pMsg);
};
