#pragma once

/**
 * Play the song at a speed.
 *
 * Scales the clock of the song scheduler and bends the pitch of the synthesiser channels to match.
 * The name is inferred.
 *
 * @param fSpeed The speed, 1 for the normal speed.
 * @ghidraAddress NTSC-U/C: 0x0013bbc8
 * @ghidraAddress PAL: 0x0013d498
 */
void SetSongSpeed(float fSpeed);
