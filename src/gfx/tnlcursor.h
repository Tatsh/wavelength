#pragma once

#include "gfx/gfxtunnel.h"
#include "gfx/tnlconnectorbeam.h"
#include "gfx/tnlgem.h"
#include "gfx/tnlgems.h"
#include "gfx/tnlseeker.h"
#include "gfx/tnltrackrange.h"
#include "math/color.h"
#include "script/dataarray.h"

/**
 * The cursor of a player in the tunnel, a seeker frame and a connector beam over the gems ahead.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0x54 bytes. A move to a
 * new window fades the beam out, moves it, and fades it back in.
 */
class TnlCursor {
public:
    /** Values of mState. */
    enum State {
        kStateIdle = 0,      /*!< The beam is steady. */
        kStateFadingIn = 1,  /*!< The beam fades in on its window. */
        kStateFadingOut = 2, /*!< The beam fades out before it moves to the pending window. */
    };

    /**
     * Create the beam and the frame of a player.
     *
     * @param pTunnel The tunnel.
     * @param nPlayer The player.
     * @param pszColor The colour of the player.
     * @param nExcludedType A gem type the beam never links.
     * @param nOtherExcludedType A second gem type the beam never links.
     * @ghidraAddress NTSC-U/C: 0x001fd030
     * @ghidraAddress PAL: 0x00205dd0
     */
    TnlCursor(GfxTunnel *pTunnel,
              int nPlayer,
              const char *pszColor,
              char nExcludedType,
              char nOtherExcludedType);

    /**
     * Delete the frame and the beam.
     *
     * @ghidraAddress NTSC-U/C: 0x001fd120
     * @ghidraAddress PAL: 0x00205ec0
     */
    ~TnlCursor();

    /**
     * Load the settings of the beam, the frame, and the fade, and take the colour of the player.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel.
     * @param nOption The fourth argument every loader of a player's tunnel graphics receives.
     * @ghidraAddress NTSC-U/C: 0x001fd188
     * @ghidraAddress PAL: 0x00205f28
     */
    void LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption);

    /**
     * Reset the beam and the frame, and forget the last target.
     *
     * @ghidraAddress NTSC-U/C: 0x001fd250
     * @ghidraAddress PAL: 0x00205ff0
     */
    void Reset();

    /**
     * Retarget the pending window with no gems and no length.
     *
     * @ghidraAddress NTSC-U/C: 0x001fd298
     * @ghidraAddress PAL: 0x00206038
     */
    void Refresh();

    /**
     * Move the cursor to a window.
     *
     * A cursor that is idle and faded out shows the window at once and fades in. Otherwise the
     * frame glides to the window, and the beam fades out before it moves.
     *
     * @param nTrack The track.
     * @param pGems The gems, or null.
     * @param flStartTick The start of the window.
     * @param flEndTick The end of the window.
     * @ghidraAddress NTSC-U/C: 0x001fd2c0
     * @ghidraAddress PAL: 0x00206060
     */
    void SetTarget(char nTrack, TnlGems *pGems, float flStartTick, float flEndTick);

    /**
     * Pass a changed range of the tunnel to the beam and the frame.
     *
     * @param pRange The range that changed.
     * @ghidraAddress NTSC-U/C: 0x001fd3a0
     * @ghidraAddress PAL: 0x00206140
     */
    void OnRangeChanged(const TnlTrackRange *pRange);

    /**
     * Pass a new gem to the beam.
     *
     * @param nTrack The track of the gem.
     * @param nType The type of the gem, or -1.
     * @param pGems The gems.
     * @param flTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x001fd3e0
     * @ghidraAddress PAL: 0x00206180
     */
    void OnGemAdded(char nTrack, char nType, TnlGems *pGems, float flTick);

    /**
     * Pass a changed span of ticks to the beam.
     *
     * @param nTrack The track.
     * @param pGems The gems.
     * @param flStartTick The start of the span.
     * @param flEndTick The end of the span.
     * @ghidraAddress NTSC-U/C: 0x001fd410
     * @ghidraAddress PAL: 0x002061b0
     */
    void OnTicksChanged(char nTrack, TnlGems *pGems, float flStartTick, float flEndTick);

    /**
     * Pass a removed gem to the beam.
     *
     * @param pGem The gem.
     * @ghidraAddress NTSC-U/C: 0x001fd438
     * @ghidraAddress PAL: 0x002061d8
     */
    void OnGemRemoved(const TnlGem *pGem);

    /**
     * Pulse the beam on a beat.
     *
     * @param flTick The tick of the beat.
     * @ghidraAddress NTSC-U/C: 0x001fd458
     * @ghidraAddress PAL: 0x002061f8
     */
    void Pulse(float flTick);

    /**
     * Set the energy of the beam.
     *
     * @param flEnergy The input of the energy model.
     * @ghidraAddress NTSC-U/C: 0x001fd478
     * @ghidraAddress PAL: 0x00206218
     */
    void SetEnergy(float flEnergy);

    /**
     * Start the multiplier colouring of the beam and the frame.
     *
     * @param pGems The gems.
     * @param flEndTick The tick the multiplier ends at.
     * @ghidraAddress NTSC-U/C: 0x001fd498
     * @ghidraAddress PAL: 0x00206238
     */
    void StartMultiplier(TnlGems *pGems, float flEndTick);

    /**
     * Advance the fade, the beam, and the frame.
     *
     * @param bCaptured Whether the track of the player is captured.
     * @param flTime The first time of the poll of the player's tunnel graphics. The routine does
     *               not read it.
     * @param flDelta The ticks since the last poll, whose magnitude advances the fade.
     * @ghidraAddress NTSC-U/C: 0x001fd4d8
     * @ghidraAddress PAL: 0x00206278
     */
    void Poll(bool bCaptured, float flTime, float flDelta);

    int mPlayer;             /*!< The player. */
    GfxTunnel *mTunnel;      /*!< The tunnel. */
    float mFade;             /*!< The opacity of the beam, from 0 to 1. */
    State mState;            /*!< The fade, one of State. */
    char mTrack;             /*!< The track of the pending window, or -1. */
    float mStartTick;        /*!< The start of the pending window. */
    float mEndTick;          /*!< The end of the pending window. */
    TnlGems *mGems;          /*!< The gems of the pending window, or null. */
    Color mColor;            /*!< The colour of the player. */
    float mTargetStartTick;  /*!< The start of the last target. */
    float mTargetEndTick;    /*!< The end of the last target. */
    char mTargetTrack;       /*!< The track of the last target, or -1. */
    TnlConnectorBeam *mBeam; /*!< The beam. */
    TnlSeeker *mSeeker;      /*!< The frame. */
};
