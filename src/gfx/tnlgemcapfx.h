#pragma once

#include "gfx/gfxtunnel.h"
#include "gfx/tnlgeom.h"
#include "math/interpolator.h"
#include "math/transform.h"
#include "math/vector3.h"
#include "rnd/view.h"

/**
 * A burst of a captured or erased gem, an animated view that spins along its track.
 *
 * The RTTI identifies the class, which is not polymorphic. The object is 0xc0 bytes. The tunnel
 * keeps a pool of them.
 */
class TnlGemCapFX {
public:
    /**
     * Construct an idle burst.
     *
     * @ghidraAddress NTSC-U/C: 0x001ef1c8
     * @ghidraAddress PAL: 0x001f7f68
     */
    TnlGemCapFX();

    /**
     * Destroy the burst.
     *
     * @ghidraAddress NTSC-U/C: 0x001ef250
     * @ghidraAddress PAL: 0x001f7ff0
     */
    ~TnlGemCapFX();

    /**
     * Start the burst at a gem.
     *
     * The burst turns to one of seven quarter turns at random, offset by the time, and a captured
     * gem moves forward along its track while it bursts.
     *
     * @param pTunnel The tunnel. The routine does not read it.
     * @param nTrack The track of the gem.
     * @param pView The view the burst animates.
     * @param flTick The tick of the gem.
     * @param flLateral The position of the gem across its track, from 0 to 1.
     * @param pScale The scale of each axis of the burst.
     * @param bErase Whether the gem is erased rather than captured.
     * @ghidraAddress NTSC-U/C: 0x001ef288
     * @ghidraAddress PAL: 0x001f8028
     */
    void Start(GfxTunnel *pTunnel,
               char nTrack,
               Rnd::View *pView,
               float flTick,
               float flLateral,
               const Vector3 *pScale,
               bool bErase);

    /**
     * Stop the burst at once.
     *
     * @ghidraAddress NTSC-U/C: 0x001ef498
     * @ghidraAddress PAL: 0x001f8238
     */
    void Stop();

    /**
     * Place, animate, and draw the burst.
     *
     * @param pGeom The geometry of the tracks.
     * @return Whether the burst ended during the call.
     * @ghidraAddress NTSC-U/C: 0x001ef4b0
     * @ghidraAddress PAL: 0x001f8250
     */
    bool Draw(TnlGeom *pGeom);

    /**
     * Report whether the burst is idle.
     *
     * @return Whether the burst is idle.
     * @ghidraAddress NTSC-U/C: 0x001ef6a8
     * @ghidraAddress PAL: 0x001f8448
     */
    bool IsIdle() const;

    /**
     * The frames of the animation of the view over a burst.
     *
     * @ghidraAddress NTSC-U/C: 0x003afb08
     */
    static float sAnimFrames;

    /**
     * The length of a capture burst, then of an erase burst, in milliseconds.
     *
     * @ghidraAddress NTSC-U/C: 0x003afb10
     */
    static float sLengths[2];

    /**
     * The ticks a capture burst moves forward, then those of an erase burst.
     *
     * @ghidraAddress NTSC-U/C: 0x003afb18
     */
    static float sTravels[2];

    char mTrack;                    /*!< The track of the burst, or -1 for an idle burst. */
    Rnd::View *mView;               /*!< The view the burst animates. */
    Transform mSpin;                /*!< The turn and the scale of the burst. */
    Transform mXfm;                 /*!< The placed transform of the burst. */
    float mLateral;                 /*!< The position of the burst across its track. */
    LinearInterpolator mFrameCurve; /*!< The frame of the animation over time. */
    LinearInterpolator mTickCurve;  /*!< The tick of the burst over time. */
};
