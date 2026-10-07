#pragma once

/**
 * Find the `freq_winner.mat` material the result screens draw the winner's character with, and
 * keep it for them.
 *
 * The routine belongs to a unit of front end helpers that keeps its objects in file-level state.
 * The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x001993e0
 * @ghidraAddress PAL: 0x001a0900
 */
void FindWinnerMaterial();
