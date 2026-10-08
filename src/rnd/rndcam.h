#pragma once

#include <list>

#include "math/frustum.h"
#include "math/rect.h"
#include "math/transform.h"
#include "math/vector2.h"
#include "rnd/rndcollideable.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndtex.h"
#include "rnd/rndtransformable.h"

/**
 * Camera.
 *
 * The RTTI includes the class name and records RndDrawable, RndTransformable, and RndCollideable as
 * bases. The camera projects onto mScreenRect, a rectangle in fractions of the display or of
 * mTargetTex. A field of view of 0 selects an orthographic projection.
 */
class RndCam : public RndDrawable, public RndTransformable, public RndCollideable {
public:
    /**
     * Construct a camera looking along y with a quarter-turn field of view, from 1 to 1000 units.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00220f20
     * @ghidraAddress PAL: 0x00229cf0
     */
    explicit RndCam(const char *pszName);

    /**
     * Release the camera and stop drawing through it.
     *
     * @ghidraAddress NTSC-U/C: 0x00221498
     * @ghidraAddress PAL: 0x0022a268
     */
    ~RndCam() override;

    /**
     * Add the camera itself to a list.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x00221e60
     * @ghidraAddress PAL: 0x0022ac30
     */
    void ListDrawObjects(std::list<RndObject *> &objects) override;

    /**
     * Make the camera current and add the drawables of the children to a list.
     *
     * @param drawables The list to add to.
     * @ghidraAddress NTSC-U/C: 0x00221ee8
     * @ghidraAddress PAL: 0x0022acb8
     */
    void ListDrawables(std::list<RndDrawable *> &drawables) override;

    /**
     * Make the camera current.
     *
     * @return 1, to draw the children.
     * @ghidraAddress NTSC-U/C: 0x0037f230
     * @ghidraAddress PAL: 0x003ed948
     */
    int DrawShowing() override;

    /**
     * Recompute the world transform, and the world projection when it changed.
     *
     * @param pParent The parent, or null for a root.
     * @param bForce Non-zero to recompute even when nothing changed.
     * @return Non-zero when the world transform was recomputed.
     * @ghidraAddress NTSC-U/C: 0x0037f1f0
     * @ghidraAddress PAL: 0x003ed908
     */
    int UpdateWorldXfm(RndTransformable *pParent, int bForce) override;

    using RndCollideable::Collide;

    /**
     * Record the camera as struck when a screen point falls inside mScreenRect, then test the
     * children.
     *
     * @param point The screen point, in fractions of the display.
     * @param collisions The list to add to.
     * @ghidraAddress NTSC-U/C: 0x002205f8
     * @ghidraAddress PAL: 0x002293c8
     */
    void Collide(const Vector2 &point, std::list<Collision> &collisions) override;

    /**
     * Write a description of the camera and its bases.
     *
     * A dump level above 1 adds the projections and the view volumes.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00220738
     * @ghidraAddress PAL: 0x00229508
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the camera and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00220960
     * @ghidraAddress PAL: 0x00229730
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a referenced object with another.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00220e48
     * @ghidraAddress PAL: 0x00229c18
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x0037f268
     * @ghidraAddress PAL: 0x003ed980
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another camera and its bases.
     *
     * @param pSource The camera to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x00220d38
     * @ghidraAddress PAL: 0x00229b08
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00220ae8
     * @ghidraAddress PAL: 0x002298b8
     */
    void Load(BinStream &stream) override;

    /**
     * Project a world point to a screen point through mWorldProject and mScreenRect.
     *
     * A point that projects to a depth of exactly 0 skips the perspective divide. The name is
     * inferred.
     *
     * @param world The point in world space.
     * @param screen Receives the point in fractions of the target.
     * @ghidraAddress NTSC-U/C: 0x002204b8
     * @ghidraAddress PAL: 0x00229288
     */
    void WorldToScreen(const Vector3 &world, Vector2 &screen) const;

    /**
     * Render into a texture rather than the display, or stop.
     *
     * @param pTex The texture, or null for the display.
     * @ghidraAddress NTSC-U/C: 0x002205a0
     * @ghidraAddress PAL: 0x00229370
     */
    void SetTargetTex(RndTex *pTex);

    /**
     * Set the view volume.
     *
     * The near plane is kept at least a thousandth of the far plane.
     *
     * @param fNear The distance to the near plane.
     * @param fFar The distance to the far plane.
     * @param fFov The field of view in radians, or 0 for an orthographic projection.
     * @ghidraAddress NTSC-U/C: 0x002218d8
     * @ghidraAddress PAL: 0x0022a6a8
     */
    void SetFrustum(float fNear, float fFar, float fFov);

    /**
     * Rebuild the local projection and the local view volume, then the world projection.
     *
     * @ghidraAddress NTSC-U/C: 0x00221930
     * @ghidraAddress PAL: 0x0022a700
     */
    void UpdateProjection();

    /**
     * Rebuild the world projection and the world view volume from the world transform.
     *
     * The basis is orthonormalised first, so a scaled camera projects without the scale.
     *
     * @ghidraAddress NTSC-U/C: 0x00221ac0
     * @ghidraAddress PAL: 0x0022a890
     */
    void UpdateWorldProject();

    /**
     * Create the internal camera `[default cam]`, 150 units back along y.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00220410
     * @ghidraAddress PAL: 0x002291e0
     */
    static void CreateDefault();

    /**
     * Report the field of view that covers the same extent at another aspect.
     *
     * The name is inferred.
     *
     * @param fFrom The extent the field of view spans.
     * @param fTo The extent the result spans.
     * @param fFov The field of view in radians.
     * @return The field of view in radians.
     * @ghidraAddress NTSC-U/C: 0x00221f08
     * @ghidraAddress PAL: 0x0022acd8
     */
    static float ConvertFov(float fFrom, float fTo, float fFov);

    /**
     * The camera drawing is set up for.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0988
     */
    static RndCam *sCurrent;

    /**
     * The internal camera CreateDefault() made.
     *
     * @ghidraAddress NTSC-U/C: 0x003b098c
     */
    static RndCam *sDefault;

    /**
     * The class name a `.rnd` file writes, `Cam`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0990
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0994
     */
    static int sRev;

    Transform mOrthoWorldXfm;   /*!< The world transform with an orthonormal basis. */
    Transform mInvWorldXfm;     /*!< The inverse of mOrthoWorldXfm. */
    Transform mLocalProject;    /*!< Camera space to the unit projection. */
    Transform mInvLocalProject; /*!< The inverse of mLocalProject. */
    Transform mWorldProject;    /*!< World space to the unit projection. */
    Transform mInvWorldProject; /*!< The inverse of mWorldProject. */
    Frustum mLocalFrustum;      /*!< The view volume in camera space. */
    Frustum mWorldFrustum;      /*!< The view volume in world space. */
    float mNearPlane;           /*!< The distance to the near plane. */
    float mFarPlane;            /*!< The distance to the far plane. */
    float mFov;                 /*!< The field of view in radians, 0 for orthographic. */
    Vector2 mZRange;            /*!< The depth range the projection maps to. */
    Rect mScreenRect;           /*!< The area drawn to, in fractions of the target. */
    RndTex *mTargetTex;         /*!< The texture drawn to, or null for the display. */

protected:
    /**
     * Drop the reference on mTargetTex.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00221df0
     * @ghidraAddress PAL: 0x0022abc0
     */
    void ReleaseTargetTex();

    /**
     * Take a reference on mTargetTex and rebuild the projection.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00221e20
     * @ghidraAddress PAL: 0x0022abf0
     */
    void AcquireTargetTex();
};
