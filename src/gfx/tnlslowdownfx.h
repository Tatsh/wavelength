#pragma once

#include "gfx/gfxtunnel.h"
#include "math/interpolator.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndmesh.h"
#include "rnd/rndtransanim.h"
#include "script/dataarray.h"

/**
 * The overlay of a slowdown, a mesh over the screen whose animations run from the deployment of
 * the slowdown to the tick the song is back at its speed.
 *
 * The class is not polymorphic, and the name is inferred from the `slowdown_shell` entry the
 * tunnel creates it with and from the `slowdown overlay` objects it finds. The object is 0x30
 * bytes.
 */
class TnlSlowdownFX {
public:
    /**
     * Find the objects of the overlay, scale its animation and place its mesh to cover the screen,
     * and hide it.
     *
     * @param pszName The entry of the overlay in the configuration of the tunnel.
     * @param pTunnel The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001d6058
     * @ghidraAddress PAL: 0x001dedf8
     */
    TnlSlowdownFX(const char *pszName, GfxTunnel *pTunnel);

    /**
     * Destroy the overlay.
     *
     * @ghidraAddress NTSC-U/C: 0x001d6280
     * @ghidraAddress PAL: 0x001df020
     */
    ~TnlSlowdownFX();

    /**
     * Load the settings of the overlay. The overlay has none, and the routine does nothing.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel.
     * @param nOption The fourth argument every loader of the tunnel receives.
     * @ghidraAddress NTSC-U/C: 0x001d62b0
     * @ghidraAddress PAL: 0x001df050
     */
    void LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption);

    /**
     * Hide the overlay and stop its animation.
     *
     * @ghidraAddress NTSC-U/C: 0x001d62b8
     * @ghidraAddress PAL: 0x001df058
     */
    void Reset();

    /**
     * Show the overlay and run its animation from the song tick to the end of a slowdown.
     *
     * @param flStartTick The tick the song reaches the slow speed. The overlay does not read it.
     * @param flEndTick The tick the song starts to speed up again. The overlay does not read it.
     * @param flStopTick The tick the song is back at its speed.
     * @ghidraAddress NTSC-U/C: 0x001d6320
     * @ghidraAddress PAL: 0x001df0c0
     */
    void Start(float flStartTick, float flEndTick, float flStopTick);

    /**
     * Move the animations of the overlay to the song tick, unless the slowdown has ended.
     *
     * @ghidraAddress NTSC-U/C: 0x001d6380
     * @ghidraAddress PAL: 0x001df120
     */
    void Update();

    /**
     * Draw the mesh of the overlay.
     *
     * @ghidraAddress NTSC-U/C: 0x001d6410
     * @ghidraAddress PAL: 0x001df1b0
     */
    void Draw();

private:
    GfxTunnel *mTunnel;         /*!< The tunnel. */
    RndAnimatable *mAnim;       /*!< The animation of the material, `slowdown overlay.mnm`. */
    LinearInterpolator mFrames; /*!< The frame of mAnim at each tick, 0 to 1000. */
    RndTransAnim *mTransAnim;   /*!< The animation of the mesh, `slowdown overlay.tnm`. */
    RndMesh *mMesh;             /*!< The mesh, `slowdown overlay.mesh`. */
    const char *mName;          /*!< The entry of the overlay in the configuration. */
};
