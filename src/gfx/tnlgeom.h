#pragma once

#include <utility>
#include <vector>

#include "gfx/tnlpath.h"
#include "math/color.h"
#include "math/interpolator.h"
#include "math/plane.h"
#include "math/quaternion.h"
#include "math/transform.h"
#include "math/triangle.h"
#include "math/vector2.h"
#include "os/mem.h"
#include "rnd/rndcollideable.h"
#include "rnd/rndmat.h"
#include "rnd/rndmatanim.h"
#include "rnd/rndmesh.h"
#include "rnd/rndtransformable.h"
#include "rnd/rndview.h"
#include "script/dataarray.h"

/**
 * The geometry of the tracks of the tunnel, which places a cell of a track in the world.
 *
 * The class is not polymorphic, and the name comes from the RTTI of its nested types. The object is
 * 0x160 bytes. The tunnel is built one bar at a time into a ring of mNumBars panels on each track.
 * A panel is a strip of cross sections placed along the path and drawn through a mesh. A tick with
 * no built panel falls back to the flat cross section of its track carried along the path.
 */
class TnlGeom {
public:
    /** Bars of the ring of built panels a bar index selects with its low bits. */
    static constexpr int kRingMask = 7;

    /** The bar a slot of the ring holds when no bar is built in it. */
    static constexpr int kNoBar = 100000000;

    /** One cross section of a panel, 0x70 bytes. */
    struct CrossSectionXfm {
        Transform mXfm;        /*!< The section in the world, X across the full track. */
        Vector3 mCellBasis[3]; /*!< The unit basis of a cell at the section. */
    };

    /** The flat cross section of one track, 0xa0 bytes. */
    struct TrackGeometry {
        float mLeftX;      /*!< The X of the left edge of the track. */
        float mLeftZ;      /*!< The Z of the left edge of the track. */
        float mRightX;     /*!< The X of the right edge of the track. */
        float mRightZ;     /*!< The Z of the right edge of the track. */
        Vector3 mBasis[3]; /*!< The basis of a cell of the track. */
        Vector3 mCenter;   /*!< The middle of the track, between the edges. */
        Transform mXfm;    /*!< The track in the space of the path. */
        Quat mRotation;    /*!< The rotation of mBasis. */
    };

    /** Owner of a mesh the panels are built from, 4 bytes. */
    class MeshHolder {
    public:
        /**
         * Delete the mesh unless sKeepMeshes is set.
         *
         * @ghidraAddress NTSC-U/C: 0x001d28c8
         * @ghidraAddress PAL: 0x001db668
         */
        ~MeshHolder();

        /**
         * Whether a holder retains its mesh on destruction. The image never writes it.
         *
         * @ghidraAddress NTSC-U/C: 0x003af9f0
         * @ghidraAddress PAL: 0x0042e888
         */
        static bool sKeepMeshes;

        RndMesh *mMesh; /*!< The mesh, or null. */
    };

    /** One built bar of one track, 0xd0 bytes. */
    class PanelData {
    public:
        /**
         * Construct a white, transparent, unpickable panel with no mesh that needs a rebuild.
         *
         * BuildPanels() inlines it.
         */
        PanelData();

        /**
         * Delete the mesh of the panel.
         *
         * The destructor of the geometry and the resize of mPanels inline it.
         */
        ~PanelData();

        /**
         * Rebuild the vertices of the mesh from the sections of the panel and a source mesh.
         *
         * Each row of four vertices takes the section of its index, and the rows before
         * mStartSection all take that section.
         *
         * @param pSource The mesh whose vertices are placed.
         * @param pOffset The offset added to each source vertex first.
         * @param nRows The rows of vertices.
         * @ghidraAddress NTSC-U/C: 0x001d2940
         * @ghidraAddress PAL: 0x001db6e0
         */
        void Rebuild(const MeshHolder *pSource, const Vector3 *pOffset, int nRows);

        /**
         * Rebuild the two triangles of the panel that a pick tests.
         *
         * @ghidraAddress NTSC-U/C: 0x001d2aa8
         * @ghidraAddress PAL: 0x001db848
         */
        void UpdateTriangles();

        /**
         * Report whether a segment strikes the panel.
         *
         * @param pSegment The segment.
         * @return Whether picks test the panel and the segment strikes either triangle.
         * @ghidraAddress NTSC-U/C: 0x001d2cf0
         * @ghidraAddress PAL: 0x001dba90
         */
        bool Hit(const Segment *pSegment) const;

