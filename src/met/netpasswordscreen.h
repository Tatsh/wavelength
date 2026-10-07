#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uiscreen.h"

/**
 * Screen with a `save` button that chooses whether the online password is saved in the profile.
 *
 * The RTTI records the class as deriving from FreqScreen. NetLoginScreen, NetCreateUserScreen,
 * and NetChangePasswordScreen derive from it. The left and right directional buttons toggle the
 * choice.
 */
class NetPasswordScreen : public FreqScreen {
public:
    /**
     * Construct a screen that saves the password.
     *
     * @param pData The description of the screen.
     * @ghidraAddress NTSC-U/C: 0x00178408
     * @ghidraAddress PAL: 0x0017c018
     */
    explicit NetPasswordScreen(DataArray *pData);

    /**
     * Release the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x003590c8
     * @ghidraAddress PAL: 0x003c6430
     */
    ~NetPasswordScreen() override {
    }

    /**
     * Route a select start to HandleSelectStart().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00178550
     * @ghidraAddress PAL: 0x0017c160
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Take the choice from whether the profile has a saved password, and label the `save` button.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x00178448
     * @ghidraAddress PAL: 0x0017c058
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

protected:
    /**
     * Toggle the choice when a directional button moves on the `save` button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x001785b8
     * @ghidraAddress PAL: 0x0017c1c8
     */
    bool HandleSaveToggle(UIComponentSelectStartMsg *pMsg);

    /**
     * Label a component with the localised text of the choice.
     *
     * The binary expands it inline.
     *
     * @param pComponent The component.
     */
    void LabelSaveChoice(UIComponent *pComponent);

    int mSavePassword;        /*!< Whether the password is saved in the profile. */
    int mSavePasswordInitial; /*!< The choice when the screen entered. */
};
