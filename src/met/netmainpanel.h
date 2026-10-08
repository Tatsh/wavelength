#pragma once

#include "met/focuschangepanel.h"
#include "met/netflashupdate.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Panel of the online main screen whose data the lobby service sends, requested again a while
 * after each reply.
 *
 * The RTTI records the class as deriving from FocusChangePanel and from NetFlashUpdate at `+0x100`.
 * Its vtable is at `0x003cd028`. The L1 button requests the data at once. With the description's
 * `hilite_flash_panel` set, the panel mesh flashes while a request is pending.
 */
class NetMainPanel : public FocusChangePanel, public NetFlashUpdate {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00171d88
     * @ghidraAddress PAL: 0x001751d0
     */
    NetMainPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00357b80
     * @ghidraAddress PAL: 0x003c4f30
     */
    ~NetMainPanel() override {
    }

    /**
     * Route the controller to HandleJoypad().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001720b8
     * @ghidraAddress PAL: 0x00175500
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter unfocused, show the titles instead of the `refresh` text, and request the data on the
     * next Poll().
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00171e50
     * @ghidraAddress PAL: 0x00175298
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Cancel the pending request and exit.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00171e30
     * @ghidraAddress PAL: 0x00175278
     */
    void Exit(bool bForce, float fTime) override;

    /**
     * Request the data once its time has passed, and advance the flash.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00171f30
     * @ghidraAddress PAL: 0x00175378
     */
    void Poll(float fTime) override;

    /**
     * Finish the load and find the objects of the flash.
     *
     * @ghidraAddress NTSC-U/C: 0x00171df0
     * @ghidraAddress PAL: 0x00175238
     */
    void FinishLoad() override;

    /**
     * Request the data of the panel from the lobby service.
     *
     * The panel itself requests nothing. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00357c48
     * @ghidraAddress PAL: 0x003c4ff8
     */
    virtual void RequestUpdate() {
    }

    /**
     * Request the data on the next Poll() and start the flash.
     *
     * The name is inferred.
     *
     * @param bHilite Whether the mesh ends with the lit material.
     * @ghidraAddress NTSC-U/C: 0x00172010
     * @ghidraAddress PAL: 0x00175458
     */
    void RequestNow(bool bHilite);

protected:
    /**
     * Swallow a button during a transition, give the focus back to `fn_main` with the triangle or
     * the left button, and request the data at once with the L1 button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True during a transition, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x00172120
     * @ghidraAddress PAL: 0x00175568
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    float mRequestTime; /*!< The SystemMs() time of the next request, or 0 for none. */
    int mFlashPanel;    /*!< The description's `hilite_flash_panel`, whether the mesh flashes. */
};
