#pragma once

#include <list>
#include <vector>

#include "math/key.h"
#include "math/quaternion.h"
#include "math/transform.h"
#include "math/vector3.h"
#include "rnd/rndanimatable.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndtransformable.h"

/**
 * Animation of a transformable's local transform by rotation, translation, and scale keys.
 *
 * The RTTI includes the class name and records RndAnimatable and RndDrawable as bases. The keys
 * may be shared, in which case mFramesOwner identifies the animation that has them.
 */
class RndTransAnim : public RndAnimatable, public RndDrawable {
public:
    /** The bit of the Copy() flags that shares the source's keys rather than copying them. */
    static constexpr int kCopyShareKeys = 0x100;

    /** How the translation and scale keys interpolate. */
    enum Interp {
        kInterpLinear = 0, /*!< Straight lines between keys. */
        kInterpSpline = 1, /*!< Hermite splines through the keys. */
    };

    /**
     * Construct an animation of nothing with spline translation, linear scale, and no keys.
     *
     * The animation is its own frames owner.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00392ec0
     * @ghidraAddress PAL: 0x004015c8
     */
    explicit RndTransAnim(const char *pszName);

    /**
     * Drop the references on the target and the frames owner.
     *
     * @ghidraAddress NTSC-U/C: 0x00392a18
     * @ghidraAddress PAL: 0x00401090
     */
    ~RndTransAnim() override;

    /**
     * Report the last frame of any key.
     *
     * @return The frame.
     * @ghidraAddress NTSC-U/C: 0x002464c0
     * @ghidraAddress PAL: 0x0024ef60
     */
    float EndFrame() override;

    /**
     * Add the target to a list, then the objects of the children.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x00246400
     * @ghidraAddress PAL: 0x0024eea0
     */
    void ListAnimObjects(std::list<RndObject *> &objects) override;

    /**
     * Apply a frame to the target's local transform.
     *
     * @param fFrame The frame.
     * @return 1, to pass the frame on to the children.
     * @ghidraAddress NTSC-U/C: 0x002473a8
     * @ghidraAddress PAL: 0x0024fe48
     */
    int SetFrameSelf(float fFrame) override;

    /**
     * Report the first frame of any key.
     *
     * The name is inferred.
     *
     * @return The frame.
     * @ghidraAddress NTSC-U/C: 0x00246578
     * @ghidraAddress PAL: 0x0024f018
     */
    virtual float StartFrame();

    /**
     * Write a description of the animation and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00245700
     * @ghidraAddress PAL: 0x0024e1d0
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the animation and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002458d0
     * @ghidraAddress PAL: 0x0024e3a0
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a referenced object with another.
     *
     * A frames owner replaced by null becomes the animation itself.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00245570
     * @ghidraAddress PAL: 0x0024e040
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00392e68
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another animation and its bases.
     *
     * The keys are copied when the source has its own and the flags do not request sharing.
     * Otherwise the source's frames owner becomes this one's.
     *
     * @param pSource The animation to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x002462e8
     * @ghidraAddress PAL: 0x0024ed88
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00245ae0
     * @ghidraAddress PAL: 0x0024e598
     */
    void Load(BinStream &stream) override;

    /**
     * Set the transformable the animation drives.
     *
     * @param pTrans The target, or null.
     * @ghidraAddress NTSC-U/C: 0x002454c0
     * @ghidraAddress PAL: 0x0024df90
     */
    void SetTrans(RndTransformable *pTrans);

    /**
     * Take the keys from another animation.
     *
     * @param pOwner The animation that has the keys, or null.
     * @ghidraAddress NTSC-U/C: 0x00245518
     * @ghidraAddress PAL: 0x0024dfe8
     */
    void SetFramesOwner(RndTransAnim *pOwner);

