#pragma once

#include "memcard/memcarduser.h"
#include "met/overwritesavescreen.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Memory card screen that saves the game options once it has entered.
 *
 * The RTTI records the class as deriving from OverwriteSaveScreen and from MemcardUser at `+0xb0`.
 * Its vtables are at `0x003d0610` and, for MemcardUser, `0x003d0580`. The destructor at
 * `0x00364428` (PAL `0x003d2ba0`) is compiler-generated. Settings already on the card lead to
 * `settings_exist_error`.
 */
class SaveSettingsScreen : public OverwriteSaveScreen, public MemcardUser {
public:
    /**
     * Construct the screen from its script description, replacing saved settings.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001ae320
     */
    explicit SaveSettingsScreen(DataArray *pData) : OverwriteSaveScreen(pData) {
        mOverwriteStatus = 1;
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00364560
     */
    static UIScreen *New(DataArray *pData) {
        return new SaveSettingsScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x003645a0
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the end of the entry to HandleTransitionComplete().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001ae4d8
     * @ghidraAddress PAL: 0x001b70a8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Mark the options saved and go to the done screen, or open the dialog of the failure.
     *
     * @param nStatus How the save ended, one of MemcardTask::Status.
     * @param nNeeded The space the save needs, when the memory card is too full.
     * @ghidraAddress NTSC-U/C: 0x001ae368
     * @ghidraAddress PAL: 0x001b6ef0
     */
    void OnSettingsSaved(int nStatus, int nNeeded) override;

private:
    /**
     * Save the options once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001ae540
     * @ghidraAddress PAL: 0x001b7110
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);
};
