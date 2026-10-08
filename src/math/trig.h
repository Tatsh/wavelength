#pragma once

/**
 * Approximate the sine of an angle.
 *
 * The angle is reduced to a quarter turn with the multiple of a half pi below it, and the
 * remainder is evaluated by a polynomial. The name is inferred.
 *
 * @param fAngle The angle, in radians.
 * @return The sine.
 * @ghidraAddress NTSC-U/C: 0x00293860
 * @ghidraAddress PAL: 0x0029d228
 */
float Sin(float fAngle);

/**
 * Look up the sine of an angle in a table of 256 steps a turn.
 *
 * The name is inferred.
 *
 * @param fAngle The angle, in radians.
 * @return The sine of the nearest step.
 * @ghidraAddress NTSC-U/C: 0x00293958
 * @ghidraAddress PAL: 0x0029d320
 */
float SinLookup(float fAngle);
