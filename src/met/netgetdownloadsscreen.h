#pragma once

#include "met/freqscreen.h"
#include "netflow/reporemixesmsg.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that requests the remixes of the online repository once it has entered, then shows them
 * on `fn_download`.
 *
 * The RTTI records the class as deriving from FreqScreen. Its vtable is at `0x003cdf98`. An empty
 * or failed reply leads to `net_get_download_list_error`.
 */
class NetGetDownloadsScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0017f438
     * @ghidraAddress PAL: 0x001831b0
     */
    explicit NetGetDownloadsScreen(DataArray *pData) : FreqScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035ad60
     * @ghidraAddress PAL: 0x003c88b0
     */
    ~NetGetDownloadsScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035adf8
     * @ghidraAddress PAL: 0x003c8948
     */
    static UIScreen *New(DataArray *pData) {
        return new NetGetDownloadsScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035ae38
     * @ghidraAddress PAL: 0x003c8988
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the remixes of the repository and the end of the entry to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017f470
     * @ghidraAddress PAL: 0x001831e8
     */
    bool DispatchPriv(Message *pMsg) override;

private:
    /**
     * Request the remixes of the repository once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017f500
     * @ghidraAddress PAL: 0x00183278
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Give the remixes to `fn_download` and go there, or go to the error screen.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017f560
     * @ghidraAddress PAL: 0x001832d8
     */
    bool HandleRepoRemixes(RepoRemixesMsg *pMsg);
};
