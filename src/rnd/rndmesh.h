#pragma once

#include <cstddef>
#include <list>
#include <vector>

#include "math/box.h"
#include "math/color.h"
#include "math/plane.h"
#include "math/sphere.h"
#include "math/transform.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "rnd/rndcollideable.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndmat.h"
#include "rnd/rndtransformable.h"

/**
 * Triangle mesh with a material.
 *
 * The RTTI includes the class name and records RndDrawable, RndTransformable, and RndCollideable
 * as bases. The geometry may be shared, in which case mGeomOwner identifies the mesh that has it,
 * and the transform may come from another transformable, mTransOwner. A chain of lower detail
 * meshes follows mNext.
 */
class RndMesh : public RndDrawable, public RndTransformable, public RndCollideable {
public:
    /** How drawing reads and writes depth. */
    enum ZMode {
        kZModeDisable = 0,    /*!< No depth test or write. */
        kZModeZReadOnly = 1,  /*!< Z depth test without a write. */
        kZModeZReadWrite = 2, /*!< Z depth test and write. */
        kZModeWReadOnly = 3,  /*!< W depth test without a write. */
        kZModeWReadWrite = 4, /*!< W depth test and write. */
    };

    /** The depth comparison. */
    enum ZFunc {
        kZFuncNever = 0,        /*!< Never pass. */
        kZFuncLess = 1,         /*!< Pass when nearer. */
        kZFuncEqual = 2,        /*!< Pass when equal. */
        kZFuncLessEqual = 3,    /*!< Pass when nearer or equal. */
        kZFuncGreater = 4,      /*!< Pass when farther. */
        kZFuncNotEqual = 5,     /*!< Pass when different. */
        kZFuncGreaterEqual = 6, /*!< Pass when farther or equal. */
        kZFuncAlways = 7,       /*!< Always pass. */
    };

    /** The bit of the Copy() flags that shares the source's geometry rather than copying it. */
    static constexpr int kCopyShareGeometry = 8;

    /** The bit of the Copy() flags that copies the bones. */
    static constexpr int kCopyBones = 0x20;

    /** The parts of the geometry Sync() rebuilds. */
    enum SyncFlags {
        kSyncPositions = 1,     /*!< The vertex positions. */
        kSyncNormals = 4,       /*!< The vertex normals. */
        kSyncColors = 8,        /*!< The vertex colours. */
        kSyncTexCoords = 0x10,  /*!< The vertex texture coordinates. */
        kSyncVertexData = 0x1f, /*!< Every vertex field. */
        kSyncFaces = 0x20,      /*!< The faces. */
        kSyncEdges = 0x40,      /*!< The edges. */
        kSyncAll = 0x7f,        /*!< Everything. */
    };

    /**
     * One vertex.
     *
     * The RTTI includes the nested class name.
     */
    class Vert {
    public:
        /** Construct a white vertex at the origin with no normal and no weights. */
        Vert() {
            mWeight1 = 0.0f;
            mWeight2 = 0.0f;
            mPos = Vector3{0.0f, 0.0f, 0.0f, 1.0f};
            mNorm = Vector3{0.0f, 0.0f, 0.0f, 1.0f};
            mColor.g = 1.0f;
            mColor.a = 1.0f;
            mColor.b = 1.0f;
            mColor.r = 1.0f;
            mTex.y = 0.0f;
            mTex.x = 0.0f;
        }

        Vector3 mPos;   /*!< The position. */
        Vector3 mNorm;  /*!< The normal. */
        Color mColor;   /*!< The colour. */
        Vector2 mTex;   /*!< The texture coordinates. */
        float mWeight1; /*!< The weight of the first bone transform. */
        float mWeight2; /*!< The weight of the second bone transform. */
    };

    /**
     * One triangle, as indices into the vertices.
     *
     * The RTTI includes the nested class name.
     */
    class Face {
    public:
        /** Construct a triangle of the first vertex three times. */
        Face() : mVerts{} {
        }

        unsigned short mVerts[3]; /*!< The three vertex indices. */
    };

    /**
     * One edge, as indices into the vertices.
     *
     * The RTTI includes the nested class name.
     */
    class Edge {
    public:
        /** Construct an edge from the first vertex to itself. */
        Edge() : mVerts{} {
        }

        unsigned short mVerts[2]; /*!< The two vertex indices. */
    };

    /**
     * The two bone transforms the vertex weights blend.
     *
     * The structure is not polymorphic and emits no RTTI descriptor, so the name is inferred.
     */
    struct Bones {
        RndTransformable *mTrans[2]; /*!< The two bones. */
        Transform mXfms[2];          /*!< The bind transform of each bone. */
    };

    /**
     * Construct an empty mesh that draws with a Z depth test and write, passing when nearer.
     *
     * The mesh is its own geometry owner and its own transform owner.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00232570
     * @ghidraAddress PAL: 0x0023b138
     */
    explicit RndMesh(const char *pszName);

