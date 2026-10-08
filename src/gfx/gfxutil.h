#pragma once

#include <vector>

#include "rnd/particlesys.h"
#include "rnd/transformable.h"
#include "script/dataarray.h"

// Helpers the display shares between its effects. The names are inferred.

/**
 * One key of a curve of floats, sorted by frame.
 */
struct FloatKey {
    float mValue; /*!< The value at the frame. */
    float mFrame; /*!< The frame of the key. */
};

/**
 * Read a curve of `(value frame)` pairs from a script array, replacing the keys and sorting them by
 * frame.
 *
 * @param pData The array.
 * @param pKeys Receives the keys.
 * @ghidraAddress NTSC-U/C: 0x001e3c88
 * @ghidraAddress PAL: 0x001eca28
 */
void LoadFloatKeys(DataArray *pData, std::vector<FloatKey> *pKeys);

/**
 * Clear the scroll of the textures of every mesh under a transform, recursively.
 *
 * @param pRoot The transform, or null.
 * @ghidraAddress NTSC-U/C: 0x001e1fc8
 * @ghidraAddress PAL: 0x001ead68
 */
void ClearMeshScroll(Rnd::Transformable *pRoot);

/**
 * Scale the size and the force of the particles of a particle system.
 *
 * @param pSys The particle system.
 * @param fScale The scale.
 * @ghidraAddress NTSC-U/C: 0x001e2138
 * @ghidraAddress PAL: 0x001eaed8
 */
void ScaleParticles(Rnd::ParticleSys *pSys, float fScale);
