#pragma once

#include <list>

#include "met/freqscreen.h"
#include "netflow/lobbylocation.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uiscreen.h"

/**
 * Screen where the player chooses the location of the lobby server to log in to.
 *
 * The RTTI records the class as deriving from FreqScreen. Each button of the `fn_sel_server`
 * panel, in the order of their names, shows one location.
 */
class NetServerSelScreen : public FreqScreen {
public:
    /**
     * Construct a screen with no locations.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x00178020
     * @ghidraAddress PAL: 0x0017bc30
     */
    explicit NetServerSelScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x003597e0
     * @ghidraAddress PAL: 0x003c9d08
     */
    ~NetServerSelScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     * @ghidraAddress NTSC-U/C: 0x003598a0
     * @ghidraAddress PAL: 0x003c9dc8
     */
    static UIScreen *New(DataArray *pData) {
        return new NetServerSelScreen(pData);
    }

    /**
     * Route a choice to HandleSelect().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001782a8
     * @ghidraAddress PAL: 0x0017beb8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Label one button with each location, disable the rest, and enter.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x001780a0
     * @ghidraAddress PAL: 0x0017bcb0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    std::list<LobbyLocation> mLocations; /*!< The locations of the lobby server. */

private:
    /**
     * Pass the chosen location to NetServerLogin and change to it.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00178310
     * @ghidraAddress PAL: 0x0017bf20
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);
};
