#pragma once

#include "memcard/memcarduser.h"
#include "met/errorscreen.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen `mc_format` that formats the memory card once it has entered.
 *
 * The RTTI records the class as deriving from ErrorScreen and from MemcardUser at `+0xa0`. Its
 * vtables are at `0x003d0d48` and, for MemcardUser, `0x003d0cb8`. A failure leads to
 * `already_formatted_error` or `format_card_error`, which retry the format or go to the done
 * screen.
 */
class FormatScreen : public ErrorScreen, public MemcardUser {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001ac868
     * @ghidraAddress PAL: 0x001b4fb8
     */
    explicit FormatScreen(DataArray *pData) : ErrorScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x003639f0
     */
    ~FormatScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00363b08
     */
    static UIScreen *New(DataArray *pData) {
        return new FormatScreen(pData);
    }

    /**
     * Report the empty title of the screen.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x00363b48
     */
    const char *Title() override {
        return "";
    }

    /**
     * Route the end of the entry to HandleTransitionComplete().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001ac8a8
     * @ghidraAddress PAL: 0x001b4ff8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Go to the done screen after the format, or open the dialog of the failure.
     *
     * @param nStatus How the format ended, one of MemcardTask::Status.
     * @ghidraAddress NTSC-U/C: 0x001ac960
     * @ghidraAddress PAL: 0x001b50b0
     */
    void OnCardFormatted(int nStatus) override;

private:
    /**
     * Start the format once this screen has entered.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001ac910
     * @ghidraAddress PAL: 0x001b5060
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);
};