    /**
     * Drop the references the mesh has.
     *
     * @ghidraAddress NTSC-U/C: 0x0038b0b0
     * @ghidraAddress PAL: 0x003f9718
     */
    ~RndMesh() override;

    /**
     * Allocate a mesh, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Mesh", 0);
    }

    /**
     * Release a mesh.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Add the mesh, its material, its owners, the textures of the material stages from the last,
     * and the bones to a list.
     *
     * The children are not added.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x00234ee8
     * @ghidraAddress PAL: 0x0023d978
     */
    void ListDrawObjects(std::list<RndObject *> &objects) override;

    /**
     * Add the mesh and then its children to a list, unless the mesh is outside the view of the
     * current camera.
     *
     * @param drawables The list to add to.
     * @ghidraAddress NTSC-U/C: 0x002352d0
     * @ghidraAddress PAL: 0x0023de50
     */
    void ListDrawables(std::list<RndDrawable *> &drawables) override;

    /**
     * Rebuild what the back end derives from parts of the geometry.
     *
     * The base body does nothing. The name is inferred.
     *
     * @param nFlags The SyncFlags bits of the parts.
     * @ghidraAddress NTSC-U/C: 0x0038b610
     * @ghidraAddress PAL: 0x003f9d18
     */
    virtual void Sync([[maybe_unused]] int nFlags) {
    }

    using RndCollideable::Collide;

    /**
     * Add each face the segment strikes, then what the children strike, to a list.
     *
     * A hidden mesh adds nothing, and nor does a mesh whose bounding sphere the segment misses.
     *
     * @param segment The segment, in world space.
     * @param collisions The list to add to.
     * @ghidraAddress NTSC-U/C: 0x00232060
     * @ghidraAddress PAL: 0x0023ac28
     */
    void Collide(const Segment &segment, std::list<Collision> &collisions) override;

    /**
     * Write a description of the mesh and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00232d70
     * @ghidraAddress PAL: 0x0023b938
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the mesh and its bases.
     *
     * Without bones, the mesh writes a 0 where the first bone name would go.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00233038
     * @ghidraAddress PAL: 0x0023bc00
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a referenced object with another.
     *
     * An owner replaced by null becomes the mesh itself. Bones without a first bone are deleted.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00234598
     * @ghidraAddress PAL: 0x0023d118
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x0038b5e8
     * @ghidraAddress PAL: 0x003f9cf0
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another mesh and its bases.
     *
     * The geometry is copied when the source has its own and the flags do not request sharing.
     * Otherwise the source's geometry owner becomes this one's.
     *
     * @param pSource The mesh to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x00234398
     * @ghidraAddress PAL: 0x0023cf18
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00233738
     * @ghidraAddress PAL: 0x0023c2f0
     */
    void Load(BinStream &stream) override;

    /**
     * Set the material.
     *
     * @param pMat The material, or null.
     * @ghidraAddress NTSC-U/C: 0x00232500
     * @ghidraAddress PAL: 0x0023b0c8
     */
    void SetMat(RndMat *pMat);

    /**
     * Report whether the mesh itself is to be drawn, drawing a lower detail mesh in its place
     * when the current camera shows it too small.
     *
     * A mesh without a bounding sphere is always drawn, and one outside the view never is. The
     * name is inferred.
     *
     * @param worldSphere Receives the bounding sphere in world space when the mesh has one.
     * @return Whether to draw the mesh.
     * @ghidraAddress NTSC-U/C: 0x00232b30
     * @ghidraAddress PAL: 0x0023b6f8
     */
    bool CheckLod(Sphere &worldSphere);

    /**
     * Compute the bounding box of the vertices in the mesh's own space.
     *
     * A mesh without vertices reports a box with both corners at the origin.
     *
     * @param box Receives the box.
     * @ghidraAddress NTSC-U/C: 0x00234d50
     * @ghidraAddress PAL: 0x0023d8d0
     */
    void BoundingBox(Box &box);

    /**
     * Move every vertex of the geometry by a transform, then rebuild the positions and normals.
     *
     * The normals turn with the basis and are normalised. The name is inferred.
     *
     * @param xfm The transform.
     * @ghidraAddress NTSC-U/C: 0x00234df8
     */
    void TransformVerts(const Transform &xfm);

