#pragma once

#include <cstddef>
#include <list>

#include "math/transform.h"
#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndmesh.h"
#include "rnd/rndtext.h"

/**
 * Motion trail that draws a mesh or a text again at the transforms it recently occupied.
 *
 * The RTTI includes the class name and records RndDrawable as the one base. A text subject takes
 * precedence over a mesh subject, but a mesh subject supplies the material even when a text is
 * set. The trail records a transform every mRate frames and retains the newest mLength of them.
 */
class RndBlur : public RndDrawable {
public:
    /**
     * Construct a trail without a subject, recording nothing, every frame, at full falloff.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x002200c8
     * @ghidraAddress PAL: 0x00228e98
     */
    explicit RndBlur(const char *pszName);

    /**
     * Drop the references on the subjects.
     *
     * @ghidraAddress NTSC-U/C: 0x0021fe78
     * @ghidraAddress PAL: 0x00228c48
     */
    ~RndBlur() override;

    /**
     * Allocate a trail, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Blur", 0);
    }

    /**
     * Release a trail.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Add the objects of the mesh and of the text to a list.
     *
     * The children are not added.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x0021f598
     * @ghidraAddress PAL: 0x00228368
     */
    void ListDrawObjects(std::list<RndObject *> &objects) override;

    /**
     * Add the trail when its subject draws at the current transform or a recorded one, then the
     * children, to a list.
     *
     * The subject is moved to each recorded transform in turn and put back afterwards.
     *
     * @param drawables The list to add to.
     * @ghidraAddress NTSC-U/C: 0x0021f600
     * @ghidraAddress PAL: 0x002283d0
     */
    void ListDrawables(std::list<RndDrawable *> &drawables) override;

    /**
     * Draw the subject, then once per recorded transform with a falling alpha.
     *
     * The first step is drawn at the material alpha scaled by mFalloff, less the part of a step
     * the countdown has used, and each later step loses an equal share. A recorded transform equal
     * to the one before it, the first compared with the current world transform, is skipped. A
     * mesh with depth testing draws its trail with RndMesh::kZModeZReadOnly. The subject's
     * transforms, the material alpha, and the depth mode are restored afterwards. Every mRate
     * frames the world transform goes to the front of mXfms, the oldest dropping once mLength are
     * stored.
     *
     * @return 1, to draw the children.
     * @ghidraAddress NTSC-U/C: 0x0021f8d0
     * @ghidraAddress PAL: 0x002286a0
     */
    int DrawShowing() override;

    /**
     * Write a description of the trail and its base.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0021ef20
     * @ghidraAddress PAL: 0x00227cf0
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the version, the base, the subjects' names, the length, the rate, and the falloff.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x0021f048
     * @ghidraAddress PAL: 0x00227e18
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a subject with another object.
     *
     * A replacement of another class, or null, clears the subject.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x0021f420
     * @ghidraAddress PAL: 0x002281f0
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x0037eff0
     * @ghidraAddress PAL: 0x003ed708
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy the subjects, the length, the rate, and the falloff, and discard the recorded
     * transforms.
     *
     * @param pSource The trail to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x0021ee58
     * @ghidraAddress PAL: 0x00227c28
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote.
     *
     * Version 1 adds the falloff and version 2 the text.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x0021f158
     * @ghidraAddress PAL: 0x00227f28
     */
    void Load(BinStream &stream) override;

    /**
     * Set how many transforms the trail records and discard the recorded ones.
     *
     * A negative length is clamped to 0 and disables the trail. The name is inferred.
     *
     * @param nLength The number of transforms.
     * @ghidraAddress NTSC-U/C: 0x00220390
     * @ghidraAddress PAL: 0x00229160
     */
    void SetLength(int nLength);

    /**
     * Set how many frames pass between two recorded transforms and discard the recorded ones.
     *
     * A rate below 1 is clamped to 1. The name is inferred.
     *
     * @param nRate The frame interval.
     * @ghidraAddress NTSC-U/C: 0x002203d0
     * @ghidraAddress PAL: 0x002291a0
     */
    void SetRate(int nRate);

    /**
     * Create a trail.
     *
     * @param pszName The registry key.
     * @return The trail.
     * @ghidraAddress NTSC-U/C: 0x0037f008
     * @ghidraAddress PAL: 0x003ed720
     */
    static RndObject *New(const char *pszName) {
        return new RndBlur(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `Blur`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0980
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0984
     */
    static int sRev;

    /**
     * The version Load() read last.
     *
     * @ghidraAddress NTSC-U/C: 0x0043c680
     */
    static int sLoadRev;

    RndMesh *mMesh;             /*!< The mesh subject, or null. */
    RndText *mText;             /*!< The text subject, or null. */
    int mLength;                /*!< The number of transforms to record. */
    int mRate;                  /*!< The number of frames between two recorded transforms. */
    float mFalloff;             /*!< The fraction of the material alpha of the first step. */
    std::list<Transform> mXfms; /*!< The recorded world transforms, newest first. */
    int mCountdown;             /*!< The frames left before the next transform is recorded. */

protected:
    /**
     * Drop the references on the subjects.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0021f3d0
     * @ghidraAddress PAL: 0x002281a0
     */
    void ReleaseRefs();

    /**
     * Take a reference on the subjects and discard the recorded transforms.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0021f378
     * @ghidraAddress PAL: 0x00228148
     */
    void AcquireRefs();
};
