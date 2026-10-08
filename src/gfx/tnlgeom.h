#pragma once

#include "math/transform.h"
#include "rnd/transformable.h"
#include "rnd/view.h"

/**
 * The geometry of the tracks of the tunnel, which places a cell of a track in the world.
 *
 * The class is not polymorphic, and the name comes from the RTTI of its nested types. The object is
 * 0x160 bytes. Only the members the ships use are declared so far.
 */
class TnlGeom {
public:
    /**
     * The place of a player in the tunnel, 0x38 bytes.
     *
     * The RTTI names the type. Only the members of the record are declared so far.
     */
    struct Player {
        Rnd::Transformable *mCamSlide; /*!< The transformable of the camera slide. */
        int mHoldCam;                  /*!< Whether the camera stops following the player. */
        char mTrack;                   /*!< The track the player is on. */
        char mIndex;                   /*!< The index of the player, its bit in a track's mask. */
        float mActivatorPosition;      /*!< The position of the activator. */
        float mReserved10;             // +0x10 Starts at -960, not yet read.
        int mReserved14;               // +0x14 Cleared, not yet read.
        Rnd::View *mActivator;         /*!< The view of the activator, or null. */
        TnlGeom *mGeom;                /*!< The geometry. */
        float mPosition;               /*!< The track the player is on, fractional while moving. */
        float mVelocity;               /*!< The tracks moved per tick at the last update. */
        float mMoveTime;               /*!< The multiple of the length of a move. */
        float mCamPosition;            /*!< The track the camera of the player is on. */
        unsigned char mReserved30[0x08]; // +0x30 The two move curves, not yet declared here.
    };

    /**
     * Report the place of a player.
     *
     * @param nPlayer The player.
     * @return The place, or null for a player out of range.
     * @ghidraAddress NTSC-U/C: 0x001d13f0
     * @ghidraAddress PAL: 0x001da190
     */
    Player *GetPlayer(int nPlayer);

    /**
     * Report the length of a move between two tracks.
     *
     * @param flFrom The track the move starts at.
     * @param flTo The track the move ends at.
     * @return The length in ticks.
     * @ghidraAddress NTSC-U/C: 0x001d1438
     * @ghidraAddress PAL: 0x001da1d8
     */
    float MoveLength(float flFrom, float flTo) const;

    /**
     * Work out the transform of a cell of a track.
     *
     * @param nTrack The track.
     * @param pXfm Receives the transform.
     * @param bSmoothBasis Blend the basis between the two nearest cross sections.
     * @param bTrackOffset Add the offset of the track to the position.
     * @param flTick The tick along the track.
     * @param flLateral The position across the track, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001d1b28
     * @ghidraAddress PAL: 0x001da8c8
     */
    void CellXfm(int nTrack,
                 Transform *pXfm,
                 bool bSmoothBasis,
                 bool bTrackOffset,
                 float flTick,
                 float flLateral);

    /**
     * Work out the transform of a cell at a fractional track, blending the two nearest tracks.
     *
     * @param pXfm Receives the transform.
     * @param bSmoothBasis Blend the basis between the two nearest cross sections.
     * @param bTrackOffset Add the offset of the track to the position.
     * @param flTrack The track, which may lie between two tracks and wraps around the tunnel.
     * @param flTick The tick along the track.
     * @param flLateral The position across the track, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001d20d0
     * @ghidraAddress PAL: 0x001dae70
     */
    void BlendCell(Transform *pXfm,
                   bool bSmoothBasis,
                   bool bTrackOffset,
                   float flTrack,
                   float flTick,
                   float flLateral);
};
