#pragma once

#include <list>
#include <vector>

#include "math/color.h"
#include "math/key.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndmesh.h"

/**
 * Animation of a mesh's vertex positions, texture coordinates, and colours by keys.
 *
 * The RTTI includes the class name and records RndAnimatable as the one base. Each key holds a
 * value for every vertex. The keys may be shared, in which case mKeysOwner identifies the
 * animation that has them.
 */
class RndMeshAnim : public RndAnimatable {
public:
    /** The bit of the Copy() flags that shares the source's keys rather than copying them. */
    static constexpr int kCopyShareKeys = 0x40;

    /**
     * Construct an animation of nothing without keys.
     *
     * The animation is its own keys owner. The binary expands the constructor inline in New().
     *
     * @param pszName The registry key.
     */
    explicit RndMeshAnim(const char *pszName);

    /**
     * Drop the references on the mesh and the keys owner.
     *
     * @ghidraAddress NTSC-U/C: 0x0038e548
     */
    ~RndMeshAnim() override;

    /**
     * Report the last frame of any key.
     *
     * @return The frame.
     * @ghidraAddress NTSC-U/C: 0x00235c50
     * @ghidraAddress PAL: 0x0023e7d0
     */
    float EndFrame() override;

    /**
     * Add the mesh to a list, then the objects of the children.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x00235b90
     * @ghidraAddress PAL: 0x0023e710
     */
    void ListAnimObjects(std::list<RndObject *> &objects) override;

    /**
     * Apply a frame to the vertices of the mesh's geometry, then rebuild the parts that changed.
     *
     * Only as many vertices as both the key and the geometry have are set.
     *
     * @param fFrame The frame.
     * @return 1, to pass the frame on to the children.
     * @ghidraAddress NTSC-U/C: 0x00235d30
     * @ghidraAddress PAL: 0x0023e8b0
     */
    int SetFrameSelf(float fFrame) override;

    /**
     * Write a description of the animation and its base.
     *
     * The mesh appears under the label "light:".
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002355f0
     * @ghidraAddress PAL: 0x0023e170
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the animation and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00235730
     * @ghidraAddress PAL: 0x0023e2b0
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a referenced object with another.
     *
     * A keys owner replaced by null becomes the animation itself.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00235470
     * @ghidraAddress PAL: 0x0023df90
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x0038e7b8
     * @ghidraAddress PAL: 0x003fcec0
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another animation and its base.
     *
     * The keys are copied when the source has its own and the flags do not include
     * kCopyShareKeys. Otherwise the source's keys owner becomes this one's.
     *
     * @param pSource The animation to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x00235aa8
     * @ghidraAddress PAL: 0x0023e628
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x002358b8
     * @ghidraAddress PAL: 0x0023e438
     */
    void Load(BinStream &stream) override;

    /**
     * Set the mesh the animation drives.
     *
     * @param pMesh The mesh, or null.
     * @ghidraAddress NTSC-U/C: 0x00235410
     */
    void SetMesh(RndMesh *pMesh);

    /**
     * Create an animation.
     *
     * @param pszName The registry key.
     * @return The animation.
     * @ghidraAddress NTSC-U/C: 0x0038e7d0
     */
    static RndObject *New(const char *pszName) {
        return new RndMeshAnim(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `MeshAnim`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09f4
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09f8
     */
    static int sRev;

    RndMesh *mMesh;                                         /*!< The mesh the animation drives. */
    std::vector<Key<std::vector<Vector3>>> mVertPointsKeys; /*!< The vertex position keys. */
    std::vector<Key<std::vector<Vector2>>> mVertTexsKeys;   /*!< The texture coordinate keys. */
    std::vector<Key<std::vector<Color>>> mVertColorsKeys;   /*!< The vertex colour keys. */
    RndMeshAnim *mKeysOwner;                                /*!< The animation that has the keys. */

protected:
    /**
     * Drop the references on the mesh and the keys owner.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00235818
     * @ghidraAddress PAL: 0x0023e398
     */
    void ReleaseRefs();

    /**
     * Take references on the mesh and the keys owner.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00235868
     * @ghidraAddress PAL: 0x0023e3e8
     */
    void AcquireRefs();
};
