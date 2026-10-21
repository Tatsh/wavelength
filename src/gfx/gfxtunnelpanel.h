#pragma once

#include "gfx/gfxtunnel.h"
#include "math/color.h"
#include "rnd/rndmat.h"

/**
 * The look of one bar of a track waiting to be applied to its panel, either once the track is no
 * longer captured or by a reveal that sweeps the panel along the bar.
 *
 * The RTTI identifies the class, an element of the list of panels of the tunnel. The class is not
 * polymorphic, and the object is 0x3c bytes.
 */
class GfxTunnelPanel {
public:
    /**
     * Copy the look of a bar and resolve the state of its panel.
     *
     * @param pTunnel The tunnel.
     * @param nTrack The track.
     * @param pBar The bar.
     * @param pColor The colour of the panel.
     * @ghidraAddress NTSC-U/C: 0x001d6ef0
     * @ghidraAddress PAL: 0x001dfc90
     */
    GfxTunnelPanel(GfxTunnel *pTunnel,
                   int nTrack,
                   const GfxTunnel::BarData *pBar,
                   const Color *pColor);

    /**
     * Choose whether the panel waits for its track, and the tick its reveal starts at.
     *
     * @param bDeferred Whether the panel waits until its track is no longer captured.
     * @param flRevealTick The tick the reveal starts at.
     * @ghidraAddress NTSC-U/C: 0x001d6f80
     * @ghidraAddress PAL: 0x001dfd20
     */
    void SetDeferred(int bDeferred, float flRevealTick);

    /**
     * Choose the transparency, the pickability, and the checkpoint material from the flags, the
     * owner, and the song mapping of the bar.
     *
     * @ghidraAddress NTSC-U/C: 0x001d6f90
     * @ghidraAddress PAL: 0x001dfd30
     */
    void ResolveState();

    /**
     * Give the panel its state, and in a remix the checkpoint material of the bar of track 0.
     *
     * @ghidraAddress NTSC-U/C: 0x001d7040
     * @ghidraAddress PAL: 0x001dfde0
     */
    void Apply();

    /**
     * Finish a running reveal at once.
     *
     * @ghidraAddress NTSC-U/C: 0x001d70e0
     * @ghidraAddress PAL: 0x001dfe80
     */
    void Refresh();

    /**
     * Advance the panel.
     *
     * @param pTunnel The tunnel, which reports whether the track is captured.
     * @param flTick The tick of the song.
     * @return Whether the panel still waits; false once it is applied or dropped.
     * @ghidraAddress NTSC-U/C: 0x001d7128
     * @ghidraAddress PAL: 0x001dfec8
     */
    bool Update(GfxTunnel *pTunnel, float flTick);

private:
    GfxTunnel *mTunnel;     /*!< The tunnel. */
    int mTrack;             /*!< The track. */
    int mBar;               /*!< The bar of the track. */
    float mTick;            /*!< The tick the bar starts at. */
    int mOwner;             /*!< The player who owns the bar, or -1. */
    int mFlags;             /*!< The flags of the bar, a set of GfxTunnel::BarFlag. */
    int mInSong;            /*!< Whether the bar maps to a bar of the song. */
    int mDeferred;          /*!< Whether the panel waits until its track is not captured. */
    RndMat *mCheckpointMat; /*!< The checkpoint material of the panel, or null. */
    const Color *mColor;    /*!< The colour of the panel. */
    int mTransparent;       /*!< Whether the panel does not write depth. */
    int mPickable;          /*!< Whether picks test the panel. */
    float mRevealTick;      /*!< The tick the reveal starts at. */
    int mRevealTicks;       /*!< The ticks of a running reveal, or 0 before it starts. */
    int mReserved38;        // +0x38 Cleared by the constructor and Update(), never read.
};
