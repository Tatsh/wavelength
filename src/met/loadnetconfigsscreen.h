#pragma once

#include <list>

#include "memcard/memcarduser.h"
#include "met/errorscreen.h"
#include "netflow/inetconfig.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that lists the network configurations of the memory card once it has entered, then hands
 * them to `fn_config`.
 *
 * The RTTI records the class as deriving from ErrorScreen and from MemcardUser at `+0xa0`. Its
 * vtables are at `0x003d0508` and, for MemcardUser, `0x003d0478`. A configuration that cannot be
 * used leads to `net_load_config_error` with the message of the failure.
 */
class LoadNetConfigsScreen : public ErrorScreen, public MemcardUser {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001ae5c0
     * @ghidraAddress PAL: 0x001b7190
     */
    explicit LoadNetConfigsScreen(DataArray *pData) : ErrorScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x003645b0
     */
    ~LoadNetConfigsScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x003646c8
     */
    static UIScreen *New(DataArray *pData) {
        return new LoadNetConfigsScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x00364708
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the end of the entry to HandleTransitionComplete().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001ae7e8
     * @ghidraAddress PAL: 0x001b74c8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Hand the configurations to `fn_config` and go there, or open the dialog of the failure.
     *
     * @param nStatus How the listing ended, one of MemcardTask::Status.
     * @param pConfigs The configurations.
     * @ghidraAddress NTSC-U/C: 0x001ae600
     * @ghidraAddress PAL: 0x001b71d8
     */
    void OnNetConfigsListed(int nStatus, std::list<InetConfig> *pConfigs) override;

private:
    /**
     * Clear the configurations of `fn_config` and start the listing once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001ae850
     * @ghidraAddress PAL: 0x001b7530
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);
};
