#pragma once

#include <vector>

#include "gfx/gfxtunnel.h"
#include "gfx/tnlgeom.h"
#include "gfx/tnltrackrange.h"
#include "math/interpolator.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/meshface.h"
#include "rnd/meshvert.h"
#include "script/dataarray.h"

/**
 * The frame that marks the gems ahead of a player's cursor on its track.
 *
 * The class is not polymorphic, and the name is inferred from its `seeker` materials and mesh. The
 * object is 0x78 bytes. The frame is a mesh of two prongs at each end of a window of ticks, joined
 * by rails along the track. It glides to a new track and window over a time that grows with the
 * distance, first across the tracks and then along them.
 */
class TnlSeeker {
public:
    /**
     * Find the materials and create the mesh of a player's frame.
     *
     * @param pGeom The geometry of the tunnel.
     * @param nPlayer The player. The constructor does not read it.
     * @param pszColor The colour of the player, which selects the materials and titles the mesh.
     * @ghidraAddress NTSC-U/C: 0x001fb6a0
     * @ghidraAddress PAL: 0x00204440
     */
    TnlSeeker(TnlGeom *pGeom, int nPlayer, const char *pszColor);

    /**
     * Delete the mesh and the curves.
     *
     * @ghidraAddress NTSC-U/C: 0x001fb9d0
     * @ghidraAddress PAL: 0x00204770
     */
    ~TnlSeeker();

    /**
     * Load the shape of the frame from the `seeker_frame` section and build the mesh.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param pTunnel The tunnel. The routine does not read it.
     * @param nOption The fourth argument every loader of a player's tunnel graphics receives. The
     *                routine does not read it.
     * @ghidraAddress NTSC-U/C: 0x001fba80
     * @ghidraAddress PAL: 0x00204820
     */
    void LoadConfig(DataArray *pConfig, DataArray *pDefaults, GfxTunnel *pTunnel, int nOption);

    /**
     * Start the glide to a new track and window.
     *
     * The first target is taken at once.
     *
     * @param nTrack The track.
     * @param flStartTick The start of the window.
     * @param flEndTick The end of the window.
     * @ghidraAddress NTSC-U/C: 0x001fcad8
     * @ghidraAddress PAL: 0x00205878
     */
    void SetTarget(char nTrack, float flStartTick, float flEndTick);

    /**
     * Draw with the multiplier material, shrinking the frame until a tick.
     *
     * @param flEndTick The tick the multiplier ends at. A tick already passed is ignored.
     * @ghidraAddress NTSC-U/C: 0x001fccb8
     * @ghidraAddress PAL: 0x00205a58
     */
    void StartMultiplier(float flEndTick);

    /**
     * Return to the normal material.
     *
     * @ghidraAddress NTSC-U/C: 0x001fcd28
     * @ghidraAddress PAL: 0x00205ac8
     */
    void StopMultiplier();

    /**
     * Receive the window the cursor fades in on. The body is empty.
     *
     * @param nTrack The track.
     * @param flStartTick The start of the window.
     * @param flEndTick The end of the window.
     * @ghidraAddress NTSC-U/C: 0x001fcd50
     * @ghidraAddress PAL: 0x00205af0
     */
    void SetWindow(char nTrack, float flStartTick, float flEndTick);

    /**
     * Mark the mesh for rebuilding when the tunnel changes a range that overlaps the frame.
     *
     * @param pRange The range that changed.
     * @ghidraAddress NTSC-U/C: 0x001fcd58
     * @ghidraAddress PAL: 0x00205af8
     */
    void OnRangeChanged(const TnlTrackRange *pRange);

    /**
     * Advance the glide and rebuild the mesh when the frame moves.
     *
     * The frame is hidden once its end is more than 960 ticks behind the song.
     *
     * @param flDelta The ticks since the last poll. The routine does not read it.
     * @param flAlpha The opacity of the cursor. The routine does not read it.
     * @ghidraAddress NTSC-U/C: 0x001fcdb0
     * @ghidraAddress PAL: 0x00205b50
     */
    void Poll(float flDelta, float flAlpha);

    /**
     * The fade speed of the cursor, `seeker_fade_speed` over 1000.
     *
     * TnlCursor loads it. A glide over the longest distance lasts 2 over this speed.
     *
     * @ghidraAddress NTSC-U/C: 0x003afc10
     */
    static float sFadeSpeed;

