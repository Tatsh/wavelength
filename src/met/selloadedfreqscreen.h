#pragma once

#include <vector>

#include "game/campaign.h"
#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The screen that lists the Freqs of the memory card to load one.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x180 bytes and its
 * vtable is at `0x003cede0`. The metagame registers the class for the screen type
 * `sel_loaded_f_screen`, and the front-end description's `f_load` screen is one. Its panel is
 * `f_load` with the list `list`. The destructor at `0x0035d5d0` is compiler-generated and has no
 * declaration here.
 */
class SelLoadedFreqScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no Freqs.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018eb38
     * @ghidraAddress PAL: 0x00195e50
     */
    explicit SelLoadedFreqScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `sel_loaded_f_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035d6f0
     * @ghidraAddress PAL: 0x003cb7c8
     */
    static UIScreen *New(DataArray *pData) {
        return new SelLoadedFreqScreen(pData);
    }

    /**
     * Route a chosen Freq and a controller button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018ec30
     * @ghidraAddress PAL: 0x00195f48
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry, fill the list with mProfiles, and select the first Freq.
     *
     * The profile the player had before loading stays unchanged unless the player's current
     * profile is.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018eb80
     * @ghidraAddress PAL: 0x00195e98
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Replace the listed Freqs.
     *
     * @param profiles The Freqs.
     * @ghidraAddress NTSC-U/C: 0x0035d730
     */
    virtual void SetProfiles(const std::vector<Campaign> &profiles) {
        mProfiles = profiles;
    }

    /**
     * Make the Freq chosen with the cross button the only player, and go on to the skill menu of
     * a solo game or to the network settings.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0018ecc0
     * @ghidraAddress PAL: 0x00195fd8
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Go back to the Freq menu when the first controller's triangle button goes down, restoring
     * the profile the player had before loading.
     *
     * @param pMsg The message of the button.
     * @return True while the screen moves in or out, otherwise the result of
     * UIScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x0018edc0
     * @ghidraAddress PAL: 0x001960d8
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    std::vector<Campaign> mProfiles; /*!< The Freqs of the memory card. */
    Campaign mProfile;               /*!< The profile the player had before loading. */
};
