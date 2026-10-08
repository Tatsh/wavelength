#pragma once

#include "met/remixselectscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uiscreen.h"

/**
 * Screen `fn_upload` that lists the saved remixes to choose one to upload.
 *
 * The RTTI records the class as deriving from RemixSelectScreen. Its vtable is at `0x003cdf30`. The
 * chosen remix loads through `load_remix`, which leads to `fn_upload_note`.
 */
class SelRemixUploadScreen : public RemixSelectScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0017e8c0
     * @ghidraAddress PAL: 0x00182580
     */
    explicit SelRemixUploadScreen(DataArray *pData) : RemixSelectScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035ae48
     * @ghidraAddress PAL: 0x003c8998
     */
    ~SelRemixUploadScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035af48
     * @ghidraAddress PAL: 0x003c8a98
     */
    static UIScreen *New(DataArray *pData) {
        return new SelRemixUploadScreen(pData);
    }

    /**
     * Route the choice to HandleSelect(), and every other message past RemixSelectScreen.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017e8f8
     * @ghidraAddress PAL: 0x001825b8
     */
    bool DispatchPriv(Message *pMsg) override;

private:
    /**
     * Load the chosen remix with the cross button and go on to `fn_upload_note`.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0017e960
     * @ghidraAddress PAL: 0x00182620
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);
};
