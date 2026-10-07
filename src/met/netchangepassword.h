#pragma once

#include "met/freqscreen.h"
#include "msg/changepasswordresultmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that changes the online password, then saves the Freq or returns to the welcome.
 *
 * The RTTI records the class as deriving from FreqScreen.
 */
class NetChangePassword : public FreqScreen {
public:
    /**
     * Construct a screen with empty passwords.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x0017b690
     * @ghidraAddress PAL: 0x0017f2e8
     */
    explicit NetChangePassword(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x003596c8
     * @ghidraAddress PAL: 0x003c69d8
     */
    ~NetChangePassword() override {
    }

    /**
     * Produce a screen on the heap.
     *
     * @param pData The description of the screen.
     * @return The screen.
     */
    static UIScreen *New(DataArray *pData) {
        return new NetChangePassword(pData);
    }

    /**
     * Route the end of the entry and the result of the change.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0017b710
     * @ghidraAddress PAL: 0x0017f368
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report no title.
     *
     * @return The empty string.
     * @ghidraAddress NTSC-U/C: 0x003597c0
     * @ghidraAddress PAL: 0x003c6b28
     */
    const char *Title() override {
        return "";
    }

    String mOldPassword; /*!< The password to replace. */
    String mNewPassword; /*!< The replacement password. */
    int mSaveChanged;    /*!< Non-zero when the profile's password is written after the change. */
    int mSavePassword;   /*!< Whether the profile saves the new password, or clears it. */

private:
    /**
     * Change the password once the screen has entered.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017b7a0
     * @ghidraAddress PAL: 0x0017f3f8
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    /**
     * Save the password and the Freq, or show the error of the change.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x0017b808
     * @ghidraAddress PAL: 0x0017f460
     */
    bool HandleChangePasswordResult(ChangePasswordResultMsg *pMsg);
};
