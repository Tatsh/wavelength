#pragma once

#include "game/avatarpartset.h"
#include "os/string.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"

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

/**
 * Render an avatar into the part of the screen a mesh covers, and draw the mesh with the winner's
 * material while the avatar plays a `win` animation.
 *
 * The routine belongs to the unit of front end helpers. The name is inferred.
 *
 * @param pAvatar The avatar.
 * @param pMesh The mesh whose bounding box sets the part of the screen.
 * @ghidraAddress NTSC-U/C: 0x00199ab8
 * @ghidraAddress PAL: 0x001a0ff8
 */
void DrawAvatarOnMesh(AvatarPartSet *pAvatar, Rnd::Mesh *pMesh);

/**
 * Remove the spaces at the start and at the end of a string.
 *
 * The routine belongs to the unit of front end helpers. The name is inferred.
 *
 * @param pText The string.
 * @return True when spaces were removed.
 * @ghidraAddress NTSC-U/C: 0x00199cb0
 * @ghidraAddress PAL: 0x001a11f0
 */
bool TrimSpaces(String *pText);
