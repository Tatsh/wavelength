#pragma once

#include <vector>

#include "math/vector2.h"
#include "rnd/animatable.h"
#include "rnd/mesh.h"
#include "rnd/particlesys.h"
#include "rnd/transformable.h"
#include "rnd/view.h"
#include "script/dataarray.h"

// Helpers the display shares between its effects. The names are inferred.

/**
 * Take a view out of the draws, the animations, the transforms, and the collisions of another.
 *
 * @param pParent The view to take the child out of.
 * @param pChild The child, or null.
 * @ghidraAddress NTSC-U/C: 0x001e1988
 * @ghidraAddress PAL: 0x001ea728
 */
void DetachView(Rnd::View *pParent, Rnd::View *pChild);

/**
 * Add a view to the draws, the animations, and the transforms of another.
 *
 * @param pParent The view to add the child to.
 * @param pChild The child, or null.
 * @ghidraAddress NTSC-U/C: 0x001e19f0
 * @ghidraAddress PAL: 0x001ea790
 */
void AttachView(Rnd::View *pParent, Rnd::View *pChild);

/**
 * Change the rate of the scale-offset filter in the first slot of an animation, keeping the
 * filtered frame of its current frame.
 *
 * A warning is reported for an animation whose first filter is missing or is another kind.
 *
 * @param pAnim The animation.
 * @param fScale The new rate.
 * @ghidraAddress NTSC-U/C: 0x001e1a50
 * @ghidraAddress PAL: 0x001ea7f0
 */
void SetFilterScale(Rnd::Animatable *pAnim, float fScale);

/**
 * Turn a point about the origin.
 *
 * @param pPoint The point, which receives the result.
 * @param fAngle The angle, in radians.
 * @ghidraAddress NTSC-U/C: 0x001e1b70
 * @ghidraAddress PAL: 0x001ea910
 */
void Rotate(Vector2 *pPoint, float fAngle);

/**
 * Place the cross section points of the tracks along an arc that is symmetric about the Y axis.
 *
 * The configuration gives `center_height`, `track_width`, `start_angle`, `angle_inc`, and
 * `angle_inc_inc`, the angles in degrees. The middle point sits at the centre height, and each
 * point outward is one track width from the last, turned by an angle that grows by the increment.
 *
 * @param pPoints The points, sized already.
 * @param pConfig The configuration.
 * @ghidraAddress NTSC-U/C: 0x001e1bf0
 * @ghidraAddress PAL: 0x001ea990
 */
void BuildTrackArc(std::vector<Vector2> *pPoints, DataArray *pConfig);

/**
 * Give every transform under a transform the identity as its local transform.
 *
 * @param pRoot The transform.
 * @param bReset Whether the transform itself is reset as well as the transforms under it.
 * @ghidraAddress NTSC-U/C: 0x001e1e48
 * @ghidraAddress PAL: 0x001eabe8
 */
void ResetXfmTree(Rnd::Transformable *pRoot, bool bReset);

/**
 * Set the depth test of every mesh under a transform, recursively.
 *
 * @param pRoot The transform.
 * @param zMode The depth mode.
 * @param zFunc The depth comparison.
 * @ghidraAddress NTSC-U/C: 0x001e1f10
 * @ghidraAddress PAL: 0x001eacb0
 */
void SetZModeTree(Rnd::Transformable *pRoot, Rnd::Mesh::ZMode zMode, Rnd::Mesh::ZFunc zFunc);

/**
 * Scale the translation keys of a transform animation.
 *
 * @param pszName The animation, which must exist.
 * @param fScale The scale.
 * @ghidraAddress NTSC-U/C: 0x001e3c08
 * @ghidraAddress PAL: 0x001ec9a8
 */
void ScaleTransKeys(const char *pszName, float fScale);

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
 * Blend the rotations of two transforms through quaternions.
 *
 * @param pFrom The transform at a blend of 0, four rows of four floats.
 * @param pTo The transform at a blend of 1, four rows of four floats.
 * @param pOut Receives the blended rotation, four rows of four floats. It may alias pFrom.
 * @param fBlend The blend.
 * @ghidraAddress NTSC-U/C: 0x001e2b38
 * @ghidraAddress PAL: 0x001eb8d8
 */
void InterpBasis(const float *pFrom, const float *pTo, float *pOut, float fBlend);

/**
 * Give every mesh under a transform a bounding sphere at the origin of radius 0, the mark of a
 * mesh with no sphere, recursively.
 *
 * @param pRoot The transform.
 * @ghidraAddress NTSC-U/C: 0x001e1fc8
 * @ghidraAddress PAL: 0x001ead68
 */
void ClearMeshSpheres(Rnd::Transformable *pRoot);

/**
 * Scale the size and the force of the particles of a particle system.
 *
 * @param pSys The particle system.
 * @param fScale The scale.
 * @ghidraAddress NTSC-U/C: 0x001e2138
 * @ghidraAddress PAL: 0x001eaed8
 */
void ScaleParticles(Rnd::ParticleSys *pSys, float fScale);

/**
 * Scale every particle system of a transform tree with ScaleParticles().
 *
 * @param pRoot The root of the tree, or null.
 * @param fScale The scale.
 * @ghidraAddress NTSC-U/C: 0x001e2088
 * @ghidraAddress PAL: 0x001eae28
 */
void ScaleParticleTree(Rnd::Transformable *pRoot, float fScale);
