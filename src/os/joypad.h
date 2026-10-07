#pragma once

/**
 * Turn the analog stick messages of the controller poll on or off.
 *
 * While the setting is off, the poll does not compare stick positions and sends no message for a
 * moved stick. The setting starts off. The name is inferred from the behaviour.
 *
 * @param bEnable Whether moved sticks send messages.
 * @ghidraAddress NTSC-U/C: 0x0028ace0
 * @ghidraAddress PAL: 0x002944d8
 */
void JoypadSetStickMessages(bool bEnable);
