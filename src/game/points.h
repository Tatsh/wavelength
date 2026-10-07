#pragma once

#include "game/gemcursor.h"

/**
 * Report the points of a gem from the `points` configuration.
 *
 * The first entry of the gem table whose divisor divides the tick applies. The routines of the
 * file use no object, and the file name is inferred.
 *
 * @param nTick The song tick of the gem.
 * @return The points, or 0 with a warning when no divisor divides the tick.
 * @ghidraAddress NTSC-U/C: 0x00128658
 * @ghidraAddress PAL: 0x00129e50
 */
int GetGemPoints(int nTick);

/**
 * Report the points of the gems of a phrase, divided by the phrase scale and rounded up.
 *
 * @param cursor The first gem of the phrase.
 * @param nStartBar The first bar of the phrase. It is unused.
 * @param nEndBar The bar after the phrase.
 * @param nTicksPerBar The song ticks in one bar.
 * @return The points.
 * @ghidraAddress NTSC-U/C: 0x001286c8
 * @ghidraAddress PAL: 0x00129ec0
 */
int GetPhrasePoints(const GemCursor &cursor, int nStartBar, int nEndBar, int nTicksPerBar);