        std::vector<CrossSectionXfm> mBaseSections; /*!< The sections as the bar was built. */
        std::vector<CrossSectionXfm> mSections;     /*!< The sections as drawn, which may bend. */
        RndMesh *mMesh;                             /*!< The mesh of the panel. */
        RndMat *mCheckpointMat; /*!< The checkpoint material of the panel, or null. */
        Triangle mTriangles[2]; /*!< The halves of the panel a pick tests. */
        Color mColor;           /*!< The colour of the panel. */
        char mHighlighted;      /*!< Whether the panel is drawn highlighted. */
        int mDirty;             /*!< Whether the mesh needs Rebuild(). */
        int mPickable;          /*!< Whether Hit() tests the panel. */
        char mTransparent;      /*!< Whether the panel does not write depth. */
        char mStartSection;     /*!< The first section Rebuild() moves along. */
    };

    /** The state of one track, 0x30 bytes. */
    class TrackData {
    public:
        /**
         * Construct the state of a track.
         *
         * @ghidraAddress NTSC-U/C: 0x001d2890
         * @ghidraAddress PAL: 0x001db630
         */
        TrackData();

        /**
         * Clear the overlay and the players of the track. The colour is not changed.
         *
         * @ghidraAddress NTSC-U/C: 0x001d28b8
         * @ghidraAddress PAL: 0x001db658
         */
        void Clear();

        RndMat *mOverlayMat;    /*!< The material drawn over the track, or null. */
        int mOverlayMode;       /*!< How the overlay is drawn. */
        Color mColor;           /*!< The colour of the panels of the track. */
        unsigned char mPlayers; /*!< One bit for each player on the track. */
    };

    /** The place of a player in the tunnel, 0x38 bytes. */
    class Player {
    public:
        /**
         * Construct a player on track 0.
         *
         * @ghidraAddress NTSC-U/C: 0x001d2d68
         * @ghidraAddress PAL: 0x001dbb08
         */
        Player();

        /**
         * Delete the curves of the player.
         *
         * @ghidraAddress NTSC-U/C: 0x001d2dd0
         * @ghidraAddress PAL: 0x001dbb70
         */
        ~Player();

        /**
         * Attach the player to the geometry.
         *
         * @param pGeom The geometry.
         * @param nIndex The index of the player.
         * @ghidraAddress NTSC-U/C: 0x001d2e50
         * @ghidraAddress PAL: 0x001dbbf0
         */
        void Init(TnlGeom *pGeom, char nIndex);

        /**
         * Put the player at its track at once.
         *
         * @ghidraAddress NTSC-U/C: 0x001d2e60
         * @ghidraAddress PAL: 0x001dbc00
         */
        void Reset();

        /**
         * Move the player along its curves.
         *
         * @param flTick The tick of the song.
         * @param flDeltaTicks The ticks since the last update. The velocity changes only when it
         *                     is more than one tick.
         * @ghidraAddress NTSC-U/C: 0x001d2e90
         * @ghidraAddress PAL: 0x001dbc30
         */
        void Update(float flTick, float flDeltaTicks);

        /**
         * Set the transformable the camera of the player slides.
         *
         * @param pSlide The transformable.
         * @ghidraAddress NTSC-U/C: 0x001d3088
         * @ghidraAddress PAL: 0x001dbe28
         */
        void SetCamSlide(RndTransformable *pSlide);

        /**
         * Move the player to a track, along curves when the geometry animates moves.
         *
         * @param nTrack The track.
         * @ghidraAddress NTSC-U/C: 0x001d3090
         * @ghidraAddress PAL: 0x001dbe30
         */
        void SetTrack(char nTrack);

        /**
         * Take the player off the record of the players of its track.
         *
         * @ghidraAddress NTSC-U/C: 0x001d33d8
         * @ghidraAddress PAL: 0x001dc178
         */
        void LeaveTrack();

        /**
         * Set the view of the activator of the player.
         *
         * @param pView The view.
         * @ghidraAddress NTSC-U/C: 0x001d3410
         * @ghidraAddress PAL: 0x001dc1b0
         */
        void SetActivator(RndView *pView);

        /**
         * Set the position of the activator of the player.
         *
         * @param flPosition The position.
         * @ghidraAddress NTSC-U/C: 0x001d3418
         * @ghidraAddress PAL: 0x001dc1b8
         */
        void SetActivatorPosition(float flPosition);

