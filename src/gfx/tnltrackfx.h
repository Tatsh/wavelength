#pragma once

#include <list>
#include <vector>

#include "gfx/gfxtunnel.h"
#include "gfx/tnlgem.h"
#include "gfx/tnlgeom.h"
#include "math/interpolator.h"
#include "math/vector2.h"
#include "rnd/mat.h"
#include "rnd/particlesys.h"
#include "script/dataarray.h"

/**
 * The effects of one track as it is enabled or captured, which lift the track and light it with a
 * material drawn over it.
 *
 * The RTTI identifies the class, which is not polymorphic. The object is 0x7c bytes. Track 0
 * draws with `panel enable.mat` and each other track with a copy of it.
 */
class TnlTrackFX {
public:
    /** The values of mState. */
    enum State {
        kStateIdle = 0,    /*!< No effect runs. */
        kStateEnable = 1,  /*!< The track rises and lights up. */
        kStateCapture = 2, /*!< The track drops away and its gems flare. */
    };

    /**
     * Construct the effects of a track, and for track 0 the shared spew of a capture.
     *
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x001ee6d0
     * @ghidraAddress PAL: 0x001f7470
     */
    explicit TnlTrackFX(char nTrack);

    /**
     * Release the shared spew, and the material of a track other than track 0.
     *
     * @ghidraAddress NTSC-U/C: 0x001ee930
     * @ghidraAddress PAL: 0x001f76d0
     */
    ~TnlTrackFX();

    /**
     * Load the settings of the effects of the tracks, the streak arrows, and the cripplers.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel. The routine does not read it.
     * @param nOption The fourth argument every loader of the tunnel receives. The routine does not
     *                read it.
     * @ghidraAddress NTSC-U/C: 0x001ee228
     * @ghidraAddress PAL: 0x001f6fc8
     */
    static void
    LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption);

    /**
     * Start the enable effect.
     *
     * @param pTunnel The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001ee9e0
     * @ghidraAddress PAL: 0x001f7780
     */
    void Enable(GfxTunnel *pTunnel);

    /**
     * Start the capture effect, and spew from the track.
     *
     * @param pTunnel The tunnel.
     * @param flTick The tick the captured phrase ends at.
     * @ghidraAddress NTSC-U/C: 0x001eea88
     * @ghidraAddress PAL: 0x001f7828
     */
    void Capture(GfxTunnel *pTunnel, float flTick);

    /**
     * Stop the effect at once and put the track back.
     *
     * @param pTunnel The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001eecb0
     * @ghidraAddress PAL: 0x001f7a50
     */
    void Stop(GfxTunnel *pTunnel);

    /**
     * Advance the effect.
     *
     * @param pTunnel The tunnel.
     * @param pGeom The geometry of the tracks.
     * @return Whether a capture runs. An enable effect and an idle track report false.
     * @ghidraAddress NTSC-U/C: 0x001eed30
     * @ghidraAddress PAL: 0x001f7ad0
     */
    bool Poll(GfxTunnel *pTunnel, TnlGeom *pGeom);

    /**
     * Report the time the drop of a capture ends.
     *
     * @return The time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001ef1a0
     * @ghidraAddress PAL: 0x001f7f40
     */
    float CaptureEndTime() const;

    /**
     * The milliseconds the enable effect rises, holds lit, and fades, `track_activate_lengths`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afad8
     */
    static float sEnableTimes[3];

    /**
     * The milliseconds the capture drops and falls, then an unused length,
     * `autocapture_anim_times`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afae8
     */
    static float sCaptureTimes[3];

    /**
     * The height a capture raises the track, `autocapture_offset_height` times the scroll speed.
     *
     * @ghidraAddress NTSC-U/C: 0x003afaf4
     */
    static float sCaptureHeight;

    /**
     * The depth an enable effect starts the track at, `track_enable_offset_height` times the
     * scroll speed.
     *
     * @ghidraAddress NTSC-U/C: 0x003afaf8
     */
    static float sEnableHeight;

    /**
     * The ticks ahead of the song a capture spews from, `autocapture_burst_tick`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afafc
     */
    static float sCaptureSpewTicks;

    /**
     * The kick a capture gives the camera, `autocapture_cam_jiggle`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afb00
     */
    static float sCaptureCamJiggle;

    /**
     * The alpha of the overlay of a capture, `autocapture_track_brightness`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afb04
     */
    static float sCaptureBrightness;

    /**
     * The size the gems of a capture flare from and to, `autocapture_flare_size` times the scroll
     * speed.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b5c0
     */
    static Vector2 sFlareSize;

    /**
     * The particle system a capture spews from, `autocapture spew.part`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afb6c
     */
    static Rnd::ParticleSys *sSpew;

    /**
     * The frame of sSpew, which each capture advances.
     *
     * @ghidraAddress NTSC-U/C: 0x003afb70
     */
    static float sSpewFrame;

    char mState;                     /*!< The effect, one of State. */
    char mTrack;                     /*!< The track. */
    char mPhase;                     /*!< The step of the effect, from 1, or 0 for none. */
    float mStartTime;                /*!< The time the effect started, in milliseconds. */
    Rnd::Mat *mMat;                  /*!< The material drawn over the track. */
    LinearInterpolator mEnableCurve; /*!< The step of the enable effect over time. */
    InvExpInterpolator mDropCurve;   /*!< The drop of a capture over time. */
    ExpInterpolator mFallCurve;      /*!< The fall of a capture over time. */
    float mCaptureTick;              /*!< The tick the captured phrase ends at. */
    std::vector<float> mGemTicks;    /*!< The ticks of the gems of the captured phrase. */

private:
    /**
     * Set the size of the glow particle of each mesh gem of a track that has not passed.
     *
     * @param pGems The gems.
     * @param nTrack The track.
     * @param flSize The size.
     * @ghidraAddress NTSC-U/C: 0x001ee158
     * @ghidraAddress PAL: 0x001f6ef8
     */
    static void SetGlowSizes(std::list<TnlGem> *pGems, char nTrack, float flSize);

    /**
     * Put back the size of the glow particle of each mesh gem of a track that has not passed.
     *
     * @param pGems The gems.
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x001ee1b8
     * @ghidraAddress PAL: 0x001f6f58
     */
    static void RestoreGlowSizes(std::list<TnlGem> *pGems, char nTrack);
};
