#pragma once

#include <cstddef>
#include <memory>

#include "math/transform.h"
#include "os/mem.h"
#include "os/memhandle.h"
#include "rnd/rndmesh.h"

/**
 * Mesh, PlayStation 2.
 *
 * The RTTI includes the class name and records RndMesh as the one base. Sync() turns the faces and
 * the edges into VIF packets for the VU1 microprogram and then releases them. Each packet is a run
 * of batches. A batch unpacks the vertices it uses and the indices of its faces or edges, and then
 * starts the microprogram. A mesh that is not mutable has its vertices copied into the packets and
 * releases them too. A mutable mesh retains them, and its packets refer to them with DMA REF tags.
 * A mesh with bones is mutable, and SkinXfm() moves its vertices into world space every frame.
 */
class PsMesh : public RndMesh {
public:
    /** The number of entries of g_vertexCache. */
    static constexpr int kVertexCacheSize = 256;

    /**
     * The bind pose of a skinned vertex, with the weight of each bone in the fourth word.
     *
     * The structure is not polymorphic and emits no RTTI descriptor. The name is inferred. The
     * skinning code reads each half with a quadword load, so the structure starts on a quadword.
     */
    struct alignas(16) SkinVert {
        float mPos[3];  /*!< The position. */
        float mWeight1; /*!< The weight of the first bone. */
        float mNorm[3]; /*!< The normal. */
        float mWeight2; /*!< The weight of the second bone. */
    };

    /**
     * Construct a mesh without packets.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00216050
     * @ghidraAddress PAL: 0x0021ee68
     */
    explicit PsMesh(const char *pszName);

    /**
     * Release the packets after the frame and the bind pose.
     *
     * @ghidraAddress NTSC-U/C: 0x00214e88
     * @ghidraAddress PAL: 0x0021dca0
     */
    ~PsMesh() override;

    /**
     * Allocate a mesh from its pool.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolAlloc(static_cast<int>(nSize), sizeof(PsMesh), "PsMesh", 0);
    }

    /**
     * Return a mesh to its pool.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolFree(sizeof(PsMesh), pBlock);
    }

    /**
     * Copy another mesh, then copy each of its packets this mesh does not have.
     *
     * The source has to be a PsMesh.
     *
     * @param pSource The mesh to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x00213f68
     * @ghidraAddress PAL: 0x0021cd80
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Rebuild the bind pose, the face packet, and the edge packet a set of flags covers, on the
     * mesh that has the geometry.
     *
     * The faces and the edges are released once their packet is built, and the vertices are
     * released unless the mesh is mutable. A mesh with bones becomes mutable.
     *
     * @param nFlags The RndMesh::SyncFlags.
     * @ghidraAddress NTSC-U/C: 0x00214af8
     * @ghidraAddress PAL: 0x0021d910
     */
    void Sync(int nFlags) override;

    /**
     * Draw each pass of the faces, then the edges, when the level of detail selects the mesh.
     *
     * @return 1 when the mesh is drawn, or 0.
     * @ghidraAddress NTSC-U/C: 0x00215f30
     * @ghidraAddress PAL: 0x0021ed48
     */
    int DrawShowing() override;

    /**
     * Build the face packet from the faces and the vertices.
     *
     * The previous packet is released after the frame. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00214150
     * @ghidraAddress PAL: 0x0021cf68
     */
    void BuildFacesPacket();

    /**
     * Build the edge packet from the edges and the vertices.
     *
     * The previous packet is released after the frame. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00214710
     * @ghidraAddress PAL: 0x0021d528
     */
    void BuildEdgesPacket();

    /**
     * Set the GS depth test and depth write of a pass from the depth mode and comparison.
     *
     * Passes 0 and 1 draw faces and edges. A later pass changes nothing. Edges never write depth.
     * The name is inferred.
     *
     * @param nPass The pass.
     * @ghidraAddress NTSC-U/C: 0x002154e0
     * @ghidraAddress PAL: 0x0021e2f8
     */
    void SetZTest(int nPass);

    /**
     * Send the settings of the face microprogram and the face packet of the mesh with the geometry.
     *
     * The name is inferred.
     *
     * @param xfm The transform of the mesh.
     * @ghidraAddress NTSC-U/C: 0x002155b0
     * @ghidraAddress PAL: 0x0021e3c8
     */
    void DrawFaces(const Transform &xfm);

    /**
     * Send the settings of the edge microprogram and the edge packet of the mesh with the geometry.
     *
     * The edges take the edge colour of the material, or white without one. The name is inferred.
     *
     * @param xfm The transform of the mesh.
     * @ghidraAddress NTSC-U/C: 0x002159b0
     * @ghidraAddress PAL: 0x0021e7c8
     */
    void DrawEdges(const Transform &xfm);

    /**
     * Report the transform to draw the mesh with, skinning its vertices first when it has bones.
     *
     * A skinned mesh has its vertices moved into world space on VU0 and draws with the identity.
     * The name is inferred.
     *
     * @return The transform.
     * @ghidraAddress NTSC-U/C: 0x00215d18
     * @ghidraAddress PAL: 0x0021eb30
     */
    const Transform &SkinXfm();

    /**
     * Report the face packet of the mesh with the geometry and how the first batch is laid out.
     *
     * The name is inferred.
     *
     * @param pPacket Receives the face packet.
     * @param nSetupQwords Receives the quadwords up to and including the start of the first batch.
     * @param nVuEnd Receives the VU1 address after the data of the last batch.
     * @ghidraAddress NTSC-U/C: 0x002161b0
     * @ghidraAddress PAL: 0x0021efc8
     */
    void FacesPacket(MemHandle *&pPacket, int &nSetupQwords, int &nVuEnd) const;

    /**
     * Install the mesh factory for the class `Mesh` of a `.rnd` file, and set g_identityXfm.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x002161d0
     * @ghidraAddress PAL: 0x0021efe8
     */
    static void Init();

    /**
     * Create a mesh.
     *
     * @param pszName The registry key.
     * @return The mesh.
     * @ghidraAddress NTSC-U/C: 0x0037cc78
     * @ghidraAddress PAL: 0x003eb3a8
     */
    static RndObject *New(const char *pszName) {
        return new PsMesh(pszName);
    }

    std::unique_ptr<SkinVert[]> mSkinVerts; /*!< The bind pose of a mesh with bones, or null. */

    /** @return The number of faces before Sync() released them. */
    unsigned short GetNumFaces() const {
        return mNumFaces;
    }

private:
    MemHandle *mFacesPacket;          /*!< The face packet, or null. */
    MemHandle *mEdgesPacket;          /*!< The edge packet, or null. */
    unsigned short mFacesQwords;      /*!< The size of the face packet in quadwords. */
    unsigned short mEdgesQwords;      /*!< The size of the edge packet in quadwords. */
    unsigned short mNumFaces;         /*!< The number of faces before Sync() released them. */
    unsigned short mNumEdges;         /*!< The number of edges before Sync() released them. */
    unsigned short mFacesSetupQwords; /*!< The quadwords FacesPacket() reports. */
    unsigned short mFacesVuEnd;       /*!< The VU1 address FacesPacket() reports. */
};

/**
 * The identity transform a skinned mesh draws with.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c3c0
 */
extern Transform g_identityXfm;

/**
 * The vertex indices of the batch being built, in the order the batch unpacks them.
 *
 * @ghidraAddress NTSC-U/C: 0x0043c400
 */
extern unsigned short g_vertexCache[PsMesh::kVertexCacheSize];