        /**
         * Place the view of the activator, if there is one.
         *
         * @param pXfm The transform.
         * @ghidraAddress NTSC-U/C: 0x001d3420
         * @ghidraAddress PAL: 0x001dc1c0
         */
        void PlaceActivator(const Transform *pXfm);

        RndTransformable *mCamSlide; /*!< The transformable of the camera slide. */
        int mHoldCam;                /*!< Whether the camera stops following the player. */
        char mTrack;                 /*!< The track the player is on. */
        char mIndex;                 /*!< The index of the player, its bit in mPlayers. */
        float mActivatorPosition;    /*!< The position of the activator. */
        /*!< The ticks from the current tick to the first tick Poll() retains built bars from. */
        float mTrailTicks;
        int mReserved14;     // +0x14 Cleared, not yet read.
        RndView *mActivator; /*!< The view of the activator, or null. */
        TnlGeom *mGeom;      /*!< The geometry. */
        float mPosition;     /*!< The track the player is on, fractional while moving. */
        float mVelocity;     /*!< The tracks moved per tick at the last update. */
        float mMoveTime;     /*!< The multiple of the length of a move. */
        float mCamPosition;  /*!< The track the camera of the player is on. */
        TableLinInterpolator *mCamCurve;  /*!< The move of the camera, or null. */
        TableLinInterpolator *mMoveCurve; /*!< The move of the player, or null. */
    };

    /**
     * Construct an empty geometry.
     *
     * @ghidraAddress NTSC-U/C: 0x001cc360
     * @ghidraAddress PAL: 0x001d5100
     */
    TnlGeom();

    /**
     * Destroy the geometry.
     *
     * @ghidraAddress NTSC-U/C: 0x001cc5a8
     * @ghidraAddress PAL: 0x001d5348
     */
    ~TnlGeom();

    /**
     * Allocate a geometry from the pool, billed to `Tunnel`.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Tunnel", 0);
    }

    /**
     * Release a geometry.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Load the settings of the geometry. The body is empty.
     *
     * @param pConfig The configuration.
     * @param pDefaults The defaults of the configuration.
     * @param nOption The fourth argument every loader of the tunnel graphics receives.
     * @ghidraAddress NTSC-U/C: 0x001ccaa0
     * @ghidraAddress PAL: 0x001d5840
     */
    void LoadConfig(DataArray *pConfig, DataArray *pDefaults, int nOption);

    /**
     * Set up the tracks from the edges of the cross section, and the passes of the standard panel
     * from a material and an animation of it, then rebuild the meshes and the ring of panels.
     *
     * Each texture stage of pPanelMat becomes one material `tnl std panel mat pass <n>`, and each
     * stage of pPanelAnim with transform keys one animation `tnl std matanim stage <n>` of the
     * material of its pass. A track past the end of pColors takes the last colour.
     *
     * @param pEdges The left and the right edge of each track, X then Z.
     * @param nNumBars The bars of each track in the ring of built panels.
     * @param nDetail One more than the doublings of the cross sections of a bar.
     * @param nBarsBehind The built bars retained behind the current bar.
     * @param flSlabDepth The depth of a slab under the panels.
     * @param flSlabAngle The angle of the sides of a slab, in radians.
     * @param pPath The animation of the path.
     * @param pColors The colour of each track.
     * @param pPanelMat The material of the standard panel.
     * @param pPanelAnim The animation of pPanelMat.
     * @return Whether pEdges has a track.
     * @ghidraAddress NTSC-U/C: 0x001d0658
     * @ghidraAddress PAL: 0x001d93f8
     */
    bool Init(const std::vector<std::pair<Vector2, Vector2>> *pEdges,
              int nNumBars,
              int nDetail,
              int nBarsBehind,
              float flSlabDepth,
              float flSlabAngle,
              RndTransAnim *pPath,
              const std::vector<Color> *pColors,
              RndMat *pPanelMat,
              RndMatAnim *pPanelAnim);

    /**
     * Advance the geometry to a tick, building the bars ahead and moving the players and their
     * activators.
     *
     * @param flTick The tick of the song.
     * @param flDeltaTicks The ticks since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001cdab8
     * @ghidraAddress PAL: 0x001d6858
     */
    void Poll(float flTick, float flDeltaTicks);

