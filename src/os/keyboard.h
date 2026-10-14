#pragma once

#include "app/msgsink.h"

/** Keys KeyboardKeyMsg reports for keys without a character. */
enum KeyboardKeyCode {
    kKeyNone = 0,            /*!< No key. */
    kKeyCapsLock = 0x122,    /*!< Caps Lock. */
    kKeyNumLock = 0x123,     /*!< Num Lock. */
    kKeyScrollLock = 0x124,  /*!< Scroll Lock. */
    kKeyPrintScreen = 0x12c, /*!< Print Screen. */
    kKeyPause = 0x12d,       /*!< Pause. */
    kKeyEscape = 0x12e,      /*!< Escape. */
    kKeyInsert = 0x136,      /*!< Insert. */
    kKeyDelete = 0x137,      /*!< Delete. */
    kKeyHome = 0x138,        /*!< Home. */
    kKeyEnd = 0x139,         /*!< End. */
    kKeyPageUp = 0x13a,      /*!< Page Up. */
    kKeyPageDown = 0x13b,    /*!< Page Down. */
    kKeyLeft = 0x140,        /*!< The left arrow. */
    kKeyRight = 0x141,       /*!< The right arrow. */
    kKeyUp = 0x142,          /*!< The up arrow. */
    kKeyDown = 0x143,        /*!< The down arrow. */
    kKeyF1 = 0x191           /*!< F1, the first of the twelve function keys. */
};

/**
 * Start the USB keyboard library and set up keyboard 0.
 *
 * @ghidraAddress NTSC-U/C: 0x0028b148
 * @ghidraAddress PAL: 0x00294940
 */
void KeyboardInit();

/**
 * Stop the USB keyboard library.
 *
 * @ghidraAddress NTSC-U/C: 0x0028b1c8
 * @ghidraAddress PAL: 0x002949c0
 */
void KeyboardTerminate();

/**
 * Advance the keyboard's request cycle and send a KeyboardKeyMsg for a key pressed.
 *
 * Without a keyboard, the poll checks for one every hundred calls. With one, each call that finds
 * the previous request finished starts the next read.
 *
 * @ghidraAddress NTSC-U/C: 0x0028b1f0
 * @ghidraAddress PAL: 0x002949e8
 */
void KeyboardPoll();

/**
 * Add a sink to the receivers of the USB keyboard messages.
 *
 * The name is inferred from JoypadAddSink(), which it mirrors.
 *
 * @param pSink The sink.
 * @ghidraAddress NTSC-U/C: 0x0028b2c8
 * @ghidraAddress PAL: 0x00294ac0
 */
void KeyboardAddSink(MsgSink *pSink);

/**
 * Remove a sink from the receivers of the USB keyboard messages.
 *
 * @param pSink The sink.
 * @ghidraAddress NTSC-U/C: 0x0028b2f0
 * @ghidraAddress PAL: 0x00294ae8
 */
void KeyboardRemoveSink(MsgSink *pSink);