    /**
     * Create a mesh.
     *
     * @param pszName The registry key.
     * @return The mesh.
     * @ghidraAddress NTSC-U/C: 0x0038b650
     * @ghidraAddress PAL: 0x003f9d58
     */
    static RndObject *New(const char *pszName) {
        return new RndMesh(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `Mesh`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09ec
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09f0
     */
    static int sRev;

    /**
     * The version Load() read last, which the vertex and face readers read by.
     *
     * @ghidraAddress NTSC-U/C: 0x0043c690
     */
    static int sLoadRev;

    int mZMode;                    /*!< The ZMode. */
    int mZFunc;                    /*!< The ZFunc. */
    std::vector<Vert> mVerts;      /*!< The vertices. */
    std::vector<Face> mFaces;      /*!< The triangles. */
    std::vector<Edge> mEdges;      /*!< The edges. */
    RndMat *mMat;                  /*!< The material, or null. */
    Sphere mSphere;                /*!< The bounding sphere, of radius 0 for none. */
    RndMesh *mGeomOwner;           /*!< The mesh that has the geometry. */
    RndTransformable *mTransOwner; /*!< The transformable whose world transform places the mesh. */
    Bones *mBones;                 /*!< The bones, or null. */
    float mMinScreen;              /*!< The least projected size to draw at, or 0 for any. */
    RndMesh *mNext;                /*!< The next lower detail mesh, or null. */
    int mMutable;                  /*!< Non-zero when the geometry changes at run time. */

protected:
    /**
     * Drop the references on the next mesh, the material, the owners, and the bones, and delete
     * the bones.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00233598
     * @ghidraAddress PAL: 0x0023c150
     */
    void ReleaseRefs();

    /**
     * Take references on the next mesh, the material, the owners, and the bones, then rebuild
     * everything.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00233660
     * @ghidraAddress PAL: 0x0023c218
     */
    void AcquireRefs();
};

/**
 * Write a vertex.
 *
 * @param stream The stream to write to.
 * @param vert The vertex.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00232c98
 * @ghidraAddress PAL: 0x0023b860
 */
PrnStream &operator<<(PrnStream &stream, const RndMesh::Vert &vert);

/**
 * Write a triangle.
 *
 * @param stream The stream to write to.
 * @param face The triangle.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002349e0
 * @ghidraAddress PAL: 0x0023d560
 */
PrnStream &operator<<(PrnStream &stream, const RndMesh::Face &face);

/**
 * Write an edge.
 *
 * @param stream The stream to write to.
 * @param edge The edge.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00234b08
 * @ghidraAddress PAL: 0x0023d688
 */
PrnStream &operator<<(PrnStream &stream, const RndMesh::Edge &edge);

/**
 * Write the name of a depth mode. An unknown mode writes nothing.
 *
 * @param stream The stream to write to.
 * @param eMode The mode.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00234b80
 * @ghidraAddress PAL: 0x0023d700
 */
PrnStream &operator<<(PrnStream &stream, RndMesh::ZMode eMode);

/**
 * Write the name of a depth comparison. An unknown comparison writes nothing.
 *
 * @param stream The stream to write to.
 * @param eFunc The comparison.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00234c38
 * @ghidraAddress PAL: 0x0023d7b8
 */
PrnStream &operator<<(PrnStream &stream, RndMesh::ZFunc eFunc);

/**
 * Write a vertex as its position, weights, normal, colour, and texture coordinates.
 *
 * The binary expands the writer inline in the vertex vector writer.
 *
 * @param stream The stream to write to.
 * @param vert The vertex.
 * @return The stream.
 */
BinStream &operator<<(BinStream &stream, const RndMesh::Vert &vert);

/**
 * Read a vertex the vertex writer wrote, or an earlier version of it.
 *
 * The version is the one RndMesh::Load() read last.
 *
 * @param stream The stream to read from.
 * @param vert Receives the vertex.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00234248
 * @ghidraAddress PAL: 0x0023cdc8
 */
BinStream &operator>>(BinStream &stream, RndMesh::Vert &vert);

/**
 * Write a triangle as its three indices.
 *
 * The binary expands the writer inline in the triangle vector writer at `0x003893e0`.
 *
 * @param stream The stream to write to.
 * @param face The triangle.
 * @return The stream.
 */
BinStream &operator<<(BinStream &stream, const RndMesh::Face &face);

/**
 * Read a triangle the triangle writer wrote, or an earlier version of it.
 *
 * The version is the one RndMesh::Load() read last.
 *
 * @param stream The stream to read from.
 * @param face Receives the triangle.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00234a70
 * @ghidraAddress PAL: 0x0023d5f0
 */
BinStream &operator>>(BinStream &stream, RndMesh::Face &face);

/**
 * Write an edge as its two indices.
 *
 * The binary expands the writer inline in the edge vector writer at `0x003894b0`.
 *
 * @param stream The stream to write to.
 * @param edge The edge.
 * @return The stream.
 */
BinStream &operator<<(BinStream &stream, const RndMesh::Edge &edge);

/**
 * Read an edge the edge writer wrote.
 *
 * The binary expands the reader inline in the edge vector reader at `0x00389768`.
 *
 * @param stream The stream to read from.
 * @param edge Receives the edge.
 * @return The stream.
 */
BinStream &operator>>(BinStream &stream, RndMesh::Edge &edge);