    /**
     * Rebuild the dirty panels, then draw the built bars from mBarsBehind before the end of the
     * ring back to the first bar of the song, as the tunnel does in a solo game or outside the
     * Game rule set.
     *
     * Tracks with an overlay mode skip the first three passes. The checkpoint material of each
     * panel draws first. Then each pass of the standard panel draws the opaque panels without a
     * checkpoint, the first pass in the colour of each panel. Last, each track with a player, an
     * overlay, or changed bar values draws its opaque panels with the bar value or the overlay,
     * and a track with a player draws its transparent panels with the material of their bar. The
     * call then sets mAnimateMoves. The title is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001ccaa8
     * @ghidraAddress PAL: 0x001d5848
     */
    void DrawPanels();

    /**
     * Draw as DrawPanels() does, as the tunnel does in a game of several players with the Game rule
     * set.
     *
     * The first pass of the standard panel draws without the colour of each panel, and each later
     * pass takes the colour of the panel and the light colour in mPassLightColors for whether the
     * panel is highlighted. The title is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001ccfa8
     * @ghidraAddress PAL: 0x001d5d48
     */
    void DrawPanelsMultiplayer();

    /**
     * Draw the bars of the ring before the song with the bar values, the overlays of the tracks,
     * and the materials of the bars, as the last step of DrawPanels() does, but with each mesh
     * writing depth while it draws. The title is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001cd4d0
     * @ghidraAddress PAL: 0x001d6270
     */
    void DrawOverlays();

    /**
     * Build the bars of the ring that are not built yet.
     *
     * Bars at or after mEndBar are marked as not built. While mAnimateMoves is set, the call
     * stops after the first bar it builds.
     *
     * @ghidraAddress NTSC-U/C: 0x001d0020
     * @ghidraAddress PAL: 0x001d8dc0
     */
    void BuildBars();

    /**
     * Build the cross sections of one bar on every track along the path, and give the slot of the
     * bar its material.
     *
     * A bar before the song takes the kind of panel counted back from the last kind, and every
     * other bar takes the default material. The last section of each panel starts the panel as
     * transparent, not pickable, unhighlighted, and in the colour of its track.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x001d0100
     * @ghidraAddress PAL: 0x001d8ea0
     */
    void BuildBar(int nBar);

    /**
     * Put every panel back in its first state, white and transparent, and mark every slot of the
     * ring as not built.
     *
     * @ghidraAddress NTSC-U/C: 0x001d0548
     * @ghidraAddress PAL: 0x001d92e8
     */
    void ResetPanels();

    /**
     * Put every player at its track at once.
     *
     * @ghidraAddress NTSC-U/C: 0x001d0600
     * @ghidraAddress PAL: 0x001d93a0
     */
    void ResetPlayers();

    /**
     * Set the number of players, attaching each to the geometry with its index.
     *
     * @param nPlayers The number of players.
     * @ghidraAddress NTSC-U/C: 0x001d1290
     * @ghidraAddress PAL: 0x001da030
     */
    void SetNumPlayers(int nPlayers);

    /**
     * Set the ticks of a move of a player to the next track.
     *
     * @param flLength The ticks.
     * @ghidraAddress NTSC-U/C: 0x001d1430
     * @ghidraAddress PAL: 0x001da1d0
     */
    void SetMoveLength(float flLength);

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
     * Work out the transform of a cell of a track without the offset of the track.
     *
     * @param nTrack The track.
     * @param pXfm Receives the transform.
     * @param bSmoothBasis Blend the basis between the two nearest cross sections.
     * @param flTick The tick along the track.
     * @param flLateral The position across the track, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001d1488
     * @ghidraAddress PAL: 0x001da228
     */
    void TrackXfm(int nTrack, Transform *pXfm, bool bSmoothBasis, float flTick, float flLateral);

    /**
     * Work out the transform of a cell at a fractional track, blending the transforms of the two
     * nearest tracks.
     *
     * @param pXfm Receives the transform.
     * @param bSmoothBasis Blend the basis between the two nearest cross sections.
     * @param flTrack The track, which may lie between two tracks and wraps around the tunnel.
     * @param flTick The tick along the track.
     * @param flLateral The position across the track, from 0 to 1.
     * @ghidraAddress NTSC-U/C: 0x001d18c0
     * @ghidraAddress PAL: 0x001da660
     */
    void
    BlendTrack(Transform *pXfm, bool bSmoothBasis, float flTrack, float flTick, float flLateral);

    /**
     * Report the tick a little before the end of the built bars.
     *
     * @return The tick.
     * @ghidraAddress NTSC-U/C: 0x001d1af8
     * @ghidraAddress PAL: 0x001da898
     */
    float BuiltEndTick() const;

