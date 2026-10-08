#pragma once

#include <list>

#include "math/transform.h"
#include "os/binstream.h"
#include "os/prnstream.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndmesh.h"

/**
 * Mesh drawn once at each of a list of transforms.
 *
 * The RTTI includes the class name and records RndDrawable as the one base. Each draw moves the
 * mesh to an instance transform. The mesh is moved back afterwards.
 */
class RndMultiMesh : public RndDrawable {
public:
    /**
     * Construct a multimesh without a mesh or instances.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x0023b9e8
     * @ghidraAddress PAL: 0x00244568
     */
    explicit RndMultiMesh(const char *pszName);

    /**
     * Drop the reference on the mesh.
     *
     * @ghidraAddress NTSC-U/C: 0x0023b798
     * @ghidraAddress PAL: 0x00244318
     */
    ~RndMultiMesh() override;

    /**
     * Add the draw objects of the mesh to a list.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x0023bcd0
     * @ghidraAddress PAL: 0x00244850
     */
    void ListDrawObjects(std::list<RndObject *> &objects) override;

    /**
     * Add the multimesh, when the mesh has something to draw at an instance, then the children,
     * to a list.
     *
     * @param drawables The list to add to.
     * @ghidraAddress NTSC-U/C: 0x0023bd08
     * @ghidraAddress PAL: 0x00244888
     */
    void ListDrawables(std::list<RndDrawable *> &drawables) override;

    /**
     * Draw the mesh at each instance.
     *
     * @return 1, to draw the children.
     * @ghidraAddress NTSC-U/C: 0x0023b5d8
     * @ghidraAddress PAL: 0x00244158
     */
    int DrawShowing() override;

    /**
     * Write a description of the multimesh and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0023b228
     * @ghidraAddress PAL: 0x00243da8
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the multimesh and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0023b2d8
     * @ghidraAddress PAL: 0x00243e58
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a child, or the mesh, with another object.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x0023b500
     * @ghidraAddress PAL: 0x00244080
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00390158
     * @ghidraAddress PAL: 0x003fe860
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another multimesh and its base.
     *
     * @param pSource The multimesh to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x0023b178
     * @ghidraAddress PAL: 0x00243cf8
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0023b370
     * @ghidraAddress PAL: 0x00243ef0
     */
    void Load(BinStream &stream) override;

    /**
     * Set the mesh to draw.
     *
     * @param pMesh The mesh, or null.
     * @ghidraAddress NTSC-U/C: 0x0023bc78
     * @ghidraAddress PAL: 0x002447f8
     */
    void SetMesh(RndMesh *pMesh);

    /**
     * Create a multimesh.
     *
     * @param pszName The registry key.
     * @return The multimesh.
     * @ghidraAddress NTSC-U/C: 0x00390170
     * @ghidraAddress PAL: 0x003fe878
     */
    static RndObject *New(const char *pszName) {
        return new RndMultiMesh(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `MultiMesh`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a30
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a34
     */
    static int sRev;

    /**
     * The version Load() read last.
     *
     * @ghidraAddress NTSC-U/C: 0x0043c7d0
     */
    static int sLoadRev;

    RndMesh *mMesh;                   /*!< The mesh to draw. */
    std::list<Transform> mTransforms; /*!< The transform of each instance, as the mesh's local. */

protected:
    /**
     * Take a reference on the mesh.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0023b4a0
     * @ghidraAddress PAL: 0x00244020
     */
    void AcquireRefs();

    /**
     * Drop the reference on the mesh.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0023b4d0
     * @ghidraAddress PAL: 0x00244050
     */
    void ReleaseRefs();
};

/**
 * Write a list of transforms as their count followed by the rows of each.
 *
 * @param stream The stream to write to.
 * @param transforms The transforms.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0038fda8
 * @ghidraAddress PAL: 0x003fe4b0
 */
BinStream &operator<<(BinStream &stream, const std::list<Transform> &transforms);

/**
 * Read a list of transforms the transform list writer wrote.
 *
 * @param stream The stream to read from.
 * @param transforms Receives the transforms.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0038ff98
 * @ghidraAddress PAL: 0x003fe6a0
 */
BinStream &operator>>(BinStream &stream, std::list<Transform> &transforms);
