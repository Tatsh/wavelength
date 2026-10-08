#pragma once

#include "rnd/mesh.h"
#include "rnd/meshanim.h"

/**
 * Add the keys of a mesh animation that turn the vertices of a mesh about its Z axis.
 *
 * The keys are spread evenly over the frames and the angles. The name is inferred.
 *
 * @param pMesh The mesh whose vertices turn.
 * @param pAnim The animation that receives the keys.
 * @param nKeys The number of keys.
 * @param flStartFrame The frame of the first key.
 * @param flStartAngle The angle of the first key, in radians.
 * @param flEndFrame The frame of the last key.
 * @param flEndAngle The angle of the last key, in radians.
 * @ghidraAddress NTSC-U/C: 0x001e2190
 * @ghidraAddress PAL: 0x001eaf30
 */
void AddRotationKeys(Rnd::Mesh *pMesh,
                     Rnd::MeshAnim *pAnim,
                     int nKeys,
                     float flStartFrame,
                     float flStartAngle,
                     float flEndFrame,
                     float flEndAngle);