    /**
     * Work out the transform of a cell of a track.
     *
     * The X axis runs across the track and the Z axis away from its surface.
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
     * Work out the transform of a cell, sunk below the surface of the track.
     *
     * The position moves along the Z axis by one less flRaise.
     *
     * @param nTrack The track.
     * @param pXfm Receives the transform.
     * @param bSmoothBasis Blend the basis between the two nearest cross sections.
     * @param bTrackOffset Add the offset of the track to the position.
     * @param flTick The tick along the track.
     * @param flLateral The position across the track, from 0 to 1.
     * @param flRaise The height above the track, where 1 is the surface.
     * @ghidraAddress NTSC-U/C: 0x001d2040
     * @ghidraAddress PAL: 0x001dade0
     */
    void PlaceCell(int nTrack,
                   Transform *pXfm,
                   bool bSmoothBasis,
                   bool bTrackOffset,
                   float flTick,
                   float flLateral,
                   float flRaise);

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

    /**
     * Work out the transform of the path of the tunnel at a tick.
     *
     * A tunnel without a path animation gives the identity.
     *
     * @param pXfm Receives the transform.
     * @param flTick The tick.
     * @ghidraAddress NTSC-U/C: 0x001d2320
     * @ghidraAddress PAL: 0x001db0c0
     */
    void PathXfm(Transform *pXfm, float flTick);

    /**
     * Report the sections of a built panel.
     *
     * @param nTrack The track.
     * @param nBar The bar.
     * @param bDirty Mark the panel to be rebuilt.
     * @return The sections, or null for a bar that is not built.
     * @ghidraAddress NTSC-U/C: 0x001d2398
     * @ghidraAddress PAL: 0x001db138
     */
    std::vector<CrossSectionXfm> *GetSections(int nTrack, int nBar, bool bDirty);

    /**
     * Report a built panel.
     *
     * @param nTrack The track.
     * @param nBar The bar.
     * @return The panel, or null for a bar that is not built.
     * @ghidraAddress NTSC-U/C: 0x001d23d8
     * @ghidraAddress PAL: 0x001db178
     */
    PanelData *GetPanel(int nTrack, int nBar);

    /**
     * Put the sections of a built panel back as the bar was built.
     *
     * @param nTrack The track.
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x001d23f8
     * @ghidraAddress PAL: 0x001db198
     */
    void RestoreSections(int nTrack, int nBar);

    /**
     * Set the offset of a track, which CellXfm() adds in the basis of the cell.
     *
     * @param nTrack The track.
     * @param pOffset The offset.
     * @ghidraAddress NTSC-U/C: 0x001d2498
     * @ghidraAddress PAL: 0x001db238
     */
    void SetTrackOffset(int nTrack, const Vector3 *pOffset);

    /**
     * Set the colour of the panels of a track.
     *
     * @param nTrack The track.
     * @param pColor The colour.
     * @ghidraAddress NTSC-U/C: 0x001d2568
     * @ghidraAddress PAL: 0x001db308
     */
    void SetTrackColor(int nTrack, const Color *pColor);

    /**
     * Report the colour of the panels of a track.
     *
     * @param nTrack The track.
     * @return The colour.
     * @ghidraAddress NTSC-U/C: 0x001d2588
     * @ghidraAddress PAL: 0x001db328
     */
    Color *GetTrackColor(int nTrack);

    /**
     * Set the checkpoint material of a built bar in mBarValues.
     *
     * @param nBar The bar.
     * @param pMat The checkpoint material, or null.
     * @ghidraAddress NTSC-U/C: 0x001d25a0
     * @ghidraAddress PAL: 0x001db340
     */
    void SetBarValue(int nBar, RndMat *pMat);

    /**
     * Set the state of a built panel.
     *
     * @param nTrack The track.
     * @param nBar The bar.
     * @param bTransparent Whether the panel does not write depth.
     * @param pCheckpointMat The checkpoint material of the panel, or null.
     * @param bPickable Whether picks test the panel.
     * @param bHighlighted Whether the panel is drawn highlighted.
     * @param pColor The colour, or null to retain the colour.
     * @ghidraAddress NTSC-U/C: 0x001d25e0
     * @ghidraAddress PAL: 0x001db380
     */
    void SetPanelState(int nTrack,
                       int nBar,
                       char bTransparent,
                       RndMat *pCheckpointMat,
                       int bPickable,
                       bool bHighlighted,
                       const Color *pColor);

