#pragma once

#include <map>

#include "gfx/gfxtunnel.h"
#include "math/interpolator.h"
#include "math/transform.h"
#include "math/vector3.h"
#include "os/string.h"
#include "rnd/animatable.h"
#include "rnd/cam.h"
#include "rnd/transanim.h"
#include "rnd/transformable.h"
#include "script/dataarray.h"

/**
 * The camera of the tunnel, which follows a player with one of a set of algorithms.
 *
 * The class is not polymorphic. The name comes from the type of the algorithm table, a map of
 * pointers to members of `PlayerCamFX`. The object is 0x1f0 bytes. An algorithm places the camera
 * behind a frame of the tunnel. The view then pulls back from the camera along the frame, plays the
 * intro fly-in before the song starts, slides towards a transform the tunnel supplies, and jiggles
 * when kicked.
 */
class PlayerCamFX {
public:
    /**
     * An algorithm, which places the camera for a player.
     *
     * The first transform receives the frame of the track and the second the camera. The integer
     * is the player and the flag records a game of several players on one console.
     */
    typedef void (PlayerCamFX::*Algorithm)(GfxTunnel *, Transform &, Transform &, int, bool);

    /**
     * Find the camera objects and the intro animation.
     *
     * @param nPlayer The player the camera follows.
     * @param nNumTracks The number of tracks, which places the centre of the tunnel.
     * @param bBoss Whether the song is a boss song, which selects the boss intro.
     * @ghidraAddress NTSC-U/C: 0x001f7478
     * @ghidraAddress PAL: 0x00200218
     */
    PlayerCamFX(int nPlayer, int nNumTracks, bool bBoss);

    /**
     * Release the camera.
     *
     * @ghidraAddress NTSC-U/C: 0x001f77d8
     * @ghidraAddress PAL: 0x00200578
     */
    ~PlayerCamFX();

    /**
     * Place the camera objects for a frame.
     *
     * @param pTunnel The tunnel.
     * @param flTick The tick of the song, which runs the intro while it is early enough.
     * @param flTime The time of the song, which runs the pull back.
     * @ghidraAddress NTSC-U/C: 0x001f7818
     * @ghidraAddress PAL: 0x002005b8
     */
    void Poll(GfxTunnel *pTunnel, float flTick, float flTime);

    /**
     * Slide the view towards a transform, or stop sliding.
     *
     * @param pXfm The transform, or null to stop.
     * @param flBlend How far the view slides, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001f7d40
     * @ghidraAddress PAL: 0x00200ae0
     */
    void SetSlide(const Transform *pXfm, float flBlend);

    /**
     * Load the shared settings, and register the algorithms.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel. The routine does not read it.
     * @param nOption The fourth argument every loader of the tunnel receives. A nonzero value
     *                skips the registration of the algorithms.
     * @ghidraAddress NTSC-U/C: 0x001f8750
     * @ghidraAddress PAL: 0x002014f0
     */
    static void
    LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption);

    /**
     * Select the far pull back, and start the pull back to it.
     *
     * @param nZoomedOut Nonzero for the far pull back.
     * @ghidraAddress NTSC-U/C: 0x001f8f20
     * @ghidraAddress PAL: 0x00201cc0
     */
    void SetZoomedOut(int nZoomedOut);

    /**
     * Set a pull back as a multiple of the far one, and start the pull back to it.
     *
     * @param flZoom The multiple, or 0 to use the near or far pull back.
     * @ghidraAddress NTSC-U/C: 0x001f8f40
     * @ghidraAddress PAL: 0x00201ce0
     */
    void SetZoom(float flZoom);

    /**
     * Return the camera to its start.
     *
     * The intro is not played again.
     *
     * @param bNearPullBack Take the near pull back at once rather than the selected one.
     * @ghidraAddress NTSC-U/C: 0x001f9020
     * @ghidraAddress PAL: 0x00201dc0
     */
    void Reset(bool bNearPullBack);

    /**
     * Kick the jiggle of the camera.
     *
     * A camera at rest also takes new random directions to jiggle in.
     *
     * @param flAmount The speed the kick adds.
     * @ghidraAddress NTSC-U/C: 0x001f9280
     * @ghidraAddress PAL: 0x00202020
     */
    void Kick(float flAmount);

    /**
     * Blend in a second algorithm by its name.
     *
     * @param pszName The name, or null for no second algorithm.
     * @param flBlend The blend towards the second algorithm, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001f93e0
     * @ghidraAddress PAL: 0x00202180
     */
    void SetBlendAlgorithm(const char *pszName, float flBlend);

    /**
     * Set the blend towards the second algorithm.
     *
     * @param flBlend The blend, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001f9520
     * @ghidraAddress PAL: 0x002022c0
     */
    void SetBlend(float flBlend);

    int mPlayer;                          /*!< The player the camera follows. */
    int mSkipIntro;                       /*!< Whether the intro is not played. */
    Rnd::Transformable *mFx;              /*!< The jiggle transformable, `tnl cam fx1`. */
    Rnd::Transformable *mSlide;           /*!< The view transformable, `tnl cam slide1`. */
    Rnd::Cam *mCam;                       /*!< The camera, `tnl cam1`. */
    Rnd::TransAnim *mIntro;               /*!< The intro fly-in animation. */
    float mIntroTilt;                     /*!< The tilt of the view over the intro, in radians. */
    Transform mCamera;                    /*!< The camera the algorithm places. */
    Transform mView;                      /*!< The camera after the pull back and the intro. */
    ATanInterpolator mPullBackCurve;      /*!< The pull back over song time. */
    float mPullBackLevel;                 /*!< The pull back, which scales sPullBack. */
    int mPullingBack;                     /*!< Whether mPullBackCurve runs. */
    float mZoom;                          /*!< The pull back as a multiple of the far one, or 0. */
    int mZoomedOut;                       /*!< Whether the far pull back is selected. */
    ATanInterpolator mTrackCurve;         /*!< The move to a new track of the `multi` algorithm. */
    char mFocusPlayer;                    /*!< The player the `multi` algorithm follows, or -1. */
    float mFromTrack;                     /*!< The track the `multi` algorithm moves from. */
    float mTrack;                         /*!< The track of the `multi` algorithm. */
    float mCenterTrack;                   /*!< The track at the centre of the tunnel. */
    Vector3 mJiggleAxis;                  /*!< The unit axis of the jiggle of the view. */
    Vector3 mScreenJiggleAxis;            /*!< The unit axis of the jiggle across the screen. */
    Rnd::Animatable::SecondOrder mJiggle; /*!< The spring of the jiggle. */
    Vector3 mScreenJiggle;                /*!< The jiggle across the screen. */
    int mSliding;                         /*!< Whether the view slides towards mSlideXfm. */
    float mSlideBlend;                    /*!< How far the view slides. */
    Transform mSlideXfm;                  /*!< The transform the view slides towards. */
    std::map<String, Algorithm>::iterator mAlgorithm; /*!< The algorithm, `camera_alg`. */
    Algorithm mBlendAlgorithm;                        /*!< The second algorithm, or null. */
    float mBlend;                                     /*!< The blend towards mBlendAlgorithm. */

