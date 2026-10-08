#pragma once

#include "app/msgsink.h"

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