    /**
     * Set the colour of a built panel.
     *
     * @param nTrack The track.
     * @param nBar The bar.
     * @param pColor The colour, or null to retain the colour.
     * @param bHighlighted Whether the panel is drawn highlighted.
     * @ghidraAddress NTSC-U/C: 0x001d26a0
     * @ghidraAddress PAL: 0x001db440
     */
    void SetPanelColor(int nTrack, int nBar, const Color *pColor, bool bHighlighted);

    /**
     * Set the colour of every built panel.
     *
     * @param pColor The colour, or null for the colour of the track of each panel.
     * @param bHighlighted Whether the panels are drawn highlighted.
     * @ghidraAddress NTSC-U/C: 0x001d2700
     * @ghidraAddress PAL: 0x001db4a0
     */
    void SetAllPanelColors(const Color *pColor, bool bHighlighted);

    /**
     * Report the colour of a built panel.
     *
     * @param nTrack The track.
     * @param nBar The bar.
     * @return The colour, or null for a bar that is not built.
     * @ghidraAddress NTSC-U/C: 0x001d27e0
     * @ghidraAddress PAL: 0x001db580
     */
    Color *GetPanelColor(int nTrack, int nBar);

    /**
     * Set the first section of a built panel the mesh moves along.
     *
     * @param nTrack The track.
     * @param nBar The bar.
     * @param nSection The section.
     * @ghidraAddress NTSC-U/C: 0x001d2808
     * @ghidraAddress PAL: 0x001db5a8
     */
    void SetPanelStartSection(int nTrack, int nBar, char nSection);

    /**
     * Set the material drawn over a track.
     *
     * @param nTrack The track.
     * @param pMat The material, or null.
     * @param nMode How the overlay is drawn.
     * @ghidraAddress NTSC-U/C: 0x001d2848
     * @ghidraAddress PAL: 0x001db5e8
     */
    void SetTrackOverlay(int nTrack, RndMat *pMat, int nMode);

    /**
     * Set the material of a kind of panel, or the default material.
     *
     * @param pMat The material.
     * @param nKind The kind, or an index below kDefaultMatThreshold for the default material.
     * @ghidraAddress NTSC-U/C: 0x001d2868
     * @ghidraAddress PAL: 0x001db608
     */
    void SetPanelMat(RndMat *pMat, int nKind);

    /**
     * Blend the path to a new animation.
     *
     * @param pAnim The new animation.
     * @param bLoop Whether the new animation loops.
     * @param bUseTime Whether the frame of the new animation follows the time.
     * @param flLength The length of the blend in ticks.
     * @param flStart The tick the blend starts at.
     * @param flFrame The frame of the new animation at flStart.
     * @param flLoopStart The first frame of the loop of the new animation.
     * @param flLoopEnd The frame the loop of the new animation wraps at.
     * @ghidraAddress NTSC-U/C: 0x001d3498
     * @ghidraAddress PAL: 0x001dc238
     */
    void BlendPath(RndTransAnim *pAnim,
                   int bLoop,
                   int bUseTime,
                   float flLength,
                   float flStart,
                   float flFrame,
                   float flLoopStart,
                   float flLoopEnd);

    /**
     * Replace the path at once with an animation that starts at frame 0.
     *
     * @param pAnim The animation, or null for no path.
     * @ghidraAddress NTSC-U/C: 0x001d34b8
     * @ghidraAddress PAL: 0x001dc258
     */
    void SetPath(RndTransAnim *pAnim);

    /**
     * Find the built panel a segment strikes first.
     *
     * @param pSegment The segment.
     * @param pnTrack Receives the track of the panel.
     * @param pnBar Receives the bar of the panel.
     * @return Whether a panel was struck.
     * @ghidraAddress NTSC-U/C: 0x001cd6e0
     * @ghidraAddress PAL: 0x001d6480
     */
    bool Pick(const Segment *pSegment, int *pnTrack, int *pnBar);

    /**
     * A transform with no rotation and no translation.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b2d0
     */
    static Transform sIdentity;

