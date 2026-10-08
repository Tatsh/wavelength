#pragma once

#include <list>

#include "gfx/gfxtunnel.h"
#include "gfx/tnlgeom.h"
#include "gfx/tnltrackrange.h"
#include "math/transform.h"
#include "math/vector3.h"
#include "rnd/mesh.h"
#include "rnd/multimesh.h"
#include "script/dataarray.h"

/**
 * The spotlights over the gems ahead of the song, with a glow at the base of each.
 *
 * The class is not polymorphic. The type of its list of lights records the name. The object is
 * 0x1c bytes. A spotlight is an instance of a multimesh until it comes within the fade ticks of
 * the song, when it is drawn alone with an environment that fades it in.
 */
class SpotLightFX {
public:
    /** One spotlight, 0x60 bytes. */
    class LightPos {
    public:
        /**
         * Report whether a light comes before a tick, the order of the list.
         *
         * @param pos The light.
         * @param flTick The tick.
         * @return Whether the light is earlier.
         */
        static bool Before(const LightPos &pos, float flTick) {
            return pos.mTick < flTick;
        }

        float mTick;                          /*!< The tick of the gem lit. */
        char mTrack;                          /*!< The track of the gem. */
        float mLateral;                       /*!< The position across the track, 0 to 1. */
        Transform mReserved10;                // +0x10 Copied in, never read.
        int mSpotActive;                      /*!< Whether mSpot is an instance of mSpots. */
        std::list<Transform>::iterator mSpot; /*!< The transform of the spotlight. */
        std::list<Transform>::iterator mGlow; /*!< The transform of the glow. */
    };

    /**
     * Find the meshes and create the multimeshes, and fill the pool of transforms.
     *
     * @ghidraAddress NTSC-U/C: 0x001f22d8
     * @ghidraAddress PAL: 0x001fb078
     */
    SpotLightFX();

    /**
     * Release the multimeshes.
     *
     * @ghidraAddress NTSC-U/C: 0x001f26c8
     * @ghidraAddress PAL: 0x001fb468
     */
    ~SpotLightFX();

    /**
     * Load the fade ticks and the scales of the spotlights.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel. The routine does not read it.
     * @param nOption The fourth argument every loader of the tunnel receives. The routine does not
     *                read it.
     * @ghidraAddress NTSC-U/C: 0x001f2038
     * @ghidraAddress PAL: 0x001fadd8
     */
    static void
    LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption);

    /**
     * Advance the spotlights. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x001f2788
     * @ghidraAddress PAL: 0x001fb528
     */
    void Poll();

    /**
     * Drop the spotlights behind the song, and draw the glows and the spotlights.
     *
     * @ghidraAddress NTSC-U/C: 0x001f2790
     * @ghidraAddress PAL: 0x001fb530
     */
    void Draw();

    /**
     * Light a gem, unless a light of the same tick and track exists.
     *
     * @param nTrack The track.
     * @param pGeom The geometry of the tunnel.
     * @param flTick The tick of the gem.
     * @param flLateral The position across the track, 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001f2a58
     * @ghidraAddress PAL: 0x001fb7f8
     */
    void AddLight(char nTrack, TnlGeom *pGeom, float flTick, float flLateral);

    /**
     * Remove the lights of a track over one tick.
     *
     * @param nTrack The track.
     * @param flTick The tick.
     * @ghidraAddress NTSC-U/C: 0x001f2df0
     * @ghidraAddress PAL: 0x001fbb90
     */
    void RemoveLight(char nTrack, float flTick);

    /**
     * Remove the lights of a track over a span of ticks.
     *
     * @param nTrack The track.
     * @param flStartTick The start of the span.
     * @param flEndTick The end of the span.
     * @ghidraAddress NTSC-U/C: 0x001f2e20
     * @ghidraAddress PAL: 0x001fbbc0
     */
    void RemoveLights(char nTrack, float flStartTick, float flEndTick);

    /**
     * Remove every light.
     *
     * @ghidraAddress NTSC-U/C: 0x001f2f40
     * @ghidraAddress PAL: 0x001fbce0
     */
    void Clear();

    /**
     * Place again the lights in a range of the tunnel that changed.
     *
     * @param pGeom The geometry of the tunnel.
     * @param pRange The range.
     * @ghidraAddress NTSC-U/C: 0x001f2fe0
     * @ghidraAddress PAL: 0x001fbd80
     */
    void UpdateRange(TnlGeom *pGeom, const TnlTrackRange *pRange);

    std::list<Transform> mTransforms; /*!< The pool of free instance transforms. */
    Rnd::Mesh *mMesh;                 /*!< The spotlight, `gem spotlight.mesh`. */
    Rnd::MultiMesh *mSpots;           /*!< The spotlights, `gem spotlight.mm`. */
    Rnd::MultiMesh *mGlows;           /*!< The glows, `gem base glow.mm`. */
    std::list<LightPos> mLights;      /*!< The lights, sorted by tick. */

private:
    /**
     * Return both transforms of a light to the pool.
     *
     * @param light The light.
     * @ghidraAddress NTSC-U/C: 0x001f2160
     * @ghidraAddress PAL: 0x001faf00
     */
    void Release(std::list<LightPos>::iterator light);

    /**
     * Return the spotlight instance of a light to the pool, to draw it alone.
     *
     * The light still reads its transform from the pool.
     *
     * @param light The light.
     * @ghidraAddress NTSC-U/C: 0x001f2238
     * @ghidraAddress PAL: 0x001fafd8
     */
    void DetachSpot(std::list<LightPos>::iterator light);

    /**
     * Place the glow and the spotlight of a light on its track.
     *
     * @param pLight The light.
     * @param pGeom The geometry of the tunnel.
     * @ghidraAddress NTSC-U/C: 0x001f30e8
     * @ghidraAddress PAL: 0x001fbe88
     */
    static void Place(LightPos *pLight, TnlGeom *pGeom);
};
