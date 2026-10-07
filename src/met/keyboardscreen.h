#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"

/**
 * Screen of the front-end keyboard, `kb_screen`, whose KeyboardPanel types a text.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x70 bytes and its vtable
 * is at `0x003cbfa8`. The metagame registers the class for the screen type `keyboard_screen`. The
 * keys play the sounds. The screen plays none for the controller buttons.
 */
class KeyboardScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00355178
     * @ghidraAddress PAL: 0x003c2428
     */
    explicit KeyboardScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00355318
     * @ghidraAddress PAL: 0x003c25c8
     */
    ~KeyboardScreen() override;

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the entry type `keyboard_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00354ed8
     */
    static UIScreen *New(DataArray *pData) {
        return new KeyboardScreen(pData);
    }

    /**
     * Play no sound for a controller button.
     *
     * @param nButton One of JoypadButton.
     * @ghidraAddress NTSC-U/C: 0x003553b0
     * @ghidraAddress PAL: 0x003c2660
     */
    void PlayButtonSound([[maybe_unused]] int nButton) override {
    }
};