    TnlGeom *mGeom;                     /*!< The geometry of the tunnel. */
    Rnd::Mesh *mMesh;                   /*!< The mesh, `seeker <colour>.mesh`. */
    InvExpInterpolator *mTrackCurve;    /*!< The glide across the tracks, over song time. */
    InvExpInterpolator *mDepthCurve;    /*!< The glide along the track, over song time. */
    float mFromTrack;                   /*!< The track the glide started from, or -1. */
    float mTrack;                       /*!< The track the frame is on, fractional while gliding. */
    char mTargetTrack;                  /*!< The track the glide ends on. */
    float mDrawnTrack;                  /*!< The track of the mesh. */
    float mFromStartTick;               /*!< The start of the window the glide started from. */
    float mStartTick;                   /*!< The start of the window. */
    float mTargetStartTick;             /*!< The start of the window the glide ends on. */
    float mDrawnStartTick;              /*!< The start of the window of the mesh. */
    float mFromEndTick;                 /*!< The end of the window the glide started from. */
    float mEndTick;                     /*!< The end of the window. */
    float mTargetEndTick;               /*!< The end of the window the glide ends on. */
    float mDrawnEndTick;                /*!< The end of the window of the mesh. */
    int mSectionVerts;                  /*!< The vertices in each quarter of the mesh. */
    int mNumQuads;                      /*!< The quads of the mesh. */
    int mDirty;                         /*!< Whether the mesh needs rebuilding. */
    Rnd::Mat *mMat;                     /*!< The material, `seeker_<letter>.mat`. */
    Rnd::Mat *mMultiplierMat;           /*!< The multiplier material, `seeker mult_<letter>.mat`. */
    int mMultiplier;                    /*!< Whether the multiplier material is on. */
    float mMultiplierStartTime;         /*!< The song time the multiplier started at. */
    LinearInterpolator mMultiplierFade; /*!< The fraction of the multiplier passed, over ticks. */

private:
    /**
     * Append the two triangles of each quad of a strip.
     *
     * Each quad joins two vertices of one row to the two of the other.
     *
     * @param faces The faces.
     * @param nRowA The first vertex of one row.
     * @param nRowB The first vertex of the other row.
     * @param nStepA The step along the row of nRowA.
     * @param nStepB The step along the row of nRowB.
     * @param nCount The number of quads.
     * @param bFlip Exchange the two rows, which turns the faces over.
     * @ghidraAddress NTSC-U/C: 0x001f97f0
     * @ghidraAddress PAL: 0x00202590
     */
    static void AddStrip(std::vector<Rnd::MeshFace> &faces,
                         int nRowA,
                         int nRowB,
                         int nStepA,
                         int nStepB,
                         int nCount,
                         bool bFlip);

    /**
     * Set the texture coordinates of one vertex in all four quarters of the mesh.
     *
     * The inner quarters take the first pair and the outer quarters the second.
     *
     * @param verts The vertices.
     * @param nIndex The vertex in each quarter.
     * @param flInnerU The first texture coordinate of the inner quarters.
     * @param flInnerV The second texture coordinate of the inner quarters.
     * @param flOuterU The first texture coordinate of the outer quarters.
     * @param flOuterV The second texture coordinate of the outer quarters.
     * @ghidraAddress NTSC-U/C: 0x001fc470
     * @ghidraAddress PAL: 0x00205210
     */
    void SetTexCoords(std::vector<Rnd::MeshVert> &verts,
                      int nIndex,
                      float flInnerU,
                      float flInnerV,
                      float flOuterU,
                      float flOuterV);

    /**
     * Place one vertex of the frame in all four quarters of the mesh.
     *
     * The two sides of the frame mirror each other across the track, and the inner quarters rise
     * flHeight above the outer ones.
     *
     * @param verts The vertices.
     * @param nIndex The vertex in each quarter.
     * @param flTrack The track.
     * @param flLateral The position across the track, from 0 to 1.
     * @param flTick The tick along the track.
     * @param flHeight The height of the frame.
     * @ghidraAddress NTSC-U/C: 0x001fc520
     * @ghidraAddress PAL: 0x002052c0
     */
    void SetSection(std::vector<Rnd::MeshVert> &verts,
                    int nIndex,
                    float flTrack,
                    float flLateral,
                    float flTick,
                    float flHeight);

    /**
     * Place the vertices of the frame over a window.
     *
     * A vertex more than 960 ticks behind the song is not moved.
     *
     * @param verts The vertices.
     * @param flTrack The track.
     * @param flStartTick The start of the window.
     * @param flEndTick The end of the window.
     * @ghidraAddress NTSC-U/C: 0x001fc720
     * @ghidraAddress PAL: 0x002054c0
     */
    void
    Layout(std::vector<Rnd::MeshVert> &verts, float flTrack, float flStartTick, float flEndTick);
};
