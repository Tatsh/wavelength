#pragma once

#include <vector>

#include "game/campaign.h"
#include "game/gameoptions.h"
#include "memcard/memcarduser.h"
#include "met/errorscreen.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen of the startup check of the memory card, which loads the settings and the newest Freq
 * before it leads to the done screen.
 *
 * The RTTI records the class as deriving from ErrorScreen and from MemcardUser at `+0xa0`. Its
 * vtables are at `0x003d0c40` and, for MemcardUser, `0x003d0bb0`. A missing, full, or unformatted
 * card opens `mc_check_no_card`, `mc_check_no_space`, or `mc_check_unformatted`. The loads run on
 * later polls, and the done screen follows a second after the Freq loads.
 */
class MCCheckScreen : public ErrorScreen, public MemcardUser {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001acb68
     * @ghidraAddress PAL: 0x001b52c0
     */
    explicit MCCheckScreen(DataArray *pData)
        : ErrorScreen(pData), mLoadFreqs(0), mLoadSettings(0), mAdvanceTime(0.0f) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00363b58
     */
    ~MCCheckScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00363c70
     */
    static UIScreen *New(DataArray *pData) {
        return new MCCheckScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x00363cb0
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the end of the entry to HandleTransitionComplete().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001accd8
     * @ghidraAddress PAL: 0x001b5538
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start a pending load, and go to the done screen once the time to advance has passed.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001acc20
     */
    void Poll(float fTime) override;

    /**
     * Exit, and give the first player a default Freq when no Freq of the player loaded.
     *
     * @param pNextScreen The screen that replaces this one.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001acbb8
     * @ghidraAddress PAL: 0x001b5310
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Load the settings of a usable card on the next poll, or open the dialog of the card's
     * failure.
     *
     * Outside edit mode, the settings load. In edit mode the screen goes to the done screen at
     * once.
     *
     * @param nStatus The outcome, one of MemcardTask::Status.
     * @param nFormat Non-zero for a formatted card.
     * @param nFree The free space of the card in kilobytes.
     * @param nNeeded The kilobytes one more Freq needs.
     * @ghidraAddress NTSC-U/C: 0x001acda0
     */
    void OnInitialCheck(int nStatus, int nFormat, int nFree, int nNeeded) override;

    /**
     * Apply the settings loaded, and load the Freqs on the next poll whatever the outcome.
     *
     * @param nStatus The outcome, one of MemcardTask::Status.
     * @param settings A copy of the settings loaded.
     * @ghidraAddress NTSC-U/C: 0x001ad200
     * @ghidraAddress PAL: 0x001b5930
     */
    void OnSettingsLoaded(int nStatus, GameOptions settings) override;

    /**
     * Make the newest Freq loaded the first player's, and go to the done screen a second later.
     *
     * A failed load goes to the done screen at once.
     *
     * @param nStatus The outcome, one of MemcardTask::Status.
     * @param pProfiles The Freqs loaded, newest first.
     * @ghidraAddress NTSC-U/C: 0x001ad278
     * @ghidraAddress PAL: 0x001b59a8
     */
    void OnFreqsLoaded(int nStatus, std::vector<Campaign> *pProfiles) override;

private:
    /**
     * Start the check of the card once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001acd40
     * @ghidraAddress PAL: 0x001b55a0
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    int mLoadFreqs;     // Whether the next poll loads the Freqs.
    int mLoadSettings;  // Whether the next poll loads the settings.
    float mAdvanceTime; // The front-end time to go to the done screen at, or 0 for none.
};
