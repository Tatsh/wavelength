#pragma once

#include <vector>

#include "game/campaign.h"
#include "memcard/memcarduser.h"
#include "met/errorscreen.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that loads the Freqs saved on the memory card once it has entered, then hands them to the
 * SelLoadedFreqScreen that is its done screen.
 *
 * The RTTI records the class as deriving from ErrorScreen and from MemcardUser at `+0xa0`. Its
 * vtables are at `0x003d1378` and, for MemcardUser, `0x003d12e8`. A card without Freqs leads to
 * `no_freqs_loaded_error` and another failure to `load_failed_error`, which retry the load or
 * return to the start screen.
 */
class LoadFreqScreen : public ErrorScreen, public MemcardUser {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001aafb8
     * @ghidraAddress PAL: 0x001b2f90
     */
    explicit LoadFreqScreen(DataArray *pData) : ErrorScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00363190
     * @ghidraAddress PAL: 0x003d1788
     */
    ~LoadFreqScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x003632a8
     */
    static UIScreen *New(DataArray *pData) {
        return new LoadFreqScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x003632e8
     * @ghidraAddress PAL: 0x003d18e0
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the end of the entry to HandleTransitionComplete().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001ab280
     * @ghidraAddress PAL: 0x001b32b8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Hand the Freqs to the done screen and go there, or open the dialog of the failure.
     *
     * @param nStatus How the load ended, one of MemcardTask::Status.
     * @param pProfiles The Freqs loaded.
     * @ghidraAddress NTSC-U/C: 0x001aaff8
     * @ghidraAddress PAL: 0x001b2fd8
     */
    void OnFreqsLoaded(int nStatus, std::vector<Campaign> *pProfiles) override;

private:
    /**
     * Start the load once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001ab2e8
     * @ghidraAddress PAL: 0x001b3320
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);
};
