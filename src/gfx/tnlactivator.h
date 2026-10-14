#pragma once

#include "gfx/gfxtunnel.h"
#include "gfx/tnlcatcher.h"
#include "math/interpolator.h"
#include "rnd/rndview.h"
#include "script/dataarray.h"

/**
 * The activator of a player, the frame at the bottom of the track that carries the catcher.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0xdc bytes. The activator
 * slides across the track to the place of the player in the race and shrinks for a player behind
 * the others.
 */
class TnlActivator {
public:
    /**
     * Find the views of the activator of a player and attach the catcher to it.
     *
     * @param pTunnel The tunnel.
     * @param nPlayer The player.
     * @param nCam The camera of the player plus one, which the activator does not read.
     * @param pszColor The colour of the player.
     * @ghidraAddress NTSC-U/C: 0x001e6218
     * @ghidraAddress PAL: 0x001eefb8
     */
    TnlActivator(GfxTunnel *pTunnel, int nPlayer, int nCam, const char *pszColor);

    /**
     * Destroy the activator.
     *
     * @ghidraAddress NTSC-U/C: 0x001e6470
     * @ghidraAddress PAL: 0x001ef210
     */
    ~TnlActivator();

    /**
     * Load the settings of the activator. The body is empty.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel.
     * @param nOption The fourth argument every loader of a player's tunnel graphics receives.
     * @ghidraAddress NTSC-U/C: 0x001e64d0
     * @ghidraAddress PAL: 0x001ef270
     */
    void LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption);

    /**
     * Show or hide the rotating part of the activator.
     *
     * @param bShowing Whether the part is shown.
     * @ghidraAddress NTSC-U/C: 0x001e64d8
     * @ghidraAddress PAL: 0x001ef278
     */
    void SetRotShowing(int bShowing);

    /**
     * Hide the catcher, or let it show again.
     *
     * @param bSuppressed Whether the catcher is hidden.
     * @ghidraAddress NTSC-U/C: 0x001e6508
     * @ghidraAddress PAL: 0x001ef2a8
     */
    void SetSuppressed(int bSuppressed);

    /**
     * Move the activator to the place of the player in the race.
     *
     * The activator slides to its place over 240 ticks and loses 0.04 of its scale for each place
     * it is ahead of the last.
     *
     * @param nPlace The place, 0 for the leader.
     * @param nNumPlaces The number of places.
     * @ghidraAddress NTSC-U/C: 0x001e6528
     * @ghidraAddress PAL: 0x001ef2c8
     */
    void SetPlace(signed char nPlace, signed char nNumPlaces);

    /**
     * Start the hit animation of a target of the catcher.
     *
     * @param nTarget The target.
     * @param bHit Whether the gem was hit. The routine does not read it.
     * @ghidraAddress NTSC-U/C: 0x001e6680
     * @ghidraAddress PAL: 0x001ef420
     */
    void Hit(int nTarget, bool bHit);

    /**
     * Show a bump on the catcher of a leading player who is bumped.
     *
     * @param bBumped Whether the player is the one bumped.
     * @param nTrack The track of the bump, which the activator does not read.
     * @param pAttacker The activator of the attacking player, or null.
     * @ghidraAddress NTSC-U/C: 0x001e66a0
     * @ghidraAddress PAL: 0x001ef440
     */
    void Bump(bool bBumped, int nTrack, const TnlActivator *pAttacker);

    /**
     * Put the activator back at the first place at its full size and end a bump.
     *
     * @ghidraAddress NTSC-U/C: 0x001e66e8
     * @ghidraAddress PAL: 0x001ef488
     */
    void Reset();

    /**
     * Advance the slide, the scale, and the catcher, and pass the place and the scale to the ship.
     *
     * @param flDelta The time since the last update.
     * @ghidraAddress NTSC-U/C: 0x001e6770
     * @ghidraAddress PAL: 0x001ef510
     */
    void Update(float flDelta);

    int mPlayer;                    /*!< The player. */
    RndView *mRot;                  /*!< `activator rot<n>`. */
    RndView *mView;                 /*!< `activator<n>.view`. */
    int mReserved0c;                // +0x0c Not yet read or written.
    RndView *mFx;                   /*!< `activator fx<n>`, which the catcher hangs from. */
    unsigned char mReserved14[0xc]; // +0x14 to +0x1f are not yet identified.
    TnlCatcher mCatcher;            /*!< The catcher. */
    unsigned char mReserved88[0x8]; // +0x88 to +0x8f are not yet identified.
    int mBlinking;                  /*!< Whether the catcher blinks to half opacity. */
    int mReserved94;                // +0x94 Not yet read or written.
    int mMoving;                    /*!< Whether the activator slides to a new place. */
    LinearInterpolator mPlace;      /*!< The place over the ticks of the slide. */
    LinearInterpolator mScale;      /*!< The scale over the ticks of the slide. */
    float mPosition;                /*!< The place the activator is at. */
    GfxTunnel *mTunnel;             /*!< The tunnel. */
};