private:
    /**
     * Play the intro fly-in at a tick.
     *
     * @param flTick The tick of the song.
     * @ghidraAddress NTSC-U/C: 0x001f7d80
     * @ghidraAddress PAL: 0x00200b20
     */
    void PlayIntro(float flTick);

    /**
     * Orient the camera on a track `view_trailing_frames` ticks behind the place of the song.
     *
     * The camera then sits `view_distance` behind the frame of the track at the place of the
     * song.
     *
     * @param pTunnel The tunnel.
     * @param track Receives the frame of the track at the place of the song.
     * @param camera Receives the camera.
     * @param nPlayer The player. The routine does not read it.
     * @param bLocal Whether several players share the console. The routine does not read it.
     * @param flTrack The track, which may be fractional.
     * @ghidraAddress NTSC-U/C: 0x001f7f70
     * @ghidraAddress PAL: 0x00200d10
     */
    void LookAlong(GfxTunnel *pTunnel,
                   Transform &track,
                   Transform &camera,
                   int nPlayer,
                   bool bLocal,
                   float flTrack);

    /**
     * The `trailing` algorithm, which looks along the track of the player.
     *
     * @param pTunnel The tunnel.
     * @param track Receives the frame of the track at the place of the song.
     * @param camera Receives the camera.
     * @param nPlayer The player.
     * @param bLocal Whether several players share the console.
     * @ghidraAddress NTSC-U/C: 0x001f81c8
     * @ghidraAddress PAL: 0x00200f68
     */
    void
    Trailing(GfxTunnel *pTunnel, Transform &track, Transform &camera, int nPlayer, bool bLocal);

    /**
     * The `trailing_fixed` algorithm, which looks along the centre of the tunnel.
     *
     * @param pTunnel The tunnel.
     * @param track Receives the frame of the track at the place of the song.
     * @param camera Receives the camera.
     * @param nPlayer The player.
     * @param bLocal Whether several players share the console.
     * @ghidraAddress NTSC-U/C: 0x001f8250
     * @ghidraAddress PAL: 0x00200ff0
     */
    void TrailingFixed(
        GfxTunnel *pTunnel, Transform &track, Transform &camera, int nPlayer, bool bLocal);

    /**
     * The `solo` algorithm, which sits behind the player.
     *
     * @param pTunnel The tunnel.
     * @param track Receives the frame of the track at the place of the song.
     * @param camera Receives the camera.
     * @param nPlayer The player.
     * @param bLocal Whether several players share the console. The routine does not read it.
     * @ghidraAddress NTSC-U/C: 0x001f8288
     * @ghidraAddress PAL: 0x00201028
     */
    void Solo(GfxTunnel *pTunnel, Transform &track, Transform &camera, int nPlayer, bool bLocal);

    /**
     * The `multi` algorithm, which moves between the players over mTrackCurve.
     *
     * @param pTunnel The tunnel.
     * @param track Receives the frame of the track at the place of the song.
     * @param camera Receives the camera.
     * @param nPlayer The player. The routine does not read it.
     * @param bLocal Whether several players share the console. The routine does not read it.
     * @ghidraAddress NTSC-U/C: 0x001f84d0
     * @ghidraAddress PAL: 0x00201270
     */
    void Multi(GfxTunnel *pTunnel, Transform &track, Transform &camera, int nPlayer, bool bLocal);

    /**
     * Work out the selected pull back, and set it or start the pull back to it.
     *
     * @param bImmediate Set the pull back at once.
     * @ghidraAddress NTSC-U/C: 0x001f8f60
     * @ghidraAddress PAL: 0x00201d00
     */
    void UpdateZoom(bool bImmediate);
};
