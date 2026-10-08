#pragma once

#include <vector>

#include "game/remixinfo.h"
#include "memcard/memcarduser.h"
#include "met/errorscreen.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that lists the remixes saved on the memory card once it has entered, then hands them to
 * the RemixSelectScreen that is its done screen.
 *
 * The RTTI records the class as deriving from ErrorScreen and from MemcardUser at `+0xa0`. Its
 * vtables are at `0x003d0820` and, for MemcardUser, `0x003d0790`. Outside `mem_remix`, solo and
 * local games go on through their own transitions. A card without remixes leads to
 * `no_remixes_loaded_error`.
 */
class ListRemixesScreen : public ErrorScreen, public MemcardUser {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001adc88
     * @ghidraAddress PAL: 0x001b6698
     */
    explicit ListRemixesScreen(DataArray *pData) : ErrorScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00364158
     */
    ~ListRemixesScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00364270
     */
    static UIScreen *New(DataArray *pData) {
        return new ListRemixesScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x003642b0
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the end of the entry to HandleTransitionComplete().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001adf90
     * @ghidraAddress PAL: 0x001b6a00
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Hand the remixes to the done screen and go on, or open the dialog of the failure.
     *
     * @param nStatus How the listing ended, one of MemcardTask::Status.
     * @param pInfos The descriptions of the remixes.
     * @ghidraAddress NTSC-U/C: 0x001adcc8
     * @ghidraAddress PAL: 0x001b66e0
     */
    void OnRemixesListed(int nStatus, std::vector<RemixInfo> *pInfos) override;

private:
    /**
     * Start the listing once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001adff8
     * @ghidraAddress PAL: 0x001b6a68
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);
};
