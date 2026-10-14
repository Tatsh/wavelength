#pragma once

#include <list>
#include <vector>

#include "math/transform.h"
#include "math/vector3.h"
#include "rnd/multimesh.h"

class GfxTunnel;
class TnlGeom;

/**
 * The grid lines across the track of a player, instances of the `grid.mm` multi-mesh.
 *
 * The class is not polymorphic and emits no RTTI, so the name is inferred. RTTI identifies its
 * GridLine. The object is 0x30 bytes and lives inline in the per-player object of the tunnel. The
 * lines lie kLineTicks apart over kSpanTicks, alternating between a quarter and an eighth line.
 * Each update moves a line that has fallen kRecycleTicks behind the song tick to the far end of
 * the span, and a line shows only when the bar under it has an owner.
 */
class TnlGrid {
public:
    /**
     * One line of the grid, an instance of the multi-mesh.
     *
     * RTTI identifies the class in the anonymous namespace of the tunnel's source file. The class
     * is not polymorphic, and the object is 0x10 bytes. A shown line's transform sits in the
     * instances of the multi-mesh, and a hidden line's transform sits in the hidden list of its
     * grid.
     */
    class GridLine {
    public:
        /**
         * Make a line of no scale. The constructor sets nothing else.
         *
         * @ghidraAddress NTSC-U/C: 0x001d39d8
         * @ghidraAddress PAL: 0x001dc778
         */
        GridLine();

        /**
         * Destroy the line. The transform stays where it is.
         *
         * @ghidraAddress NTSC-U/C: 0x001d39e8
         * @ghidraAddress PAL: 0x001dc788
         */
        ~GridLine();

        /**
         * Add a transform to the front of the instances of a multi-mesh and take it as a shown
         * line.
         *
         * @param pScale The scale of the basis rows of the line.
         * @param pMesh The multi-mesh.
         * @ghidraAddress NTSC-U/C: 0x001d3a10
         * @ghidraAddress PAL: 0x001dc7b0
         */
        void Attach(const Vector3 *pScale, Rnd::MultiMesh *pMesh);

        /**
         * Place the line across the middle of a track at a tick, just below the surface, scale
         * its basis rows, and record the tick.
         *
         * @param pGeom The geometry of the tracks.
         * @param nTrack The track.
         * @param nTick The tick.
         * @ghidraAddress NTSC-U/C: 0x001d3ae8
         * @ghidraAddress PAL: 0x001dc888
         */
        void Place(TnlGeom *pGeom, int nTrack, int nTick);

        /**
         * Move the transform to the front of the instances of the multi-mesh to show the line, or
         * to the back of the hidden list to hide it, unless the line is already so.
         *
         * @param bShowing Whether to show the line.
         * @param pMesh The multi-mesh.
         * @param hidden The hidden transforms of the grid.
         * @ghidraAddress NTSC-U/C: 0x001d3be0
         * @ghidraAddress PAL: 0x001dc980
         */
        void SetShowing(bool bShowing, Rnd::MultiMesh *pMesh, std::list<Transform> &hidden);

        const Vector3 *mScale;                    /*!< The scale of the basis rows. */
        bool mShowing;                            /*!< Whether the transform is an instance. */
        std::list<Transform>::iterator mInstance; /*!< The transform of the line. */
        float mTick;                              /*!< The tick the line was last placed at. */
    };

    /**
     * Make the grid of a camera. For a camera number of 0 or less the grid has no mesh and no
     * lines. Camera 1 empties the instances of `grid.mm` first, and every camera then adds
     * kNumLines lines on track 0 from tick 0.
     *
     * @param pTunnel The tunnel.
     * @param nCam The camera index plus one.
     * @ghidraAddress NTSC-U/C: 0x001d3cc0
     * @ghidraAddress PAL: 0x001dca60
     */
    TnlGrid(GfxTunnel *pTunnel, int nCam);

    /**
     * Empty the instances of the multi-mesh, the transforms of every other grid included, and
     * destroy the hidden transforms.
     *
     * @ghidraAddress NTSC-U/C: 0x001d3ef0
     * @ghidraAddress PAL: 0x001dcc90
     */
    ~TnlGrid();

    /**
     * Move the grid to a track, placing again every line that is not yet kRecycleTicks behind the
     * song tick, and reconsider every line at the next update.
     *
     * @param nTrack The track.
     * @ghidraAddress NTSC-U/C: 0x001d3fb8
     * @ghidraAddress PAL: 0x001dcd58
     */
    void SetTrack(int nTrack);

    /**
     * Show the lines only for the first of the players, and reconsider every line at the next
     * update.
     *
     * @param nOrder The place of the player among the players, from 0.
     * @ghidraAddress NTSC-U/C: 0x001d4078
     * @ghidraAddress PAL: 0x001dce18
     */
    void SetOrder(int nOrder);

    /**
     * Respond to a change of a track. For the grid's track, place the lines again at once, or
     * reconsider every line at the next update.
     *
     * @param nTrack The track that changed.
     * @param bPlace Whether to place the lines at once.
     * @ghidraAddress NTSC-U/C: 0x001d4090
     * @ghidraAddress PAL: 0x001dce30
     */
    void RefreshTrack(int nTrack, bool bPlace);

    /**
     * Move the lines that fell behind the song tick, or lie beyond the span after a jump back, to
     * the far end of the span, and show or hide each moved line by the owner of its bar. With a
     * pending refresh every line is reconsidered.
     *
     * @ghidraAddress NTSC-U/C: 0x001d40d0
     * @ghidraAddress PAL: 0x001dce70
     */
    void Update();

    /**
     * The scale of a quarter line, `grid_quarter_scale` scaled to the tunnel. The tunnel sets it
     * when it builds its panels.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b3e0
     */
    static Vector3 sQuarterScale;

    /**
     * The scale of an eighth line, `grid_eighth_scale` scaled to the tunnel. The tunnel sets it
     * when it builds its panels.
     *
     * @ghidraAddress NTSC-U/C: 0x0043b400
     */
    static Vector3 sEighthScale;

    /** The lines of a grid. */
    static constexpr int kNumLines = 32;

    /** The ticks between two lines. */
    static constexpr float kLineTicks = 240.0f;

    /** The ticks the lines span, kNumLines times kLineTicks. */
    static constexpr float kSpanTicks = 7680.0f;

    /** The ticks a line falls behind the song tick before it moves to the far end of the span. */
    static constexpr float kRecycleTicks = 960.0f;

    std::vector<GridLine> mLines; /*!< The lines. */
    int mTrack;                   /*!< The track the lines lie across. */
    GfxTunnel *mTunnel;           /*!< The tunnel. */
    Rnd::MultiMesh *mMesh;        /*!< The `grid.mm` multi-mesh, shared by every grid. */
    std::list<Transform> mHidden; /*!< The transforms of the hidden lines. */
    bool mRefresh;                /*!< Whether the next update reconsiders every line. */
    bool mShowLines;              /*!< Whether the lines may show, for the first player only. */
    int mReserved2c;              // +0x2c Not set or read by the class.
};
