#pragma once

#include "gfx/gfxtunnel.h"
#include "gfx/tnlactivator.h"
#include "gfx/tnlcursor.h"
#include "gfx/tnlfreestyle.h"
#include "gfx/tnlgeom.h"
#include "gfx/tnlgrid.h"
#include "math/color.h"
#include "rnd/rndcam.h"
#include "rnd/rndtransanim.h"
#include "rnd/rndview.h"
#include "script/dataarray.h"

/**
 * The tunnel graphics of one player: the camera, the activator, the grid, the cursor, and the
 * freestyle.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0x2cc bytes. A player
 * without a camera of the tunnel, one that plays on another console, has no camera views.
 */
class TnlPlayer {
public:
    /**
     * Build the graphics of a player and find the views of its camera.
     *
     * @param nPlayer The player.
     * @param nCam The camera of the player, or -1 for none.
     * @param pszColor The colour of the player.
     * @param pTunnel The tunnel.
     * @param pGeom The geometry of the tunnel.
     * @param nFreestyleType The kind of freestyle of the player, one of Freestylus::Type.
     * @param nExcludedType A gem type the cursor never links.
     * @param nOtherExcludedType A second gem type the cursor never links.
     * @ghidraAddress NTSC-U/C: 0x001d6780
     * @ghidraAddress PAL: 0x001df520
     */
    TnlPlayer(int nPlayer,
              int nCam,
              const char *pszColor,
              GfxTunnel *pTunnel,
              TnlGeom *pGeom,
              int nFreestyleType,
              char nExcludedType,
              char nOtherExcludedType);

    /**
     * Destroy the graphics of the player.
     *
     * @ghidraAddress NTSC-U/C: 0x001d6ad8
     * @ghidraAddress PAL: 0x001df878
     */
    ~TnlPlayer();

    /**
     * Place a camera from the base pitch and offset of the configuration.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pCam The camera.
     * @ghidraAddress NTSC-U/C: 0x001d6b48
     * @ghidraAddress PAL: 0x001df8e8
     */
    static void LoadCamXfm(DataArray *pConfig, DataArray *pDefaults, RndCam *pCam);

    /**
     * Load the settings of the graphics of the player and the colours of its track.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel.
     * @param nOption The fourth argument every loader of a player's tunnel graphics receives.
     * @param bFreestyle Whether the freestyle settings load.
     * @ghidraAddress NTSC-U/C: 0x001d6c88
     * @ghidraAddress PAL: 0x001dfa28
     */
    void LoadConfig(
        DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption, int bFreestyle);

    /**
     * Choose the colour of the track for a multiplier.
     *
     * @param nMultiplier The multiplier.
     * @return Whether the colour changed.
     * @ghidraAddress NTSC-U/C: 0x001d6e18
     * @ghidraAddress PAL: 0x001dfbb8
     */
    bool SelectColor(int nMultiplier);

    /**
     * Advance the activator, the grid, and the cursor.
     *
     * @param flTicks The ticks since the last poll.
     * @param flTime The time since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001d6e48
     * @ghidraAddress PAL: 0x001dfbe8
     */
    void Update(float flTicks, float flTime);

    /**
     * Draw the needle of the scratch freestyle.
     *
     * @ghidraAddress NTSC-U/C: 0x001d6ed0
     * @ghidraAddress PAL: 0x001dfc70
     */
    void UpdateTrail();

    int mCamNumber;                  /*!< The camera plus one, the number of its views. */
    RndCam *mCam;                    /*!< `tnl cam<n>`, or null. */
    RndTransAnim *mIntro;            /*!< `tnl cam intro<players>.tnm`, or null. */
    RndView *mLocalView;             /*!< `tnl local<n>.view`, or null. */
    GfxTunnel *mTunnel;              /*!< The tunnel. */
    TnlGeom *mGeom;                  /*!< The geometry of the tunnel. */
    int mReserved18;                 // +0x18 Cleared, not yet read.
    int mPlayer;                     /*!< The player. */
    int mCamIndex;                   /*!< The camera, or -1. */
    unsigned char mReserved24[0xc];  // +0x24 to +0x2f are not yet identified.
    Color mTrackColor;               /*!< `track_color_<c>`. */
    Color mStreakColor;              /*!< `track_color_<c>_streak`, for a raised multiplier. */
    Color *mColor;                   /*!< mTrackColor or mStreakColor. */
    unsigned char mReserved54[0xc];  // +0x54 to +0x5f are not yet identified.
    TnlActivator mActivator;         /*!< The activator. */
    int mReserved13c;                // +0x13c Not yet read or written.
    TnlGrid mGrid;                   /*!< The grid. */
    TnlCursor mCursor;               /*!< The cursor. */
    unsigned char mReserved1c4[0xc]; // +0x1c4 to +0x1cf are not yet identified.
    TnlFreestyle mFreestyle;         /*!< The freestyle. */
};
