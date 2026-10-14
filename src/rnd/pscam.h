#pragma once

#include <cstddef>

#include "math/frustum.h"
#include "math/transform.h"
#include "math/vector3.h"
#include "os/mem.h"
#include "rnd/rndcam.h"

/**
 * Camera, PlayStation 2.
 *
 * The RTTI includes the class name and records RndCam as the one base. The class adds no data
 * member. DrawShowing() programs the GS scissor and fills the transforms the mesh and particle
 * paths read.
 */
class PsCam : public RndCam {
public:
    /**
     * Construct a camera with the default projection.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00212648
     * @ghidraAddress PAL: 0x0021b460
     */
    explicit PsCam(const char *pszName);

    /**
     * @ghidraAddress NTSC-U/C: 0x002127a0
     * @ghidraAddress PAL: 0x0021b5b8
     */
    ~PsCam() override;

    /**
     * Allocate a camera, billed to the base class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Cam", 0);
    }

    /**
     * Release a camera.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Make this camera the one the frame is drawn through.
     *
     * A camera following one that drew into a texture flushes the texture cache first. A render
     * target is bound with PsTex::BindAsRenderTarget(), and otherwise a camera following one that
     * drew into a texture points the GS back at the display buffer. The routine then computes the
     * guard band, widens the four side planes of the local frustum by it, moves all six planes
     * into world space as g_drawFrustum, builds the viewport and projection transforms, and
     * programs SCISSOR_1 from the screen rectangle clamped to the unit square. A render target is
     * then cleared to black with depth writes enabled. The camera becomes RndCam::sCurrent. The
     * base implementation is not called.
     *
     * @return 1, to draw the children.
     * @ghidraAddress NTSC-U/C: 0x002128f0
     * @ghidraAddress PAL: 0x0021b708
     */
    int DrawShowing() override;

    /**
     * Install the camera factory and build the default camera.
     *
     * A `.rnd` file naming the class `Cam` then loads this subclass. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00212548
     * @ghidraAddress PAL: 0x0021b360
     */
    static void Init();

    /**
     * Create a camera.
     *
     * @param pszName The registry key.
     * @return The camera.
     * @ghidraAddress NTSC-U/C: 0x0037ca10
     * @ghidraAddress PAL: 0x003eb140
     */
    static RndObject *New(const char *pszName) {
        return new PsCam(pszName);
    }
};

// The globals below are the draw state PsCam::DrawShowing() sets up for the mesh and particle
// paths. Every title is inferred from the computation that fills the global.

/**
 * Reciprocal of the factor the view is widened by on each axis before clipping.
 *
 * Each axis of the factor is the room left in the 4096-unit GS coordinate space around the screen
 * rectangle, divided by the rectangle's extent with a 2 per cent margin. Primitives inside the
 * widened view need no clipping, because the GS scissors them. The third component is 1.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c180
 */
extern Vector3 g_invGuardBandScale;

/**
 * The widened projection scale on each axis, with y negated, and two zero components.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c190
 */
extern Vector3 g_particleProjectScale;

/**
 * The view volume widened by the guard band, in world space.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c1a0
 */
extern Frustum g_drawFrustum;

/**
 * World to clip transform without the guard band widening.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c200
 */
extern Transform g_viewProjectUnscaledXfm;

/**
 * World to clip transform, the projection widened by the guard band.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c240
 */
extern Transform g_viewProjectXfm;

/**
 * g_viewProjectXfm with the depth of a perspective projection moved one depth unit nearer.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c280
 */
extern Transform g_viewProjectBiasedXfm;

/**
 * Clip to GS coordinate transform without the guard band scaling.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c2c0
 */
extern Transform g_viewportUnscaledXfm;

/**
 * Clip to GS coordinate transform, scaled by the guard band.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c300
 */
extern Transform g_viewportXfm;
