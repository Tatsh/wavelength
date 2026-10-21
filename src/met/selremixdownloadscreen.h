#pragma once

#include <list>

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "netflow/netreporemix.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uiscreen.h"

/**
 * Screen `fn_download` that lists the remixes of the online repository to choose one to download.
 *
 * The RTTI records the class as deriving from FreqScreen. Its vtable is at `0x003cddf0`. The circle
 * button switches the list between the note of the selected remix and its band picture, and the
 * cross button goes to download the selected remix on `fn_do_download`.
 */
class SelRemixDownloadScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no remixes.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0017f660
     * @ghidraAddress PAL: 0x001833d8
     */
    explicit SelRemixDownloadScreen(DataArray *pData) : FreqScreen(pData) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x0035b1a8
     * @ghidraAddress PAL: 0x003c8d00
     */
    ~SelRemixDownloadScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035b268
     * @ghidraAddress PAL: 0x003c8dc0
     */
    static UIScreen *New(DataArray *pData) {
        return new SelRemixDownloadScreen(pData);
    }

    /**
     * Route the choice and the controller to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017f760
     * @ghidraAddress PAL: 0x001834d8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter, list the remixes with their notes, and select the first.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0017f6e0
     * @ghidraAddress PAL: 0x00183458
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /** @param remixes The remixes of the repository. */
    void SetRemixes(const std::list<NetRepoRemix> &remixes) {
        mRemixes = remixes;
    }

private:
    /**
     * Go to download the selected remix with the cross button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0017f7f0
     * @ghidraAddress PAL: 0x00183568
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Switch between the note and the band picture with the circle button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True during a transition, otherwise the result of UIScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x0017f940
     * @ghidraAddress PAL: 0x001836d0
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mShowNote; // Whether the list shows the note rather than the band picture.
    std::list<NetRepoRemix> mRemixes; /*!< The remixes of the repository. */
};