    int mNumBars;        /*!< The bars of each track in the ring of built panels. */
    int mReserved04;     // +0x04 Starts at 2, the nDetail of Init(), not yet read.
    float mSlabDepth;    /*!< The depth of a slab under the panels. */
    float mSlabAngle;    /*!< The angle of the sides of a slab, in radians. */
    TnlPath *mPath;      /*!< The path of the tunnel. */
    int mBarsBehind;     /*!< The built bars kept behind the current bar. */
    float mMoveLength;   /*!< The ticks of a move of a player to the next track. */
    float mCamMoveSpeed; /*!< The tracks a camera moves per tick. */
    int mWraps;          /*!< Whether the tracks wrap around the tunnel. */
    int mShowPanels;     /*!< Whether the panels show, `show_panels`. */
    /*!< Whether players move between tracks along curves and BuildBars() builds one bar a call. */
    int mAnimateMoves;
    float mBarsPerTick;               /*!< The reciprocal of mTicksPerBar. */
    float mTicksPerBar;               /*!< The ticks of one bar. */
    int mSectionsPerBar;              /*!< The cross sections of one bar. */
    int mFirstBar;                    /*!< The first bar of the ring. */
    int mMinFirstBar;                 /*!< The least bar Poll() starts the ring at. */
    int mEndBar;                      /*!< The first bar that is never built. */
    std::vector<int> mBarRing;        /*!< The bar each slot of the ring holds. */
    std::vector<RndMat *> mBarMats;   /*!< The material of each slot of the ring. */
    std::vector<MeshHolder> mMeshes;  /*!< The opaque and the transparent source mesh. */
    std::vector<PanelData> mPanels;   /*!< The panels, mNumBars for each track. */
    std::vector<TrackData> mTracks;   /*!< The state of each track. */
    std::vector<RndMat *> mBarValues; /*!< The checkpoint material of each slot of the ring. */
    /*!< The material of each pass of the standard panel, `tnl std panel mat pass <n>`. */
    std::vector<RndMat *> mPassMats;
    /*!< The animations of the passes, `tnl std matanim stage <n>`. */
    std::vector<RndMatAnim *> mPassAnims;
    unsigned char mReservedC4[0xc]; // +0xc4 to +0xcf are not yet identified.
    /*!< The light colour DrawPanelsMultiplayer() gives the passes after the first for a panel that
         is not highlighted, opaque black, and for a highlighted panel, the diffuse colour of the
         material of the standard panel. */
    Color mPassLightColors[2];
    int mBarValuesDirty;                       /*!< Whether mBarValues changed. */
    RndMat *mDefaultMat;                       /*!< The material of a panel with no kind. */
    std::vector<RndMat *> mPanelMats;          /*!< The material of each kind of panel. */
    std::vector<Vector3> mTrackOffsets;        /*!< The offset of each track. */
    std::vector<bool> mTrackOffsetChanged;     /*!< Whether the offset of each track changed. */
    std::vector<TrackGeometry> mTrackGeometry; /*!< The flat cross section of each track. */
    int mNumTracks;                            /*!< The number of tracks. */
    std::vector<Player> mPlayers;              /*!< The players. */

private:
    /** Below this index SetPanelMat() sets the default material. */
    static constexpr int kDefaultMatThreshold = -16;

    /** The source meshes in mMeshes, indexed by PanelData::mTransparent. */
    enum SourceMesh {
        kSourceMeshOpaque = 0,      /*!< The mesh of an opaque panel, with a full slab. */
        kSourceMeshTransparent = 1, /*!< The mesh of a transparent panel, with a shallow slab. */
        kSourceMeshCount = 2,       /*!< The number of source meshes. */
    };

    /**
     * Delete the panels, the state of the tracks, and the materials and the animations of the
     * passes of the standard panel.
     *
     * @param bClear Whether to do anything at all.
     * @ghidraAddress NTSC-U/C: 0x001cef30
     * @ghidraAddress PAL: 0x001d7cd0
     */
    void Clear(bool bClear);

    /**
     * Rebuild the meshes of the built panels that are dirty or whose track moved.
     *
     * @ghidraAddress NTSC-U/C: 0x001cd808
     * @ghidraAddress PAL: 0x001d65a8
     */
    void RebuildPanels();

    /**
     * Replace mMeshes with kSourceMeshCount empty holders and build the source meshes.
     *
     * @ghidraAddress NTSC-U/C: 0x001cfd98
     * @ghidraAddress PAL: 0x001d8b38
     */
    void BuildMeshes();

    /**
     * Build each source mesh as wide as the first track, naming it `panel fx <n> src.mesh`.
     *
     * The transparent mesh takes a shallow slab and does not write depth.
     *
     * @ghidraAddress NTSC-U/C: 0x001cfea0
     * @ghidraAddress PAL: 0x001d8c40
     */
    void BuildSourceMeshes();

