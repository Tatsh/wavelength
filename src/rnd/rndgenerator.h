#pragma once

#include <cstddef>
#include <list>

#include "math/transform.h"
#include "math/vector3.h"
#include "os/binstream.h"
#include "os/mem.h"
#include "os/prnstream.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndcam.h"
#include "rnd/rnddrawable.h"
#include "rnd/rndmesh.h"
#include "rnd/rndmultimesh.h"
#include "rnd/rndparticlesys.h"
#include "rnd/rndtransanim.h"
#include "rnd/rndtransformable.h"
#include "rnd/rndview.h"

/**
 * Emitter of instances that each follow a path animation from the frame they are born at.
 *
 * The RTTI includes the class name and records RndAnimatable, RndTransformable, and RndDrawable
 * as bases. Instances are born at a random interval between mRateGenLow and mRateGenHigh frames,
 * each with a random rotation within mPathVarMax degrees and a random scale, and live as long as
 * the path lasts. Each instance draws one subject, the first of mView, mMesh, mMultiMesh, and
 * mParticleSys that is set, at the transform the path gives for the instance's age.
 */
class RndGenerator : public RndAnimatable, public RndTransformable, public RndDrawable {
public:
    /** One generated instance. The RTTI includes the nested class name. */
    struct Instance {
        float mFrame;   /*!< The frame the instance was born at. */
        Transform mXfm; /*!< The rotation and the position of the generator at the birth. */
        Vector3 mScale; /*!< The scale along each axis. */
    };

    /** A routine that draws or places one instance's subject at a transform. */
    typedef void (RndGenerator::*DrawInstanceFunc)(float fAge, const Transform &xfm);

    /**
     * Construct a generator without a path or a subject that generates every 100 frames.
     *
     * @param pszName The registry key.
     * @ghidraAddress NTSC-U/C: 0x00226980
     * @ghidraAddress PAL: 0x0022f730
     */
    explicit RndGenerator(const char *pszName);

    /**
     * Drop the references and the instances.
     *
     * @ghidraAddress NTSC-U/C: 0x00382708
     * @ghidraAddress PAL: 0x003f0e10
     */
    ~RndGenerator() override;

    /**
     * Allocate a generator, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "Generator", 0);
    }

    /**
     * Release a generator.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Drop the instances whose path has run out, animate the view, and generate the instances
     * due by a frame.
     *
     * The first call only records the frame. A negative mRateGenLow generates nothing.
     *
     * @param fFrame The frame.
     * @return 1.
     * @ghidraAddress NTSC-U/C: 0x002287e8
     * @ghidraAddress PAL: 0x00231558
     */
    int SetFrameSelf(float fFrame) override;

    /**
     * Add the generator and the objects of the subjects to a list.
     *
     * @param objects The list to add to.
     * @ghidraAddress NTSC-U/C: 0x00228a40
     * @ghidraAddress PAL: 0x002317b0
     */
    void ListDrawObjects(std::list<RndObject *> &objects) override;

    /**
     * Add the generator, when a subject of an instance would draw, then the children, to a list.
     *
     * Only a generator without a path, or with a path and no subject, tests its instances. The
     * subject is moved to each instance in turn until one would draw.
     *
     * @param drawables The list to add to.
     * @ghidraAddress NTSC-U/C: 0x00228b60
     * @ghidraAddress PAL: 0x002318d0
     */
    void ListDrawables(std::list<RndDrawable *> &drawables) override;

    /**
     * Draw the subject at each instance.
     *
     * A multimesh receives one transform for each instance and draws once, and a particle system
     * moves one particle to each instance and draws once.
     *
     * @return 1.
     * @ghidraAddress NTSC-U/C: 0x002290f8
     * @ghidraAddress PAL: 0x00231e68
     */
    int DrawShowing() override;

    /**
     * Write a description of the generator and its bases.
     *
     * The instances and the path frames are listed at a dump level of 2 and above.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00227398
     * @ghidraAddress PAL: 0x00230148
     */
    void DumpText(PrnStream &stream) override;

    /**
     * Write the generator and its bases.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002279c0
     * @ghidraAddress PAL: 0x00230770
     */
    void Save(BinStream &stream) override;

    /**
     * Replace a referenced object with another in the bases and the subjects.
     *
     * @param pFrom The object being replaced.
     * @param pTo The replacement, or null.
     * @ghidraAddress NTSC-U/C: 0x00226fa0
     * @ghidraAddress PAL: 0x0022fd50
     */
    void Replace(RndObject *pFrom, RndObject *pTo) override;

    /**
     * Report the class name.
     *
     * @return sClassName.
     * @ghidraAddress NTSC-U/C: 0x00382b68
     */
    const char *ClassName() const override {
        return sClassName;
    }

    /**
     * Copy another generator and its bases.
     *
     * @param pSource The generator to copy from.
     * @param nFlags The set of fields to copy.
     * @ghidraAddress NTSC-U/C: 0x00227700
     * @ghidraAddress PAL: 0x002304b0
     */
    void Copy(const RndObject *pSource, int nFlags) override;

    /**
     * Read what Save() wrote, or an earlier version of it.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00227cb8
     * @ghidraAddress PAL: 0x00230a50
     */
    void Load(BinStream &stream) override;

    /**
     * Drop the references on the subjects, the path, and the birth camera, and the instances.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00227850
     * @ghidraAddress PAL: 0x00230600
     */
    void ReleaseRefs();

