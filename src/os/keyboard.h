#pragma once

#include "app/msgsink.h"

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
