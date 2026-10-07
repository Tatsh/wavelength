#pragma once

#include <vector>

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uibutton.h"
#include "ui/uicomponentselectstartmsg.h"

/**
 * The emblem screen of the Freq maker, for choosing the emblem on the torso.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x90 bytes and its vtable
 * is at `0x003cfd70`. The metagame registers the class for the screen type
 * `f_maker_emblem_screen`, and the front-end description's `f_maker_emblem` screen is one. Its
 * `f_maker_e` panel steps through the unlocked emblems with `part`.
 */
class FreqMakerEmblemScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001a20b8
     * @ghidraAddress PAL: 0x001a9d98
     */
    explicit FreqMakerEmblemScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00361020
     * @ghidraAddress PAL: 0x003cf538
     */
    ~FreqMakerEmblemScreen() override;

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `f_maker_emblem_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x003610e0
     * @ghidraAddress PAL: 0x003cf5f8
     */
    static UIScreen *New(DataArray *pData) {
        return new FreqMakerEmblemScreen(pData);
    }

    /**
     * Route buttons about to be chosen and controller buttons, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001a2388
     * @ghidraAddress PAL: 0x001aa068
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry, list the unlocked emblems, and show the current one.
     *
     * A current emblem that is not unlocked is added to the end of the list.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a1d60
     * @ghidraAddress PAL: 0x001a9a40
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Start the exit and turn the camera back to the whole Freq.
     *
     * @param pNextScreen The screen that replaces this one, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a2060
     * @ghidraAddress PAL: 0x001a9d40
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Report the localised title, `f_maker_c_TITLE`.
     *
     * @return The title.
     * @ghidraAddress NTSC-U/C: 0x001a2088
     * @ghidraAddress PAL: 0x001a9d68
     */
    const char *Title() override;

    /**
     * Step through the emblems on `part`, and move the focus to `done` on the cross button.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x001a2108
     * @ghidraAddress PAL: 0x001a9de8
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Restore the emblem and return to `f_maker_custom` on the triangle button.
     *
     * @param pMsg The message.
     * @return True while the screen moves, otherwise the result of FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x001a22b8
     * @ghidraAddress PAL: 0x001a9f98
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mOriginalIndex;                 /*!< The entry of mEmblems the torso had on entry. +0x70 */
    int mIndex;                         /*!< The entry of mEmblems the torso has. */
    int mReserved78;                    // +0x78, set to 1 on entry. The purpose is not recovered.
    std::vector<const char *> mEmblems; /*!< The emblems. +0x7c */
    UIButton *mPartButton;              /*!< The `part` button. +0x8c */
};
