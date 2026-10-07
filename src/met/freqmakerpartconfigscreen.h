#pragma once

#include <vector>

#include "math/color.h"
#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/lrbutton.h"
#include "ui/uibutton.h"
#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectstartmsg.h"

/**
 * The part screen of the Freq maker, for choosing one part of the Freq and its colour.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0xe0 bytes and its vtable
 * is at `0x003cfdd0`. The metagame registers the class for the screen type `f_maker_part_screen`,
 * and the front-end description's `f_maker_part` screen is one. The part is the button of the
 * `f_maker_c` panel with the focus. The `f_maker_s` panel steps through the unlocked choices with
 * `part` and sets the colour with `colorize` (the hue), `saturation`, and `brightness`, each in
 * 51 steps.
 */
class FreqMakerPartConfigScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001a0b38
     * @ghidraAddress PAL: 0x001a8818
     */
    explicit FreqMakerPartConfigScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00360f20
     * @ghidraAddress PAL: 0x003cf438
     */
    ~FreqMakerPartConfigScreen() override;

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `f_maker_part_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00360fe0
     * @ghidraAddress PAL: 0x003cf4f8
     */
    static UIScreen *New(DataArray *pData) {
        return new FreqMakerPartConfigScreen(pData);
    }

    /**
     * Route buttons about to be chosen, focus changes, and controller buttons, and pass on every
     * other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001a1cb0
     * @ghidraAddress PAL: 0x001a9990
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry, list the choices of the part, and show the current choice and colour.
     *
     * A current choice that is not unlocked is added to the end of the list.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a0ba8
     * @ghidraAddress PAL: 0x001a8888
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Start the exit and turn the camera back to the whole Freq.
     *
     * @param pNextScreen The screen that replaces this one, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a12f0
     * @ghidraAddress PAL: 0x001a8fd0
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Report the localised title, `f_maker_c_TITLE`.
     *
     * @return The title.
     * @ghidraAddress NTSC-U/C: 0x001a1450
     * @ghidraAddress PAL: 0x001a9130
     */
    const char *Title() override;

    /**
     * Show the help `f_maker_s_part_HELP` with the name of the part.
     *
     * @ghidraAddress NTSC-U/C: 0x001a13d8
     * @ghidraAddress PAL: 0x001a90b8
     */
    void ShowPartHelp();

    /**
     * Colour the part with the hue, saturation, and brightness the buttons show.
     *
     * @ghidraAddress NTSC-U/C: 0x001a1480
     * @ghidraAddress PAL: 0x001a9160
     */
    void ApplyColor();

    /**
     * Step a setting down, hiding the left arrow at the lowest step and playing the `WRONG` cue
     * once when the setting is already there.
     *
     * @param nValue The setting.
     * @param nMin The lowest step.
     * @param nLimit One more than the highest step.
     * @param pValue Receives the new setting.
     * @param pButton The button of the setting.
     * @ghidraAddress NTSC-U/C: 0x001a1528
     * @ghidraAddress PAL: 0x001a9208
     */
    void StepDown(int nValue, int nMin, int nLimit, int *pValue, LRButton *pButton);

    /**
     * Step a setting up, hiding the right arrow at the highest step and playing the `WRONG` cue
     * once when the setting is already there.
     *
     * @param nValue The setting.
     * @param nMin The lowest step.
     * @param nLimit One more than the highest step.
     * @param pValue Receives the new setting.
     * @param pButton The button of the setting.
     * @ghidraAddress NTSC-U/C: 0x001a15f0
     * @ghidraAddress PAL: 0x001a92d0
     */
    void StepUp(int nValue, int nMin, int nLimit, int *pValue, LRButton *pButton);

    /**
     * Step the choice or a colour setting on left and right, and move the focus to `done` on the
     * cross button.
     *
     * @param pMsg The message.
     * @return True for the buttons of the panel, otherwise the result of
     *         FreqScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x001a16c0
     * @ghidraAddress PAL: 0x001a93a0
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Restore the part and return to `f_maker_custom` on the triangle button, and allow the
     * `WRONG` cue again when left or right is released.
     *
     * @param pMsg The message.
     * @return True while the screen moves, otherwise the result of FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x001a1b30
     * @ghidraAddress PAL: 0x001a9810
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Show the help of the part when `part` receives the focus.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001a1c48
     * @ghidraAddress PAL: 0x001a9928
     */
    bool HandleFocusChange(UIComponentFocusChangeMsg *pMsg);

    int mOriginalIndex;               /*!< The entry of mChoices the part had on entry. +0x70 */
    int mReserved74[3];               // +0x74, not yet recovered.
    alignas(16) Color mOriginalColor; /*!< The colour the part had on entry. +0x80 */
    const char *mPartLabel; /*!< The text of the `f_maker_c` button of the part, or null. +0x90 */
    int mPart;              /*!< One of AvatarPartSet::Part, or AvatarPartSet::kNumParts. */
    int mIndex;             /*!< The entry of mChoices the part has. */
    int mHue;               /*!< The hue step, from 0 to 50. */
    int mSaturation;        /*!< The saturation step, from 0 to 50. */
    int mBrightness;        /*!< The brightness step, from 0 to 50. */
    int mReservedA8;        // +0xa8, set to 1 on entry. The purpose is not recovered.
    int mReservedAC;        // +0xac, cleared on entry. The purpose is not recovered.
    int mErrorPlayed;       /*!< Non-zero once a step past the end played the `WRONG` cue. */
    std::vector<const char *> mChoices; /*!< The choices of the part. +0xb4 */
    UIButton *mPartButton;              /*!< The `part` button. +0xc4 */
    LRButton *mHueButton;               /*!< The `colorize` button. */
    LRButton *mBrightnessButton;        /*!< The `brightness` button. */
    LRButton *mSaturationButton;        /*!< The `saturation` button. +0xd0 */
};
