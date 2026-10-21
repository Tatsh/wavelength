#pragma once

#include "app/rampanimator.h"
#include "gfx/gfxtunnel.h"
#include "gfx/tnlgeom.h"
#include "math/color.h"
#include "rnd/rndanimatable.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndmatanim.h"
#include "rnd/rndview.h"
#include "script/dataarray.h"

// The activator includes this header, so the class is declared here instead.
class TnlActivator;

/**
 * The catcher of a player, the three targets at the bottom of the track the gems fly into.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0x68 bytes. Each target
 * plays a hit animation for a short time after a gem lands in it, and a bumped player shows a flare
 * in the colour of the player who bumped.
 */
class TnlCatcher {
public:
    /** The targets of the catcher, one for each gem button. */
    static constexpr int kNumTargets = 3;

    /**
     * Find the views and animations of the catcher of a colour and place its targets.
     *
     * @param pszColor The colour of the player.
     * @param pTunnel The tunnel.
     * @ghidraAddress NTSC-U/C: 0x001e5608
     * @ghidraAddress PAL: 0x001ee3a8
     */
    TnlCatcher(const char *pszColor, GfxTunnel *pTunnel);

    /**
     * Destroy the catcher.
     *
     * @ghidraAddress NTSC-U/C: 0x001e5bb8
     * @ghidraAddress PAL: 0x001ee958
     */
    ~TnlCatcher();

    /**
     * Set the opacity of the catcher. The body is empty.
     *
     * @param flAlpha The opacity.
     * @ghidraAddress NTSC-U/C: 0x001e5c08
     * @ghidraAddress PAL: 0x001ee9a8
     */
    void SetAlpha(float flAlpha);

    /**
     * Record whether the multiplier of the player is raised.
     *
     * @param bMultiplied Whether the multiplier is raised.
     * @ghidraAddress NTSC-U/C: 0x001e5c10
     * @ghidraAddress PAL: 0x001ee9b0
     */
    void SetMultiplied(int bMultiplied);

    /**
     * Hang the catcher and the bump flare from a view.
     *
     * @param pParent The view.
     * @ghidraAddress NTSC-U/C: 0x001e5c18
     * @ghidraAddress PAL: 0x001ee9b8
     */
    void AttachTo(RndView *pParent);

    /**
     * Start the hit animation of a target, if the catcher is shown.
     *
     * @param nTarget The target.
     * @ghidraAddress NTSC-U/C: 0x001e5c88
     * @ghidraAddress PAL: 0x001eea28
     */
    void Hit(int nTarget);

    /**
     * Set whether the catcher may be shown.
     *
     * @param bEnabled Whether the catcher may be shown.
     * @ghidraAddress NTSC-U/C: 0x001e5cb8
     * @ghidraAddress PAL: 0x001eea58
     */
    void SetEnabled(int bEnabled);

    /**
     * Set whether the catcher is shown.
     *
     * @param bShown Whether the catcher is shown.
     * @ghidraAddress NTSC-U/C: 0x001e5cc0
     * @ghidraAddress PAL: 0x001eea60
     */
    void SetShown(int bShown);

    /**
     * Stop the hit animations of every target.
     *
     * @ghidraAddress NTSC-U/C: 0x001e5cc8
     * @ghidraAddress PAL: 0x001eea68
     */
    void ResetTargets();

    /**
     * Hide the bump flare and let the camera of the player follow the player again.
     *
     * @param pGeom The geometry of the tunnel.
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001e5d08
     * @ghidraAddress PAL: 0x001eeaa8
     */
    void StopBump(TnlGeom *pGeom, int nPlayer);

    /**
     * Show the bump flare in the colour of the attacker and hold the camera of the player.
     *
     * @param pAttacker The activator of the attacking player, or null for no flare.
     * @param pGeom The geometry of the tunnel.
     * @param nPlayer The bumped player.
     * @ghidraAddress NTSC-U/C: 0x001e5d68
     * @ghidraAddress PAL: 0x001eeb08
     */
    void StartBump(const TnlActivator *pAttacker, TnlGeom *pGeom, int nPlayer);

    /**
     * Move the animation of the catcher towards its raised or its lowered pose.
     *
     * @param bRaised Whether the catcher is raised.
     * @ghidraAddress NTSC-U/C: 0x001e5e10
     * @ghidraAddress PAL: 0x001eebb0
     */
    void SetRaised(bool bRaised);

    /**
     * Advance the hit animations, the animation of the catcher, and the bump flare.
     *
     * Before the song starts the catcher rises into place with the ticks of the count-in.
     *
     * @param pGeom The geometry of the tunnel.
     * @param nPlayer The player.
     * @param bBumped Whether the ship of the player is knocked across the tracks.
     * @param flDelta The time since the last update.
     * @param flBumpTime The time the bump started.
     * @ghidraAddress NTSC-U/C: 0x001e5ee8
     * @ghidraAddress PAL: 0x001eec88
     */
    void Update(TnlGeom *pGeom, int nPlayer, bool bBumped, float flDelta, float flBumpTime);

    /**
     * Load the size, the offset, and the depth test of the targets.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel, which is not read.
     * @param nOption The fourth argument every loader of the tunnel graphics receives, which is not
     * read.
     * @ghidraAddress NTSC-U/C: 0x001e6160
     * @ghidraAddress PAL: 0x001eef00
     */
    static void
    LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption);

    /**
     * The scale of a target, `target_size`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa8c
     * @ghidraAddress PAL: 0x0042e924
     */
    static float sTargetSize;

    /**
     * The height of a target above the track, `target_offset`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa90
     * @ghidraAddress PAL: 0x0042e928
     */
    static float sTargetOffset;

    /**
     * Whether the targets draw without a depth test, `targets_always_on_top`.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa94
     * @ghidraAddress PAL: 0x0042e92c
     */
    static int sTargetsOnTop;

    /**
     * The time after the start of a bump at which the catcher shows again.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa98
     * @ghidraAddress PAL: 0x0042e930
     */
    static float sBumpShowTime;

    /**
     * The time after the start of a bump at which the flare ends, the last frame of its alpha.
     *
     * @ghidraAddress NTSC-U/C: 0x003afa9c
     * @ghidraAddress PAL: 0x0042e934
     */
    static float sBumpEndTime;

private:
    Color mColor;                            /*!< The colour of the flare a bump shows. */
    RndView *mView;                          /*!< `catcher_<c>`. */
    int mShown;                              /*!< Whether the catcher is shown. */
    int mEnabled;                            /*!< Whether the catcher may be shown. */
    RndView *mTargets[kNumTargets];          /*!< `target <n>_<c>.view`. */
    RndAnimatable *mTargetHits[kNumTargets]; /*!< `target hit <n>_<c>.tnm`. */
    RndDrawable *mBumpFlareMesh;             /*!< `target bump flare 0_<c>.mesh`. */
    RndMatAnim *mBumpFlare;                  /*!< `target bump flare_<c>.mnm`. */
    float mHitTimes[kNumTargets];            /*!< The time of the last hit, or kNoHit. */
    RndAnimatable *mAmbient;                 /*!< `target ambient_<c>.view`. */
    RampAnimator mAnim;                      /*!< The pose, `target anim_<c>.view`. */
    int mMultiplied;                         /*!< Whether the multiplier is raised. */
    int mBumping;                            /*!< Whether the bump flare shows. */

    /** The value of mHitTimes for a target that is not hit. */
    static constexpr float kNoHit = -1e9f;
};