    /**
     * Take the references on the subjects, the path, and the birth camera, then give each
     * instance a particle.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00227908
     * @ghidraAddress PAL: 0x002306b8
     */
    void AcquireRefs();

    /**
     * Free the particles of the particle system, then allocate one for each instance.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00228368
     * @ghidraAddress PAL: 0x002310d8
     */
    void ResetParticles();

    /**
     * Set the path and the frames of it the instances play.
     *
     * A frame of -1 takes the start or the end of the path.
     *
     * @param pPath The path, or null.
     * @param fStart The first frame.
     * @param fEnd The last frame.
     * @ghidraAddress NTSC-U/C: 0x00228420
     * @ghidraAddress PAL: 0x00231190
     */
    void SetPath(RndTransAnim *pPath, float fStart, float fEnd);

    /**
     * Add an instance born at a frame, unless the birth camera rules it out.
     *
     * The birth camera rules out an instance behind it when mBirthFrontOnly is set, and one
     * further than mBirthSquareDist when mBirthCheckDist is set.
     *
     * @param fFrame The frame.
     * @ghidraAddress NTSC-U/C: 0x00228520
     * @ghidraAddress PAL: 0x00231290
     */
    void Generate(float fFrame);

    /**
     * Move the view to a transform, animate it to an age when mAnimateFromStart is set, and draw
     * it.
     *
     * The name is inferred.
     *
     * @param fAge The age of the instance.
     * @param xfm The transform.
     * @ghidraAddress NTSC-U/C: 0x00228f70
     * @ghidraAddress PAL: 0x00231ce0
     */
    void DrawView(float fAge, const Transform &xfm);

    /**
     * Move the mesh to a transform and draw it.
     *
     * The name is inferred.
     *
     * @param fAge The age of the instance.
     * @param xfm The transform.
     * @ghidraAddress NTSC-U/C: 0x00229008
     * @ghidraAddress PAL: 0x00231d78
     */
    void DrawMesh(float fAge, const Transform &xfm);

    /**
     * Store a transform in the next transform of the multimesh.
     *
     * The name is inferred.
     *
     * @param fAge The age of the instance.
     * @param xfm The transform.
     * @ghidraAddress NTSC-U/C: 0x00229088
     * @ghidraAddress PAL: 0x00231df8
     */
    void SetMultiMeshXfm(float fAge, const Transform &xfm);

    /**
     * Move the next particle to the position of a transform.
     *
     * The name is inferred.
     *
     * @param fAge The age of the instance.
     * @param xfm The transform.
     * @ghidraAddress NTSC-U/C: 0x002290d0
     * @ghidraAddress PAL: 0x00231e40
     */
    void SetParticlePos(float fAge, const Transform &xfm);

    /**
     * Create a generator.
     *
     * @param pszName The registry key.
     * @return The generator.
     * @ghidraAddress NTSC-U/C: 0x00382b78
     * @ghidraAddress PAL: 0x003f1280
     */
    static RndObject *New(const char *pszName) {
        return new RndGenerator(pszName);
    }

    /**
     * The class name a `.rnd` file writes, `Generator`.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09c0
     */
    static const char *sClassName;

    /**
     * The format version Save() writes and Load() accepts.
     *
     * @ghidraAddress NTSC-U/C: 0x003b09c4
     */
    static int sRev;

    std::list<Instance> mInstances;         /*!< The live instances, oldest first. */
    RndTransAnim *mPath;                    /*!< The path the instances follow, or null. */
    float mPathStartFrame;                  /*!< The frame of the path an instance starts at. */
    float mPathEndFrame;                    /*!< The frame of the path an instance ends at. */
    RndMesh *mMesh;                         /*!< The mesh drawn at each instance, or null. */
    RndView *mView;                         /*!< The view drawn at each instance, or null. */
    RndMultiMesh *mMultiMesh;               /*!< The multimesh of the instances, or null. */
    RndParticleSys *mParticleSys;           /*!< The particles of the instances, or null. */
    int mAnimateFromStart;                  /*!< Non-zero to animate the view to each age. */
    float mNextFrame;                       /*!< The frame the next instance is born at. */
    int mBirthFrontOnly;                    /*!< Non-zero to rule out births behind mBirthCam. */
    int mBirthCheckDist;                    /*!< Non-zero to rule out distant births. */
    float mBirthSquareDist;                 /*!< The squared distance mBirthCheckDist applies. */
    RndCam *mBirthCam;                      /*!< The camera the births are tested against. */
    float mRateGenLow;                      /*!< The least number of frames between births. */
    float mRateGenHigh;                     /*!< The most number of frames between births. */
    float mScaleGenLow;                     /*!< The least scale of an instance. */
    float mScaleGenHigh;                    /*!< The most scale of an instance. */
    float mPathVarMax[3];                   /*!< The most rotation about each axis, in degrees. */
    Particle *mCurParticle;                 /*!< The particle of the instance being placed. */
    std::list<Transform>::iterator mCurXfm; /*!< The multimesh transform being placed. */

private:
    // Build the transform of an instance from the path at its age, its scale, and its birth
    // transform, and report the age. The binary inlines this in ListDrawables() and DrawShowing().
    inline float InstanceXfm(const Instance &instance, float fDirection, Transform &xfm);
};

/**
 * Write an instance's birth frame and transform.
 *
 * @param stream The stream to write to.
 * @param instance The instance.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002294a8
 * @ghidraAddress PAL: 0x00232218
 */
PrnStream &operator<<(PrnStream &stream, const RndGenerator::Instance &instance);
