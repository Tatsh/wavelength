#pragma once

/**
 * Build the sine tables.
 *
 * The quarter-wave table has the `sin_table_size` entries of the `math` block of the system
 * configuration (32 by default) and a final 1. A second table of 256 entries covers the full turn
 * for FastSin().
 *
 * @ghidraAddress NTSC-U/C: 0x00293598
 * @ghidraAddress PAL: 0x0029cf60
 */
void SinTableInit();

/**
 * Release the quarter-wave table.
 *
 * @ghidraAddress NTSC-U/C: 0x002937c0
 * @ghidraAddress PAL: 0x0029d188
 */
void SinTableTerminate();

/**
 * Approximate a sine by interpolating the quarter-wave table.
 *
 * @param fRadians The angle.
 * @return The sine.
 * @ghidraAddress NTSC-U/C: 0x00293860
 * @ghidraAddress PAL: 0x0029d228
 */
float SinApprox(float fRadians);

/**
 * Approximate a sine by reading the nearest entry of the full-turn table.
 *
 * @param fRadians The angle.
 * @return The sine.
 * @ghidraAddress NTSC-U/C: 0x00293958
 * @ghidraAddress PAL: 0x0029d320
 */
float FastSin(float fRadians);
