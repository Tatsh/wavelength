#pragma once

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectstartmsg.h"

/**
 * The game options menu: the size of the track display, the speaker output, the hints, and the
 * controller vibration.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x80 bytes and its vtable
 * is at `0x003cc370`. The menu edits a copy of the settings and stores it in the game database when
 * its `done` button is chosen.
 */
class GameOptionsScreen : public FreqScreen {
public:
    /** Bits of mToggles. */
    enum Toggle {
        kToggleForceFeedback = 1, /*!< Controller vibration. */
        kToggleHelpText = 2,      /*!< The hints. */
    };

    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00356938
     * @ghidraAddress PAL: 0x003c3b98
     */
    explicit GameOptionsScreen(DataArray *pData) : FreqScreen(pData) {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x003568a0
     * @ghidraAddress PAL: 0x003c3b00
     */
    static UIScreen *New(DataArray *pData) {
        return new GameOptionsScreen(pData);
    }

    /**
     * Route the start of a choice and a controller button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0016c018
     * @ghidraAddress PAL: 0x0016f1a0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter, remember the screen to return to, and show the stored settings.
     *
     * @param pPrevScreen The screen this one replaces, and the one `done` returns to.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016b850
     * @ghidraAddress PAL: 0x0016e9d8
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Copy the stored settings into the menu and show them.
     *
     * @ghidraAddress NTSC-U/C: 0x0016b890
     * @ghidraAddress PAL: 0x0016ea18
     */
    void Refresh();

    /**
     * Show whether one of the two toggles is on.
     *
     * @param pszComponent The component of the toggle, `helptext` or `vibration`.
     * @ghidraAddress NTSC-U/C: 0x0016b9c8
     * @ghidraAddress PAL: 0x0016eb50
     */
    void RefreshToggle(const char *pszComponent);

    /**
     * Store the settings of the menu in the game database.
     *
     * @ghidraAddress NTSC-U/C: 0x0016bab0
     * @ghidraAddress PAL: 0x0016ec38
     */
    void Commit();

    /**
     * Report whether toggles are on.
     *
     * @param nToggle The bits of Toggle.
     * @return Whether any of the toggles is on.
     * @ghidraAddress NTSC-U/C: 0x0016bb30
     * @ghidraAddress PAL: 0x0016ecb8
     */
    bool IsToggleOn(unsigned char nToggle) const;

    /**
     * Turn toggles over.
     *
     * When any of the toggles is on, all of them turn off. Otherwise all of them turn on.
     *
     * @param nToggle The bits of Toggle.
     * @ghidraAddress NTSC-U/C: 0x0016bb48
     * @ghidraAddress PAL: 0x0016ecd0
     */
    void FlipToggle(unsigned char nToggle);

    /**
     * Turn toggles on or off.
     *
     * @param nToggle The bits of Toggle.
     * @param nOn Non-zero for on.
     * @ghidraAddress NTSC-U/C: 0x0016bb78
     * @ghidraAddress PAL: 0x0016ed00
     */
    void SetToggle(unsigned char nToggle, int nOn);

    /**
     * Report the label of a size of the track display.
     *
     * @param nFreqSize One of GameOptions::FreqSize.
     * @return The localized label, or `NONE` for another value.
     * @ghidraAddress NTSC-U/C: 0x0016bba8
     * @ghidraAddress PAL: 0x0016ed30
     */
    const char *FreqSizeLabel(int nFreqSize) const;

    /**
     * Report the label of a toggle.
     *
     * @param bOn Whether the toggle is on.
     * @return The localized label.
     * @ghidraAddress NTSC-U/C: 0x0016bc38
     * @ghidraAddress PAL: 0x0016edc0
     */
    const char *OnOffLabel(bool bOn) const;

    /**
     * Report the label of a speaker output mode.
     *
     * @param nOutputMode The mode, 0 for mono, 1 for stereo, or 2 for surround.
     * @return The localized label, or `NONE` for another value.
     * @ghidraAddress NTSC-U/C: 0x0016bc80
     * @ghidraAddress PAL: 0x0016ee08
     */
    const char *OutputModeLabel(int nOutputMode) const;

    /**
     * Change the setting of the component a button starts to choose, or leave with `done`.
     *
     * The directional buttons step the display size and the output mode, and the other components
     * turn their toggle over.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x0016bd10
     * @ghidraAddress PAL: 0x0016ee98
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Go back to the screen the menu was entered from on Triangle, unless the screen is between
     * screens.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleJoypad(), or true between screens.
     * @ghidraAddress NTSC-U/C: 0x0016bf98
     * @ghidraAddress PAL: 0x0016f120
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mFreqSize;           /*!< The size of the track display, one of GameOptions::FreqSize. */
    int mOutputMode;         /*!< The speaker output mode. */
    unsigned char mToggles;  /*!< The bits of Toggle that are on. */
    UIScreen *mReturnScreen; /*!< The screen the menu was entered from. */
};
