#pragma once

#include "met/freqscreen.h"
#include "msg/reporemixstatusmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that uploads the remix with its note and shows the progress.
 *
 * The RTTI records the class as deriving from FreqScreen.
 */
class NetDoUploadScreen : public FreqScreen {
public:
    /**
     * Construct a screen with no note.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017f178
     * @ghidraAddress PAL: 0x00182ef0
     */
    explicit NetDoUploadScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035b0a8
     * @ghidraAddress PAL: 0x003c8bb8
     */
    ~NetDoUploadScreen() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     */
    static UIScreen *New(DataArray *pData) {
        return new NetDoUploadScreen(pData);
    }

    /**
     * Route the status of the upload and the end of the entry.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017f298
     * @ghidraAddress PAL: 0x00183010
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report no title.
     *
     * @return The empty string.
     * @ghidraAddress NTSC-U/C: 0x0035b198
     * @ghidraAddress PAL: 0x003c8cf0
     */
    const char *Title() override {
        return "";
    }

    String mNote; /*!< The note uploaded with the remix. */

private:
    /**
     * Show the part of the remix uploaded on the dialog.
     *
     * The name is inferred.
     *
     * @param fProgress The part uploaded, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x0017f1e0
     * @ghidraAddress PAL: 0x00182f58
     */
    void ShowProgress(float fProgress);

    /**
     * Start the upload once the screen has entered.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017f328
     * @ghidraAddress PAL: 0x001830a0
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Show the progress, or go on when the upload ends.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017f398
     * @ghidraAddress PAL: 0x00183110
     */
    bool HandleRepoRemixStatus(RepoRemixStatusMsg *pMsg);
};