    /**
     * Create a panel mesh, `tunnel drawdummy`, of one row of four vertices for each cross section
     * of a bar.
     *
     * The top of a row runs across the panel from -0.5 to 0.5. The bottom sits a slab below it,
     * with its sides drawn in by the slab angle. The faces cover the top, the bottom, both sides,
     * and the end at the first row.
     *
     * @param flWidth The width of the track the mesh spans.
     * @param nLevel The doublings of the rows each quad of the faces spans.
     * @param pExtent Receives 1 across, the sections of a bar along, and 0 deep.
     * @param bSolid Whether the slab is mSlabDepth deep and opaque, rather than a quarter of it and
     *               clear.
     * @return The mesh.
     * @ghidraAddress NTSC-U/C: 0x001cf6f8
     * @ghidraAddress PAL: 0x001d8498
     */
    RndMesh *BuildPanelMesh(float flWidth, int nLevel, Vector3 *pExtent, bool bSolid);

    /**
     * Size the ring of panels, the tracks, and their offsets for the tracks Init() set, and build
     * the geometry of the tracks and the panels.
     *
     * Every slot of the ring starts with no bar, the default material, and no bar value, and
     * every track offset starts at zero and changed.
     *
     * @ghidraAddress NTSC-U/C: 0x001cdc48
     * @ghidraAddress PAL: 0x001d69e8
     */
    void BuildRing();

    /**
     * Work out the basis, the centre, the transform, and the rotation of each track from its
     * edges.
     *
     * @ghidraAddress NTSC-U/C: 0x001cece8
     * @ghidraAddress PAL: 0x001d7a88
     */
    void BuildTrackGeometry();

    /**
     * Size mPanels for every slot of every track, create the mesh of each panel that has none as
     * a copy of the opaque source mesh, and size the sections of each panel.
     *
     * @ghidraAddress NTSC-U/C: 0x001cf1d8
     * @ghidraAddress PAL: 0x001d7f78
     */
    void BuildPanels();

    /** The colours DrawPass() gives the material of a pass for each panel. */
    enum PassColor {
        kPassColorNone = 0, /*!< The material retains its colours. */
        kPassColorBase = 1, /*!< The base colour becomes the colour of the panel. */
        /*!< The light colour comes from mPassLightColors, then the base colour becomes the colour
             of the panel. */
        kPassColorLit = 2,
    };

    /**
     * Draw the opaque panels of the tracks with the bar values or the overlays, and the
     * transparent panels of the tracks with a player with the materials of their bars, from the
     * last bar back to the first.
     *
     * DrawPanels(), DrawPanelsMultiplayer(), and DrawOverlays() inline it.
     *
     * @param nFirstBar The first bar drawn.
     * @param nEndBar The bar after the last bar drawn.
     * @param bWriteDepth Whether each mesh writes depth while it draws.
     */
    void DrawBarValues(int nFirstBar, int nEndBar, bool bWriteDepth);

    /**
     * Draw the checkpoint material of each built panel of the tracks with no overlay mode, from
     * the last bar back to the first.
     *
     * DrawPanels() and DrawPanelsMultiplayer() inline it.
     *
     * @param nFirstBar The first bar drawn.
     * @param nEndBar The bar after the last bar drawn.
     */
    void DrawCheckpoints(int nFirstBar, int nEndBar);

    /**
     * Select a pass of the standard panel with depth writes and a greater or equal depth test,
     * and draw the faces of the opaque panels with no checkpoint of the tracks with no overlay
     * mode, from the last bar back to the first.
     *
     * DrawPanels() and DrawPanelsMultiplayer() inline it.
     *
     * @param pMat The material of the pass.
     * @param nFirstBar The first bar drawn.
     * @param nEndBar The bar after the last bar drawn.
     * @param nColor The colours the material takes for each panel.
     */
    void DrawPass(RndMat *pMat, int nFirstBar, int nEndBar, PassColor nColor);

    /**
     * Find a built panel.
     *
     * @param nTrack The track.
     * @param nBar The bar.
     * @return The panel, or null for a bar that is not built or a geometry with no panels.
     * @ghidraAddress NTSC-U/C: 0x001d2438
     * @ghidraAddress PAL: 0x001db1d8
     */
    PanelData *FindPanel(int nTrack, int nBar);
};
