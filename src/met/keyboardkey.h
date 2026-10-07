#pragma once

#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uibutton.h"

/**
 * Key of the front-end keyboard.
 *
 * The RTTI records the class as deriving from UIButton. The object is 0x40 bytes and its vtable
 * is at `0x003d03f8`. The metagame registers the class for the component type `key_comp`. Unlike
 * a button, a key reports its choice as soon as the cross button goes down and flashes once
 * afterwards.
 */
class KeyboardKey : public UIButton {
public:
    /**
     * Construct a key from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the key belongs to.
     * @ghidraAddress NTSC-U/C: 0x00362520
     * @ghidraAddress PAL: 0x003d0a38
     */
    KeyboardKey(DataArray *pData, const char *pszPanel);

    /**
     * Destroy the key.
     *
     * @ghidraAddress NTSC-U/C: 0x00362438
     * @ghidraAddress PAL: 0x003d0950
     */
    ~KeyboardKey() override;

    /**
     * Create a key from its script description.
     *
     * The metagame registers the routine for the component type `key_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the key belongs to.
     * @return The new key.
     * @ghidraAddress NTSC-U/C: 0x003624d0
     * @ghidraAddress PAL: 0x003d09e8
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new KeyboardKey(pData, pszPanel);
    }

    /**
     * Route controller buttons, and pass every other message to UIButton.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001a8aa0
     * @ghidraAddress PAL: 0x001b0790
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Report the choice as the cross button goes down, and flash once when no sink handled it.
     *
     * The key must not be flashing already.
     *
     * @param pMsg The message.
     * @return True for a press of the cross button, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x001a8948
     * @ghidraAddress PAL: 0x001b0638
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Flash once, as when the key is chosen. KeyboardPanel flashes the key a controller button
     * stands for.
     *
     * @ghidraAddress NTSC-U/C: 0x001a8a50
     * @ghidraAddress PAL: 0x001b0740
     */
    void Flash();

    /**
     * Number of flashes of a chosen key.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8e0
     */
    static int sNumFlashes;

    /**
     * Milliseconds of each flash a chosen key spends in the selected state.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8e4
     */
    static float sSelectedMs;

    /**
     * Milliseconds of each flash a chosen key spends in the normal state.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8e8
     */
    static float sNormalMs;
};
