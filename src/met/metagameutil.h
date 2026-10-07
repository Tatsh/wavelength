#pragma once

#include "rnd/mat.h"

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

/**
 * Find the material of a grade, `grade_0<grade>.mat`, or `grade_end_0<grade>.mat` for the end of
 * a song.
 *
 * The routine reads no object. The name is inferred.
 *
 * @param nGrade The grade.
 * @param bEnd Whether the material of the end of a song is found.
 * @return The material, or null.
 * @ghidraAddress NTSC-U/C: 0x00199a08
 * @ghidraAddress PAL: 0x001a0f48
 */
Rnd::Mat *FindGradeMaterial(int nGrade, bool bEnd);