    /**
     * Build the transform at a frame.
     *
     * A repeating translation adds the distance of each completed pass. A path-following
     * animation turns the basis along the path. A channel without keys leaves its part of the
     * transform unchanged, or sets it to the identity when asked to.
     *
     * @param fFrame The frame.
     * @param xfm Receives the transform.
     * @param bWhole Non-zero to set the parts that have no keys to the identity.
     * @ghidraAddress NTSC-U/C: 0x00246ee0
     * @ghidraAddress PAL: 0x0024f980
     */
    void MakeTransform(float fFrame, Transform &xfm, int bWhole);

    /**
     * Measure the length of the translation path between two frames.
     *
     * The name is inferred.
     *
     * @param fStart The first frame.
     * @param fEnd The last frame.
     * @return The length, or 0 with fewer than two translation keys.
     * @ghidraAddress NTSC-U/C: 0x00247630
     * @ghidraAddress PAL: 0x002500d0
     */
    float PathLength(float fStart, float fEnd);

    /**
     * Create an animation.
     *
     * @param pszName The registry key.
     * @return The animation.
     * @ghidraAddress NTSC-U/C: 0x00392e78
     * @ghidraAddress PAL: 0x00401580
     */
    static RndObject *New(const char *pszName) {
        return new RndTransAnim(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `TransAnim`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a5c
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0a60
     */
    static int sRev;

    RndTransformable *mTrans;             /*!< The transformable the animation drives. */
    int mTransInterp;                     /*!< The Interp of the translation keys. */
    int mScaleInterp;                     /*!< The Interp of the scale keys. */
    std::vector<Key<Quat>> mRotKeys;      /*!< The rotation keys. */
    std::vector<Key<Vector3>> mTransKeys; /*!< The translation keys. */
    std::vector<Key<Vector3>> mScaleKeys; /*!< The scale keys. */
    RndTransAnim *mFramesOwner;           /*!< The animation that has the keys. */
    int mRepeatTrans;                     /*!< Non-zero to repeat the translation path. */
    int mFollowPath;                      /*!< Non-zero to turn along the translation path. */

protected:
    /**
     * Drop the references on the target and the frames owner.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00245a40
     * @ghidraAddress PAL: 0x0024e4f8
     */
    void ReleaseRefs();

    /**
     * Take references on the target and the frames owner.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00245a90
     * @ghidraAddress PAL: 0x0024e548
     */
    void AcquireRefs();

    /**
     * Measure the translation path between two keys, from one position to another.
     *
     * A linear path is measured as the straight distance between the keys whatever the
     * positions. The name is inferred.
     *
     * @param nFrom The first key.
     * @param nTo The second key.
     * @param fStep The step a spline path is sampled at.
     * @param fStart The position to start at, from 0 to 1.
     * @param fEnd The position to stop at, from 0 to 1.
     * @return The length.
     * @ghidraAddress NTSC-U/C: 0x002474a8
     * @ghidraAddress PAL: 0x0024ff48
     */
    float SegmentLength(int nFrom, int nTo, float fStep, float fStart, float fEnd);
};

/**
 * Interpolate vector keys at a frame.
 *
 * Nothing is written for no keys.
 *
 * @param keys The keys.
 * @param nInterp The RndTransAnim::Interp.
 * @param out Receives the value.
 * @param pTangent Receives the direction of the path at the frame, or null for none.
 * @param fFrame The frame.
 * @ghidraAddress NTSC-U/C: 0x00246988
 * @ghidraAddress PAL: 0x0024f428
 */
void InterpVector(const std::vector<Key<Vector3>> &keys,
                  int nInterp,
                  Vector3 &out,
                  Vector3 *pTangent,
                  float fFrame);

/**
 * Write the name of an interpolation.
 *
 * @param stream The stream to write to.
 * @param eInterp The interpolation. An unknown one writes nothing.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00247438
 * @ghidraAddress PAL: 0x0024fed8
 */
PrnStream &operator<<(PrnStream &stream, RndTransAnim::Interp eInterp);
