#pragma once

/**
 * Mix-in for a screen that receives the text the player types on the front-end keyboard.
 *
 * The RTTI records the class with no base. The subobject is the four-byte vptr. The routines of
 * the class are not reconstructed.
 */
class KeyboardUser {
public:
    /**
     * Destroy the mix-in.
     *
     * @ghidraAddress NTSC-U/C: 0x00359c58
     * @ghidraAddress PAL: 0x003c6fc0
     */
    virtual ~KeyboardUser();

    /**
     * Receive the text the player typed.
     *
     * @param pszText The text.
     * @return Non-zero when the text was not accepted.
     */
    virtual int ReceiveKeyboardText(const char *pszText) = 0;
};
