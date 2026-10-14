#pragma once

/**
 * Per-frame counters the renderer maintains and Ps::DumpStats() reports.
 *
 * The structure is not polymorphic and emits no RTTI descriptor. The name is inferred. Each
 * title comes from the label Ps::DumpStats() prints.
 */
struct RenderStats {
    int mPoints;        /*!< Points drawn. */
    int mDraws;         /*!< Meshes drawn. */
    int mCulledTris;    /*!< Triangles culled. */
    int mClippedTris;   /*!< Triangles clipped. */
    int mTris;          /*!< Triangles drawn. */
    int mCulledLines;   /*!< Lines culled. */
    int mLines;         /*!< Lines drawn. */
    int mCulledSprites; /*!< Sprites culled. */
    int mSprites;       /*!< Sprites drawn. */
    int mVerts;         /*!< Vertices transformed. */
    int mTags;          /*!< GIF tags written. */
    int mPackets;       /*!< Packets sent. */
    int mMats;          /*!< Materials applied. */
    int mLitVerts;      /*!< Vertices lit. */
    int mTexSwaps;      /*!< Textures bound. */
    int mTexLoads;      /*!< Textures uploaded. */
};

/**
 * The renderer's counters, cleared by Ps::BeginFrame().
 *
 * @ghidraAddress NTSC-U/C: 0x0043bea8
 */
extern RenderStats g_renderStats;
